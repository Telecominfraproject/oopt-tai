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
#include "attribute.hpp"
#include "capability.hpp"

using grpc::Status;
using grpc::StatusCode;

static const std::string _serialize_status(const tai_status_t status) {
    tai_serialize_option_t option{ .human = true, .valueonly = true, .json = false};
    size_t size = 64;
    char buf[size];
    auto ret = tai_serialize_status(buf, size, status, &option);
    if ( ret < 0 ) {
        return "unknown";
    }
    return std::string(buf, ret);
}

static void add_status(::grpc::ServerContext* context, tai_status_t status) {
    context->AddTrailingMetadata("tai-status-code", std::to_string(status));
    context->AddTrailingMetadata("tai-status-msg", _serialize_status(status));
}

const tai_attr_metadata_t* const get_metadata(tai_meta_api_t* meta_api, const tai_metadata_key_t * const key, tai_attr_id_t attr_id) {
    if ( meta_api != nullptr && meta_api->get_attr_metadata != nullptr) {
        return meta_api->get_attr_metadata(key, attr_id);
    }
    auto type = key->type;
    if ( key->oid != TAI_NULL_OBJECT_ID ) {
        type = tai_object_type_query(key->oid);
    }
    return tai_metadata_get_attr_metadata(type, attr_id);
}

::grpc::Status TAIServiceImpl::ListModule(::grpc::ServerContext* context, const taish::ListModuleRequest* request, ::grpc::ServerWriter< taish::ListModuleResponse>* writer) {

    std::vector<tai_api_module_t> list;
    auto ret = m_api->list_module(list);
    if ( ret != TAI_STATUS_SUCCESS ) {
        goto err;
    }
    for ( const auto& module : list ) {
        auto res = taish::ListModuleResponse();
        auto m = res.mutable_module();

        m->set_location(module.location);
        m->set_present(module.present);
        m->set_oid(module.id);

        if ( module.id != TAI_NULL_OBJECT_ID ) {
            m->clear_hostifs();
            for ( uint32_t i = 0; i < module.hostifs.size(); i++ ) {
                auto hostif = m->add_hostifs();
                hostif->set_index(i);
                hostif->set_oid(module.hostifs[i]);
                hostif->set_module_oid(module.id);
            }
            m->clear_netifs();
            for ( uint32_t i = 0; i < module.netifs.size(); i++ ) {
                auto netif = m->add_netifs();
                netif->set_index(i);
                netif->set_oid(module.netifs[i]);
                netif->set_module_oid(module.id);
            }
        }
        writer->Write(res);
    }
err:
    add_status(context, ret);
    return Status::OK;
}

