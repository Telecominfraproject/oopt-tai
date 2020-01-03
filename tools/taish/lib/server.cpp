/**
 * @file    server.cpp
 *
 * @brief   This module implements TAI gRPC server
 *
 * @copyright Copyright (c) 2018 Nippon Telegraph and Telephone Corporation
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *
 */

#include "taigrpc.hpp"
#include "taimetadata.h"
#include <sstream>
#include <chrono>
#include <functional>

using grpc::Status;
using grpc::StatusCode;

static const std::string _serialize_status(const tai_status_t status) {
    tai_serialize_option_t option{ .human = true, .valueonly = true, .json = false};
    size_t size = 64;
    char buf[64] = {0};
    if ( tai_serialize_status(buf, 64, status, &option) < 0 ) {
        return "unknown";
    }
    return std::string(buf);
}

::grpc::Status _status(const std::string& message, const tai_status_t status) {
    std::stringstream ss;
    std::string r = _serialize_status(status);
    ss << message << ": " << r;
    return Status(StatusCode::UNKNOWN, ss.str());
}

static void add_status(::grpc::ServerContext* context, tai_status_t status) {
    context->AddTrailingMetadata("tai-status-code", std::to_string(status));
    context->AddTrailingMetadata("tai-status-msg", _serialize_status(status));
}

static int _serialize_attribute(const tai_attr_metadata_t* meta, const tai_attribute_t* attr, std::string& out) {
    tai_serialize_option_t option{true, true, true};
    size_t buf_size = 64;
    char bbuf[64] = {0}, *buf = bbuf;
    auto count = tai_serialize_attribute(buf, buf_size, meta, attr, &option);
    if ( count < 0 ) {
        return count;
    }
    if ( count > buf_size ) {
        buf_size = count + 1;
        buf = new char[buf_size];
        count = tai_serialize_attribute(buf, buf_size, meta, attr, &option);
        if ( count < 0 || count > buf_size ) {
            return count;
        }
        out = buf;
    }
    out = buf;
    // check if buf is dynamically allocated
    if ( buf != bbuf ) {
        delete[] buf;
    }
    return 0;
}

TAIAPIModuleList::TAIAPIModuleList(uint32_t module_size, uint32_t hostif_size, uint32_t netif_size) : m_module_size(module_size), m_hostif_size(hostif_size), m_netif_size(netif_size) {
    m_list.count = module_size;
    m_list.list = new tai_api_module_t[module_size];
    for ( auto i = 0; i < module_size; i++) {
        m_list.list[i].hostifs.count = hostif_size;
        m_list.list[i].hostifs.list = new tai_object_id_t[hostif_size];
        m_list.list[i].netifs.count = netif_size;
        m_list.list[i].netifs.list = new tai_object_id_t[netif_size];
    }
}

TAIAPIModuleList::~TAIAPIModuleList() {
    for ( auto i = 0; i < m_module_size; i++ ) {
        auto l = m_list.list[i];
        delete[] l.hostifs.list;
        delete[] l.netifs.list;
    }
    delete[] m_list.list;
}

::grpc::Status TAIServiceImpl::ListModule(::grpc::ServerContext* context, const taish::ListModuleRequest* request, ::grpc::ServerWriter< taish::ListModuleResponse>* writer) {

    TAIAPIModuleList l;
    auto list = l.list();
    auto ret = m_api->list_module(list);
    if ( ret != TAI_STATUS_SUCCESS ) {
        add_status(context, ret);
        return Status::OK;
    }
    auto meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_MODULE, TAI_MODULE_ATTR_LOCATION);
    tai_attribute_t attr = {0};
    ret = tai_metadata_alloc_attr_value(meta, &attr, nullptr);
    if( ret != TAI_STATUS_SUCCESS ) {
        add_status(context, ret);
        return Status::OK;
    }

    for ( auto i = 0; i < list->count; i++ ) {
        auto res = taish::ListModuleResponse();
        auto m = res.mutable_module();
        auto module = list->list[i];
        m->set_location(module.location);
        m->set_present(module.present);
        m->set_oid(module.id);
        if ( module.id != TAI_NULL_OBJECT_ID ) {
            ret = m_api->module_api->get_module_attribute(module.id, &attr);
            if ( ret != TAI_STATUS_SUCCESS ) {
                goto err;
            }
            m->set_location(attr.value.charlist.list, attr.value.charlist.count);
            m->clear_hostifs();
            for ( auto i = 0; i < module.hostifs.count; i++ ) {
                auto hostif = m->add_hostifs();
                hostif->set_index(i);
                hostif->set_oid(module.hostifs.list[i]);
            }
            m->clear_netifs();
            for ( auto i = 0; i < module.netifs.count; i++ ) {
                auto netif = m->add_netifs();
                netif->set_index(i);
                netif->set_oid(module.netifs.list[i]);
            }
        }
        writer->Write(res);
    }

err:
    if ( (ret = tai_metadata_free_attr_value(meta, &attr, nullptr)) != TAI_STATUS_SUCCESS ) {
        add_status(context, ret);
        return Status::OK;
    }
    add_status(context, ret);
    return Status::OK;
}

static void usage(const tai_attr_metadata_t* meta, std::string* str) {
    if ( meta->isenum && meta->enummetadata != nullptr ) {
        auto m = meta->enummetadata;
        *str = "[";
        for ( auto i = 0; i < m->valuescount; i++ ) {
            *str += m->valuesshortnames[i];
            if ( i < m->valuescount - 1 ) {
                *str += "|";
            }
        }
        *str += "]";
        return;
    }
    char buf[32] = {0};
    tai_serialize_option_t option = {true};
    auto ret = tai_serialize_attr_value_type(buf, 32, meta->attrvaluetype, &option);
    if ( ret < 0 ) {
        *str = "<unknown>";
    }
    std::stringstream ss;
    ss << "<" << buf << ">";
    *str = ss.str();
}

static void convert_metadata(const tai_attr_metadata_t* const src, taish::AttributeMetadata* const dst) {
    dst->set_attr_id(src->attrid);
    dst->set_name(src->attridname);
    dst->set_short_name(src->attridshortname);
    dst->set_object_type(static_cast<taish::TAIObjectType>(src->objecttype));
    auto u = dst->mutable_usage();
    usage(src, u);
    dst->set_is_readonly(src->isreadonly);
    dst->set_is_mandatoryoncreate(src->ismandatoryoncreate);
    dst->set_is_createonly(src->iscreateonly);
    dst->set_is_createandset(src->iscreateandset);
    dst->set_is_key(src->iskey);
}

::grpc::Status TAIServiceImpl::ListAttributeMetadata(::grpc::ServerContext* context, const taish::ListAttributeMetadataRequest* request, ::grpc::ServerWriter< taish::ListAttributeMetadataResponse>* writer) {
    auto res = taish::ListAttributeMetadataResponse();
    auto object_type = request->object_type();
    auto info = tai_metadata_all_object_type_infos[object_type];
    if ( info == nullptr ) {
        for ( auto i = 0; i < tai_metadata_attr_sorted_by_id_name_count; i++ ) {
            auto src = tai_metadata_attr_sorted_by_id_name[i];
            auto dst = res.mutable_metadata();
            convert_metadata(src, dst);
            writer->Write(res);
        }
        return Status::OK;
    }
    for ( auto i = 0; i < info->attrmetadatalength; i++ ) {
        auto src = info->attrmetadata[i];
        auto dst = res.mutable_metadata();
        convert_metadata(src, dst);
        writer->Write(res);
    }
    return Status::OK;
}

::grpc::Status TAIServiceImpl::GetAttributeMetadata(::grpc::ServerContext* context, const taish::GetAttributeMetadataRequest* request, taish::GetAttributeMetadataResponse* response) {
    auto object_type = request->object_type();
    int32_t attr_id = 0;
    if ( request->attr_name() != "" ) {
        auto attr_name = request->attr_name();
        int ret;
        tai_serialize_option_t option{true};
        switch (object_type) {
        case taish::MODULE:
            ret = tai_deserialize_module_attr(attr_name.c_str(), &attr_id, &option);
            break;
        case taish::NETIF:
            ret = tai_deserialize_network_interface_attr(attr_name.c_str(), &attr_id, &option);
            break;
        case taish::HOSTIF:
            ret = tai_deserialize_host_interface_attr(attr_name.c_str(), &attr_id, &option);
            break;
        default:
            ret = TAI_STATUS_NOT_SUPPORTED;
        }
        if ( ret < 0 ) {
            add_status(context, ret);
            return Status::OK;
        }
    } else {
        attr_id = request->attr_id();
    }

    auto meta = tai_metadata_get_attr_metadata(static_cast<tai_object_type_t>(object_type), attr_id);
    if ( meta == nullptr ) {
        return Status(StatusCode::NOT_FOUND, "not found metadata");
    }
    auto res = response->mutable_metadata();
    convert_metadata(meta, res);
    add_status(context, TAI_STATUS_SUCCESS);
    return Status::OK;
}