static void usage(const tai_attr_metadata_t* meta, std::string* str) {
    if ( meta->isenum && meta->enummetadata != nullptr ) {
        auto m = meta->enummetadata;
        *str = "[";
        for ( uint32_t i = 0; i < m->valuescount; i++ ) {
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
    dst->set_is_enum(src->isenum);
}

static void convert_capability(tai_serialize_option_t option, const tai_attr_metadata_t* const meta, const tai_attribute_capability_t* const src, taish::AttributeCapability* const dst) {
    dst->set_attr_id(src->id);
    if ( src->valid_defaultvalue ) {
        tai_attribute_t a{.id = meta->attrid, .value = src->defaultvalue};
        auto attr = std::make_unique<tai::Attribute>(meta, a);
        auto v = attr->to_string(&option);
        dst->set_default_value(v);
    }
    if ( src->valid_min ) {
        tai_attribute_t a{.id = meta->attrid, .value = src->min};
        auto attr = std::make_unique<tai::Attribute>(meta, a);
        auto v = attr->to_string(&option);
        dst->set_min(v);
    }
    if ( src->valid_max ) {
        tai_attribute_t a{.id = meta->attrid, .value = src->max};
        auto attr = std::make_unique<tai::Attribute>(meta, a);
        auto v = attr->to_string(&option);
        dst->set_max(v);
    }
    if ( src->valid_supportedvalues ) {
        for ( int i = 0; i < static_cast<int>(src->supportedvalues.count); i++ ) {
            tai_attribute_t a{.id = meta->attrid, .value = src->supportedvalues.list[i]};
            auto attr = std::make_unique<tai::Attribute>(meta, a);
            auto v = attr->to_string(&option);
            dst->add_supportedvalues(v);
        }
    }
}

static tai_serialize_option_t convert_serialize_option(const taish::SerializeOption& src) {
    tai_serialize_option_t dst;
    dst.human = src.human();
    dst.valueonly = src.value_only();
    dst.json = src.json();
    return dst;
}

::grpc::Status TAIServiceImpl::ListAttributeMetadata(::grpc::ServerContext* context, const taish::ListAttributeMetadataRequest* request, ::grpc::ServerWriter< taish::ListAttributeMetadataResponse>* writer) {
    auto res = taish::ListAttributeMetadataResponse();
    auto object_type = request->object_type();
    auto info = tai_metadata_all_object_type_infos[object_type];
    auto oid = request->oid();
    uint32_t count;
    tai_attr_metadata_t const * const *list;

    if ( oid != TAI_NULL_OBJECT_ID && m_api->meta_api != nullptr ) {
        tai_metadata_key_t key{.oid = oid};
        auto ret = m_api->meta_api->list_metadata(&key, &count, &list);
        if ( ret < 0 ) {
            add_status(context, ret);
            return Status::OK;
        }
    } else if ( info == nullptr ) {
        count = tai_metadata_attr_sorted_by_id_name_count;
        list = tai_metadata_attr_sorted_by_id_name;
    } else {
        count = info->attrmetadatalength;
        list = info->attrmetadata;
    }

    for ( uint32_t i = 0; i < count; i++ ) {
        auto src = list[i];
        auto dst = res.mutable_metadata();
        convert_metadata(src, dst);
        writer->Write(res);
    }

    return Status::OK;
}

::grpc::Status TAIServiceImpl::GetAttributeMetadata(::grpc::ServerContext* context, const taish::GetAttributeMetadataRequest* request, taish::GetAttributeMetadataResponse* response) {
    auto object_type = request->object_type();
    int32_t attr_id = 0;
    auto oid = request->oid();
    auto l = request->location();
    tai_char_list_t location{.count = static_cast<uint32_t>(l.size()), .list = const_cast<char*>(l.c_str())};
    tai_metadata_key_t key{.oid = oid, .type = static_cast<tai_object_type_t>(object_type), .location = location};

    if ( request->attr_name() != "" ) {
        auto attr_name = request->attr_name();
        int ret;
        auto option = convert_serialize_option(request->serialize_option());

        if ( m_api->meta_api != nullptr ) {
            auto info = m_api->meta_api->get_object_info(&key);
            if ( info == nullptr ) {
                add_status(context, TAI_STATUS_NOT_SUPPORTED);
                return Status::OK;
            }
            ret = tai_deserialize_enum(attr_name.c_str(), info->enummetadata, &attr_id, &option);
        } else {
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
        }

        if ( ret < 0 ) {
            add_status(context, ret);
            return Status::OK;
        }
    } else {
        attr_id = request->attr_id();
    }

    auto meta = get_metadata(m_api->meta_api, &key, attr_id);
    if ( meta == nullptr ) {
        return Status(StatusCode::NOT_FOUND, "not found metadata");
    }
    auto res = response->mutable_metadata();
    convert_metadata(meta, res);
    add_status(context, TAI_STATUS_SUCCESS);
    return Status::OK;
}


::grpc::Status TAIServiceImpl::GetAttributeCapability(::grpc::ServerContext* context, const taish::GetAttributeCapabilityRequest* request, taish::GetAttributeCapabilityResponse* response) {
    auto oid = request->oid();
    tai_attr_id_t attr_id = request->attr_id();
    auto type = tai_object_type_query(oid);
    tai_metadata_key_t key{.oid = oid};
    tai_status_t ret = TAI_STATUS_SUCCESS;
    auto meta = get_metadata(m_api->meta_api, &key, attr_id);
    if ( meta == nullptr ) {
        return Status(StatusCode::NOT_FOUND, "not found metadata");
    }

    auto getter = [&](tai_attribute_capability_t* cap) -> tai_status_t {
        switch (type) {
        case TAI_OBJECT_TYPE_MODULE:
            if ( m_api->module_api->get_module_capability == nullptr ) {
                return TAI_STATUS_NOT_SUPPORTED;
            }
            return m_api->module_api->get_module_capability(oid, cap);
        case TAI_OBJECT_TYPE_NETWORKIF:
            if ( m_api->netif_api->get_network_interface_capability == nullptr ) {
                return TAI_STATUS_NOT_SUPPORTED;
            }
            return m_api->netif_api->get_network_interface_capability(oid, cap);
        case TAI_OBJECT_TYPE_HOSTIF:
            if ( m_api->hostif_api->get_host_interface_capability == nullptr ) {
                return TAI_STATUS_NOT_SUPPORTED;
            }
            return m_api->hostif_api->get_host_interface_capability(oid, cap);
        default:
            return TAI_STATUS_NOT_SUPPORTED;
        }
    };

    try {
        auto cap = std::make_unique<tai::Capability>(meta, getter);
        auto option = convert_serialize_option(request->serialize_option());
        convert_capability(option, meta, cap->raw(), response->mutable_capability());
    } catch (tai::Exception& e) {
        ret = e.err();
    }
    add_status(context, ret);
    return Status::OK;
}

::grpc::Status TAIServiceImpl::GetAttribute(::grpc::ServerContext* context, const taish::GetAttributeRequest* request, taish::GetAttributeResponse* response) {
    auto oid = request->oid();
    taish::Attribute a = request->attribute();
    taish::Attribute* res;
    auto id = a.attr_id();
    auto value = a.value();
    auto type = tai_object_type_query(oid);
    tai_metadata_key_t key{.oid = oid};
    auto meta = get_metadata(m_api->meta_api, &key, id);
    auto option = convert_serialize_option(request->serialize_option());

    auto getter = [&](tai_attribute_t* attr) -> tai_status_t {

        if ( value.size() > 0 ) {
            auto ret = tai_deserialize_attribute_value(value.c_str(), meta, &attr->value, &option);
            if ( ret < 0 ) {
                return ret;
            }
        }

        std::unique_lock<std::mutex> lk(m_mtx);

        switch (type) {
        case TAI_OBJECT_TYPE_MODULE:
            return m_api->module_api->get_module_attribute(oid, attr);
        case TAI_OBJECT_TYPE_NETWORKIF:
            return m_api->netif_api->get_network_interface_attribute(oid, attr);
        case TAI_OBJECT_TYPE_HOSTIF:
            return m_api->hostif_api->get_host_interface_attribute(oid, attr);
        default:
            return TAI_STATUS_NOT_SUPPORTED;
        }
    };

    auto ret = TAI_STATUS_SUCCESS;
    try {
        auto attr = std::make_unique<tai::Attribute>(meta, getter);
        auto v = attr->to_string(&option);
        res = response->mutable_attribute();
        res->set_attr_id(id);
        res->set_value(v);
    } catch (tai::Exception& e) {
        ret = e.err();
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
    tai_metadata_key_t key{.oid = oid};
    auto meta = get_metadata(m_api->meta_api, &key, id);
    auto option = convert_serialize_option(request->serialize_option());

    auto ret = TAI_STATUS_SUCCESS;
    try {
        auto attr = std::make_unique<tai::Attribute>(meta, v, &option);
        std::unique_lock<std::mutex> lk(m_mtx);
        switch (type) {
        case TAI_OBJECT_TYPE_MODULE:
            ret = m_api->module_api->set_module_attribute(oid, attr->raw());
            break;
        case TAI_OBJECT_TYPE_NETWORKIF:
            ret = m_api->netif_api->set_network_interface_attribute(oid, attr->raw());
            break;
        case TAI_OBJECT_TYPE_HOSTIF:
            ret = m_api->hostif_api->set_host_interface_attribute(oid, attr->raw());
            break;
        default:
            ret = TAI_STATUS_NOT_SUPPORTED;
        }
    } catch (tai::Exception& e) {
        ret = e.err();
    }
    add_status(context, ret);
    return Status::OK;
}

::grpc::Status TAIServiceImpl::ClearAttribute(::grpc::ServerContext* context, const taish::ClearAttributeRequest* request, taish::ClearAttributeResponse* response) {
    auto oid = request->oid();
    auto id = request->attr_id();
    auto type = tai_object_type_query(oid);
    tai_status_t ret;
    std::unique_lock<std::mutex> lk(m_mtx);

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
    tai_metadata_key_t key{.oid = oid};
    auto n = static_cast<TAINotifier*>(context);

    for ( uint32_t i = 0; i < attr_count; i++ ) {
        auto meta = get_metadata(n->meta_api(), &key, attr_list[i].id);
        if ( meta == nullptr ) {
            continue;
        }
        notification.attrs.emplace_back(std::make_shared<tai::Attribute>(meta, &attr_list[i]));
    }

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
    auto option = convert_serialize_option(request->serialize_option());

    tai_metadata_key_t k{.oid = oid};
    auto meta = get_metadata(m_api->meta_api, &k, nid);
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
                std::unique_lock<std::mutex> lk(m_notifiers_mtx);
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
                a->set_attr_id(e->id());
                a->set_value(e->to_string(&option));
            }

            if (!writer->Write(res)) {
                break;
            }
        }

    }

    {
        std::unique_lock<std::mutex> lk(m_notifiers_mtx);

        if ( notifier->size() == 1 ) {
            std::unique_lock<std::mutex> lk(m_mtx);
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

    std::vector<tai::S_Attribute> attrs;
    auto option = convert_serialize_option(request->serialize_option());

    tai_metadata_key_t key{.type = TAI_OBJECT_TYPE_MODULE};
    auto meta = get_metadata(m_api->meta_api, &key, TAI_MODULE_ATTR_LOCATION);
    tai::S_Attribute loc;
    if ( type != TAI_OBJECT_TYPE_MODULE ) {
        std::unique_lock<std::mutex> lk(m_mtx);
        auto getter = [&](tai_attribute_t* attr) -> tai_status_t {
            return m_api->module_api->get_module_attribute(mid, attr);
        };
        try {
            loc = std::make_unique<tai::Attribute>(meta, getter);
        } catch ( tai::Exception& e ) {
            add_status(context, e.err());
            return Status::OK;
        }
    } else {
        for ( auto i = 0; i < request->attrs_size(); i++ ) {
            auto a = request->attrs(i);
            auto id = a.attr_id();
            if ( id == TAI_MODULE_ATTR_LOCATION ) {
                loc = std::make_unique<tai::Attribute>(meta, a.value());
                break;
            }
        }
        if ( !loc ) {
            add_status(context, TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING);
            return Status::OK;
        }
    }

    try {
        for ( auto i = 0; i < request->attrs_size(); i++ ) {
            auto a = request->attrs(i);
            auto id = a.attr_id();
            auto v = a.value();
            tai_metadata_key_t key{.type = type, .location = loc->raw()->value.charlist};
            auto meta = get_metadata(m_api->meta_api, &key, id);
            auto attr = std::make_shared<tai::Attribute>(meta, v, &option);
            attrs.emplace_back(attr);
        }
    } catch (tai::Exception& e) {
        add_status(context, e.err());
        return Status::OK;
    }

    tai_object_id_t oid;
    std::vector<tai_attribute_t> list;
    for ( auto& v : attrs ) {
        list.emplace_back(*(v->raw()));
    }

    tai_status_t ret;

    {
        std::unique_lock<std::mutex> lk(m_mtx);
        ret = create(&oid, list.size(), list.data());
    }

    if ( ret == TAI_STATUS_SUCCESS ) {
        response->set_oid(oid);
        m_api->object_update(type, oid, true);
    }
    add_status(context, ret);
    return Status::OK;
}

::grpc::Status TAIServiceImpl::Remove(::grpc::ServerContext* context, const taish::RemoveRequest* request, taish::RemoveResponse* response) {
    auto oid = request->oid();
    auto type = tai_object_type_query(oid);
    tai_status_t ret;
    {
        std::unique_lock<std::mutex> lk(m_mtx);
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
    }

    if ( ret == TAI_STATUS_SUCCESS ) {
        std::unique_lock<std::mutex> lk(m_notifiers_mtx);
        auto it = m_notifiers.begin();
        while  ( it != m_notifiers.end() ) {
            if ( it->first.first == oid ) {
                auto notifier = it->second;
                tai_notification_t empty;
                it = m_notifiers.erase(it);
                notifier->notify(empty); // signal the deletion
            } else {
                it++;
            }
        }
        m_api->object_update(type, oid, false);
    }
    add_status(context, ret);
    return Status::OK;
}