::grpc::Status TAIServiceImpl::GetAttribute(::grpc::ServerContext* context, const taish::GetAttributeRequest* request, taish::GetAttributeResponse* response) {
    auto oid = request->oid();
    taish::Attribute a = request->attribute();
    taish::Attribute* res;
    auto id = a.attr_id();
    auto v = a.value();
    auto type = tai_object_type_query(oid);
    auto meta = tai_metadata_get_attr_metadata(type, id);
    tai_attribute_t attr = {0}, reference = {0};
    attr.id = id;
    tai_serialize_option_t option{true, true, true};
    tai_alloc_info_t alloc_info = { .list_size = 16 };
    size_t count = 0;
    std::string value;

again:
    if( tai_metadata_alloc_attr_value(meta, &attr, &alloc_info) != TAI_STATUS_SUCCESS ) {
        return Status(StatusCode::UNKNOWN, "failed to alloc value");
    }

    tai_status_t ret;
    if ( v.size() > 0 ) {
        ret = tai_deserialize_attribute_value(v.c_str(), meta, &attr.value, &option);
        if ( ret == TAI_STATUS_BUFFER_OVERFLOW && alloc_info.reference == nullptr ) {
            reference = attr;
            alloc_info.reference = &reference;
            goto again;
        } else if ( ret < 0 ) {
            goto err;
        }
    }

    switch (type) {
    case TAI_OBJECT_TYPE_MODULE:
        ret = m_api->module_api->get_module_attribute(oid, &attr);
        break;
    case TAI_OBJECT_TYPE_NETWORKIF:
        ret = m_api->netif_api->get_network_interface_attribute(oid, &attr);
        break;
    case TAI_OBJECT_TYPE_HOSTIF:
        ret = m_api->hostif_api->get_host_interface_attribute(oid, &attr);
        break;
    default:
        ret = TAI_STATUS_FAILURE;
    }

    // if alloc size is not enough and this is the first try (we can decide this
    // by checking alloc_info.reference == nullptr), try again with the reference
    // attribute which contains size information for the necessary allocation
    if ( ret == TAI_STATUS_BUFFER_OVERFLOW && alloc_info.reference == nullptr ) {
        reference = attr;
        alloc_info.reference = &reference;
        goto again;
    } else if ( ret != TAI_STATUS_SUCCESS ) {
        goto err;
    }

    if ( _serialize_attribute(meta, &attr, value) != 0 ) {
        ret = TAI_STATUS_FAILURE;
        goto err;
    }

    res = response->mutable_attribute();
    res->set_attr_id(id);
    res->set_value(value);
err:
    if ( tai_metadata_free_attr_value(meta, &attr, &alloc_info) != TAI_STATUS_SUCCESS ) {
        return Status(StatusCode::UNKNOWN, "failed to free value");
    }
    if ( alloc_info.reference != nullptr ) {
        if ( tai_metadata_free_attr_value(meta, const_cast<tai_attribute_t*>(alloc_info.reference), &alloc_info) != TAI_STATUS_SUCCESS ) {
            return Status(StatusCode::UNKNOWN, "failed to free reference value");
        }
    }
    add_status(context, ret);
    return Status::OK;
}

::grpc::Status TAIServiceImpl::SetAttribute(::grpc::ServerContext* context, const taish::SetAttributeRequest* request, taish::SetAttributeResponse* response) {
    auto oid = request->oid();
    auto a = request->attribute();
    auto id = a.attr_id();
    auto v = a.value();
    auto type = tai_object_type_query(oid);
    auto meta = tai_metadata_get_attr_metadata(type, id);
    tai_attribute_t attr = {0}, reference = {0};
    attr.id = id;
    tai_serialize_option_t option{true};
    tai_alloc_info_t alloc_info = { .list_size = 16 };

again:
    if( tai_metadata_alloc_attr_value(meta, &attr, &alloc_info) != TAI_STATUS_SUCCESS ) {
        return Status(StatusCode::UNKNOWN, "failed to alloc value");
    }

    auto ret = tai_deserialize_attribute_value(v.c_str(), meta, &attr.value, &option);
    if ( ret == TAI_STATUS_BUFFER_OVERFLOW && alloc_info.reference == nullptr ) {
        reference = attr;
        alloc_info.reference = &reference;
        goto again;
    } else if ( ret < 0 ) {
        goto err;
    }

    switch (type) {
    case TAI_OBJECT_TYPE_MODULE:
        ret = m_api->module_api->set_module_attribute(oid, &attr);
        break;
    case TAI_OBJECT_TYPE_NETWORKIF:
        ret = m_api->netif_api->set_network_interface_attribute(oid, &attr);
        break;
    case TAI_OBJECT_TYPE_HOSTIF:
        ret = m_api->hostif_api->set_host_interface_attribute(oid, &attr);
        break;
    default:
        ret = TAI_STATUS_FAILURE;
    }
err:
    if ( tai_metadata_free_attr_value(meta, &attr, nullptr) != TAI_STATUS_SUCCESS ) {
        return Status(StatusCode::UNKNOWN, "failed to free value");
    }
    if ( alloc_info.reference != nullptr ) {
        if ( tai_metadata_free_attr_value(meta, const_cast<tai_attribute_t*>(alloc_info.reference), &alloc_info) != TAI_STATUS_SUCCESS ) {
            return Status(StatusCode::UNKNOWN, "failed to free reference value");
        }
    }
    add_status(context, ret);
    return Status::OK;
}

::grpc::Status TAIServiceImpl::ClearAttribute(::grpc::ServerContext* context, const taish::ClearAttributeRequest* request, taish::ClearAttributeResponse* response) {
    auto oid = request->oid();
    auto id = request->attr_id();
    auto type = tai_object_type_query(oid);
    tai_status_t ret;

    switch (type) {
    case TAI_OBJECT_TYPE_HOSTIF:
        ret = m_api->hostif_api->clear_host_interface_attribute(oid, id);
        break;
    default:
        ret = TAI_STATUS_FAILURE;
    }
    add_status(context, ret);
    return Status::OK;
}

int TAINotifier::notify(const tai_notification_t& n) {
    std::unique_lock<std::mutex> lk(mtx);
    for ( auto& s : m ) {
        auto v = s.second;
        std::unique_lock<std::mutex> lk(v->mtx);
        v->q.push(n);
        v->cv.notify_one();
    }
    return 0;
}

void monitor_callback(void* context, tai_object_id_t oid, uint32_t attr_count, tai_attribute_t const * const attr_list) {
    if ( context == nullptr ) {
        return;
    }
    tai_notification_t notification;
    notification.oid = oid;
    auto type = tai_object_type_query(oid);

    for ( int i = 0; i < attr_count; i++ ) {
        auto a = attr_list[i];
        auto meta = tai_metadata_get_attr_metadata(type, a.id);
        if ( meta == nullptr ) {
            continue;
        }
        std::shared_ptr<tai_notification_elem_t> ptr(new tai_notification_elem_t, [](tai_notification_elem_t* n) {
            tai_metadata_free_attr_value(n->meta, &n->attr, nullptr);
        });
        ptr->meta = meta;
        ptr->attr.id = a.id;
        tai_alloc_info_t alloc_info;
        alloc_info.reference = &a;

        if ( tai_metadata_alloc_attr_value(meta, &ptr->attr, &alloc_info) != TAI_STATUS_SUCCESS ) {
            continue;
        }

        if ( tai_metadata_deepcopy_attr_value(meta, &a, &ptr->attr) != TAI_STATUS_SUCCESS ) {
            continue;
        }

        notification.attrs.emplace_back(ptr);
    }

    auto n = static_cast<TAINotifier*>(context);
    n->notify(notification);
}

::grpc::Status TAIServiceImpl::Monitor(::grpc::ServerContext* context, const taish::MonitorRequest* request, ::grpc::ServerWriter< taish::MonitorResponse>* writer) {
    auto oid = request->oid();
    auto nid = request->notification_attr_id();
    auto type = tai_object_type_query(oid);
    tai_attribute_t attr = {0};
    tai_status_t ret;
    tai_subscription_t s;
    std::shared_ptr<TAINotifier> notifier = nullptr;
    auto key = std::pair<tai_object_id_t, tai_attr_id_t>(oid, nid);

    auto meta = tai_metadata_get_attr_metadata(type, nid);
    if ( meta == nullptr ) {
        return Status(StatusCode::NOT_FOUND, "not found metadata");
    }

    if ( meta->attrvaluetype != TAI_ATTR_VALUE_TYPE_NOTIFICATION ) {
        return Status(StatusCode::INVALID_ARGUMENT, "value type is not notification");
    }


    {
        std::unique_lock<std::mutex> lk(m_mtx);

        attr.id = nid;

        switch (type) {
        case TAI_OBJECT_TYPE_NETWORKIF:
            ret = m_api->netif_api->get_network_interface_attribute(oid, &attr);
            break;
        case TAI_OBJECT_TYPE_HOSTIF:
            ret = m_api->hostif_api->get_host_interface_attribute(oid, &attr);
            break;
        case TAI_OBJECT_TYPE_MODULE:
            ret = m_api->module_api->get_module_attribute(oid, &attr);
            break;
        default:
            ret = TAI_STATUS_NOT_SUPPORTED;
        }

        if ( ret != TAI_STATUS_SUCCESS ) {
            add_status(context, ret);
            return Status::OK;
        }

        notifier = get_notifier(oid, nid);

        if ( attr.value.notification.notify == nullptr ) {
            attr.value.notification.notify = monitor_callback;
            attr.value.notification.context = notifier.get();
            switch (type) {
            case TAI_OBJECT_TYPE_NETWORKIF:
                ret = m_api->netif_api->set_network_interface_attribute(oid, &attr);
                break;
            case TAI_OBJECT_TYPE_HOSTIF:
                ret = m_api->hostif_api->set_host_interface_attribute(oid, &attr);
                break;
            case TAI_OBJECT_TYPE_MODULE:
                ret = m_api->module_api->set_module_attribute(oid, &attr);
                break;
            default:
                ret = TAI_STATUS_NOT_SUPPORTED;
            }
            if ( ret != TAI_STATUS_SUCCESS ) {
                add_status(context, ret);
                return Status::OK;
            }
        } else if ( attr.value.notification.notify != nullptr && notifier->size() == 0 ) {
            return Status(StatusCode::UNKNOWN, "notify attribute is set by others");
        }

        if ( notifier->subscribe(writer, &s) < 0 ) {
            return Status(StatusCode::UNKNOWN, "failed to subscribe");
        }
    }

    {

        std::unique_lock<std::mutex> lk(s.mtx);

        while(true) {
            std::chrono::seconds sec(1);
            s.cv.wait_for(lk, sec, [&]{ return !s.q.empty(); });

            if ( context->IsCancelled() ) {
                break;
            }

            {
                std::unique_lock<std::mutex> lk(m_mtx);
                if ( m_notifiers.find(key) == m_notifiers.end() ) {
                    return Status(StatusCode::UNKNOWN, "object is removed");
                }
            }

            if ( s.q.size() == 0 ) {
                continue;
            }

            auto n = s.q.front();
            s.q.pop();

            taish::MonitorResponse res;
            for ( auto e : n.attrs ) {
                auto a = res.add_attrs();
                a->set_attr_id(e->attr.id);
                std::string value;
                _serialize_attribute(e->meta, &e->attr, value);
                a->set_value(value);
            }

            if (!writer->Write(res)) {
                break;
            }
        }

    }

    {
        std::unique_lock<std::mutex> lk(m_mtx);

        if ( notifier->size() == 1 ) {
            attr.value.notification.notify = nullptr;
            attr.value.notification.context = nullptr;
            switch (type) {
            case TAI_OBJECT_TYPE_NETWORKIF:
                ret = m_api->netif_api->set_network_interface_attribute(oid, &attr);
                break;
            case TAI_OBJECT_TYPE_HOSTIF:
                ret = m_api->hostif_api->set_host_interface_attribute(oid, &attr);
                break;
            case TAI_OBJECT_TYPE_MODULE:
                ret = m_api->module_api->set_module_attribute(oid, &attr);
                break;
            default:
                ret = TAI_STATUS_NOT_SUPPORTED;
            }
            if ( ret != TAI_STATUS_SUCCESS ) {
                add_status(context, ret);
                return Status::OK;
            }
        }

        if ( notifier->desubscribe(writer) < 0 ) {
            return Status(StatusCode::UNKNOWN, "failed to desubscribe");
        }

        if ( notifier->size() == 0 ) {
            m_notifiers.erase(key);
        }

    }

    return Status::OK;
}

::grpc::Status TAIServiceImpl::SetLogLevel(::grpc::ServerContext* context, const taish::SetLogLevelRequest* request, taish::SetLogLevelResponse* response) {
    auto ret = tai_log_set(static_cast<tai_api_t>(request->api()), static_cast<tai_log_level_t>(request->level()), nullptr);
    add_status(context, ret);
    return Status::OK;
}

::grpc::Status TAIServiceImpl::Create(::grpc::ServerContext* context, const taish::CreateRequest* request, taish::CreateResponse* response) {
    auto object_type = request->object_type();
    auto mid = static_cast<tai_object_id_t>(request->module_id());
    tai_object_type_t type;
    std::function<tai_status_t(tai_object_id_t*, uint32_t, const tai_attribute_t*)> create;

    switch (object_type) {
    case taish::MODULE:
        type = TAI_OBJECT_TYPE_MODULE;
        create = m_api->module_api->create_module;
        break;
    case taish::NETIF:
        type = TAI_OBJECT_TYPE_NETWORKIF;
        create = std::bind(m_api->netif_api->create_network_interface, std::placeholders::_1, mid, std::placeholders::_2, std::placeholders::_3);
        break;
    case taish::HOSTIF:
        type = TAI_OBJECT_TYPE_HOSTIF;
        create = std::bind(m_api->hostif_api->create_host_interface, std::placeholders::_1, mid, std::placeholders::_2, std::placeholders::_3);
        break;
    default:
        return Status(StatusCode::INVALID_ARGUMENT, "unsupported object type");
    }

    std::vector<tai_attribute_t> attrs;
    ::grpc::Status status = Status::OK;
    tai_status_t ret;

    for ( auto i = 0; i < request->attrs_size(); i++ ) {
        auto a = request->attrs(i);
        auto id = a.attr_id();
        auto v = a.value();
        auto meta = tai_metadata_get_attr_metadata(type, id);
        tai_attribute_t attr = {0}, reference = {0};
        attr.id = id;
        tai_serialize_option_t option{true};
        tai_alloc_info_t alloc_info = { .list_size = 16 };
    again:
        if( tai_metadata_alloc_attr_value(meta, &attr, &alloc_info) != TAI_STATUS_SUCCESS ) {
            status = Status(StatusCode::UNKNOWN, "failed to alloc value");
            goto err;
        }
        ret = tai_deserialize_attribute_value(v.c_str(), meta, &attr.value, &option);
        if ( ret == TAI_STATUS_BUFFER_OVERFLOW && alloc_info.reference == nullptr ) {
            reference = attr;
            alloc_info.reference = &reference;
            goto again;
        }

        if ( alloc_info.reference != nullptr ) {
            if ( tai_metadata_free_attr_value(meta, &reference, nullptr) != TAI_STATUS_SUCCESS ) {
                status = Status(StatusCode::UNKNOWN, "failed to free value");
                goto err;
            }
        }

        if ( ret < 0 ) {
            status = Status(StatusCode::UNKNOWN, "failed to deserialize value");
            goto err;
        }

        attrs.emplace_back(attr);
    }

    tai_object_id_t oid;
    ret = create(&oid, attrs.size(), attrs.data());
    if ( ret == TAI_STATUS_SUCCESS ) {
        m_api->object_update(type, oid, true);
    }
err:
    for ( auto& attr : attrs ) {
        auto meta = tai_metadata_get_attr_metadata(type, attr.id);
        if ( tai_metadata_free_attr_value(meta, &attr, nullptr) != TAI_STATUS_SUCCESS ) {
            status = Status(StatusCode::UNKNOWN, "failed to free value");
        }
    }
    add_status(context, ret);
    return status;
}

::grpc::Status TAIServiceImpl::Remove(::grpc::ServerContext* context, const taish::RemoveRequest* request, taish::RemoveResponse* response) {
    auto oid = request->oid();
    auto type = tai_object_type_query(oid);
    tai_status_t ret;

    switch (type) {
    case TAI_OBJECT_TYPE_MODULE:
        ret = m_api->module_api->remove_module(oid);
        break;
    case TAI_OBJECT_TYPE_NETWORKIF:
        ret = m_api->netif_api->remove_network_interface(oid);
        break;
    case TAI_OBJECT_TYPE_HOSTIF:
        ret = m_api->hostif_api->remove_host_interface(oid);
        break;
    default:
        ret = TAI_STATUS_NOT_SUPPORTED;
    }
    if ( ret == TAI_STATUS_SUCCESS ) {
        {
            std::unique_lock<std::mutex> lk(m_mtx);
            auto it = m_notifiers.begin();
            while  ( it != m_notifiers.end() ) {
                if ( it->first.first == oid ) {
                    it = m_notifiers.erase(it);
                } else {
                    it++;
                }
            }
        }
        m_api->object_update(type, oid, false);
    }
    add_status(context, ret);
    return Status::OK;
}
