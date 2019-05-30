/**
 * @file    server.cpp
 *
 * @brief   This module implements TAI gRPC server
 *
 * @copyright Copyright (c) 2018 Nippon Telegraph and Telephone Corporation
 *
 * @remark  Licensed under the Apache License, Version 2.0 (the "License"); you
 *          may not use this file except in compliance with the License. You may
 *          obtain a copy of the License at
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 * @remark  THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 *          CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *          LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 *          FOR A PARTICULAR PURPOSE, MERCHANTABILITY OR NON-INFRINGEMENT.
 *
 * @remark  See the Apache Version 2.0 License for specific language governing
 *          permissions and limitations under the License.
 *
 */

#include "taigrpc.hpp"
#include "taimetadata.h"
#include <sstream>
#include <chrono>

using grpc::Status;
using grpc::StatusCode;

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

::grpc::Status TAIServiceImpl::ListModule(::grpc::ServerContext* context, const ::tai::ListModuleRequest* request, ::grpc::ServerWriter< ::tai::ListModuleResponse>* writer) {
    auto res = ::tai::ListModuleResponse();

    TAIAPIModuleList l;
    auto list = l.list();
    auto ret = m_api->list_module(list);
    if ( ret != TAI_STATUS_SUCCESS ) {
        return Status(StatusCode::UNKNOWN, "failed to get module list");
    }
    auto meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_MODULE, TAI_MODULE_ATTR_LOCATION);
    tai_attribute_t attr = {0};
    if( tai_metadata_alloc_attr_value(meta, &attr, nullptr) != TAI_STATUS_SUCCESS ) {
        return Status(StatusCode::UNKNOWN, "failed to alloc value");
    }

    for ( auto i = 0; i < list->count; i++ ) {
        auto m = res.mutable_module();
        auto module = list->list[i];
        m->set_oid(module.id);
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
        writer->Write(res);
    }

err:
    if ( tai_metadata_free_attr_value(meta, &attr, nullptr) != TAI_STATUS_SUCCESS ) {
        return Status(StatusCode::UNKNOWN, "failed to free value");
    }
    if ( ret == 0 ) {
        return Status::OK;
    }
    return Status(StatusCode::UNKNOWN, "failed to get module location");
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
    switch (meta->attrvaluetype) {
    case TAI_ATTR_VALUE_TYPE_BOOLDATA:
        *str = "<bool>";
        break;
    case TAI_ATTR_VALUE_TYPE_CHARDATA:
        *str = "<chardata>";
        break;
    case TAI_ATTR_VALUE_TYPE_U8:
        *str = "<uint8>";
        break;
    case TAI_ATTR_VALUE_TYPE_S8:
        *str = "<int8>";
        break;
    case TAI_ATTR_VALUE_TYPE_U16:
        *str = "<uint16>";
        break;
    case TAI_ATTR_VALUE_TYPE_S16:
        *str = "<int16>";
        break;
    case TAI_ATTR_VALUE_TYPE_U32:
        *str = "<uint32>";
        break;
    case TAI_ATTR_VALUE_TYPE_S32:
        *str = "<int32>";
        break;
    case TAI_ATTR_VALUE_TYPE_U64:
        *str = "<uint64>";
        break;
    case TAI_ATTR_VALUE_TYPE_S64:
        *str = "<int64>";
        break;
    case TAI_ATTR_VALUE_TYPE_FLT:
        *str = "<float>";
        break;
    case TAI_ATTR_VALUE_TYPE_PTR:
        *str = "<pointer>";
        break;
    case TAI_ATTR_VALUE_TYPE_OID:
        *str = "<oid>";
        break;
    case TAI_ATTR_VALUE_TYPE_OBJLIST:
        *str = "<oid list>";
        break;
    case TAI_ATTR_VALUE_TYPE_CHARLIST:
        *str = "<string>";
        break;
    case TAI_ATTR_VALUE_TYPE_U8LIST:
        *str = "<uint8 list>";
        break;
    case TAI_ATTR_VALUE_TYPE_S8LIST:
        *str = "<int8 list>";
        break;
    case TAI_ATTR_VALUE_TYPE_U16LIST:
        *str = "<uint16 list>";
        break;
    case TAI_ATTR_VALUE_TYPE_S16LIST:
        *str = "<int16 list>";
        break;
    case TAI_ATTR_VALUE_TYPE_U32LIST:
        *str = "<uint32 list>";
        break;
    case TAI_ATTR_VALUE_TYPE_S32LIST:
        *str = "<int32 list>";
        break;
    case TAI_ATTR_VALUE_TYPE_FLOATLIST:
        *str = "<flaot list>";
        break;
    case TAI_ATTR_VALUE_TYPE_U32RANGE:
        *str = "<uint32 range>";
        break;
    case TAI_ATTR_VALUE_TYPE_S32RANGE:
        *str = "<int32 range>";
        break;
    case TAI_ATTR_VALUE_TYPE_OBJMAPLIST:
        *str = "<oid map list>";
        break;
    case TAI_ATTR_VALUE_TYPE_ATTRLIST:
        *str = "<attribute list>";
        break;
    default:
        *str = "<unknown>";
    }
}

static void convert_metadata(const tai_attr_metadata_t* const src, ::tai::AttributeMetadata* const dst) {
    dst->set_attr_id(src->attrid);
    dst->set_name(src->attridname);
    dst->set_short_name(src->attridshortname);
    dst->set_object_type(static_cast<tai::TAIObjectType>(src->objecttype));
    auto u = dst->mutable_usage();
    usage(src, u);
    dst->set_is_readonly(src->isreadonly);
    dst->set_is_mandatoryoncreate(src->ismandatoryoncreate);
    dst->set_is_createonly(src->iscreateonly);
    dst->set_is_createandset(src->iscreateandset);
    dst->set_is_key(src->iskey);
}

::grpc::Status TAIServiceImpl::ListAttributeMetadata(::grpc::ServerContext* context, const ::tai::ListAttributeMetadataRequest* request, ::grpc::ServerWriter< ::tai::ListAttributeMetadataResponse>* writer) {
    auto res = ::tai::ListAttributeMetadataResponse();
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

::grpc::Status TAIServiceImpl::GetAttributeMetadata(::grpc::ServerContext* context, const ::tai::GetAttributeMetadataRequest* request, ::tai::GetAttributeMetadataResponse* response) {
    auto object_type = request->object_type();
    int32_t attr_id = 0;
    if ( request->attr_name() != "" ) {
        auto attr_name = request->attr_name();
        int ret;
        tai_serialize_option_t option{true};
        switch (object_type) {
        case ::tai::MODULE:
            ret = tai_deserialize_module_attr(attr_name.c_str(), &attr_id, &option);
            break;
        case ::tai::NETIF:
            ret = tai_deserialize_network_interface_attr(attr_name.c_str(), &attr_id, &option);
            break;
        case ::tai::HOSTIF:
            ret = tai_deserialize_host_interface_attr(attr_name.c_str(), &attr_id, &option);
            break;
        default:
            return Status(StatusCode::INVALID_ARGUMENT, "unsupported object type");
        }

        if ( ret < 0 ) {
            return Status(StatusCode::INVALID_ARGUMENT, "failed to deserialize attr name");
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
    return Status::OK;
}

::grpc::Status TAIServiceImpl::GetAttribute(::grpc::ServerContext* context, const ::tai::GetAttributeRequest* request, ::tai::GetAttributeResponse* response) {
    auto oid = request->oid();
    ::tai::Attribute a = request->attribute();
    ::tai::Attribute* res;
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
        if ( ret < 0 ) {
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
    if ( ret < 0 ) {
        std::stringstream ss;
        ss << "failed to get attribute(" << std::hex << id << "): ret:" << -ret;
        return Status(StatusCode::UNKNOWN, ss.str());
    }
    return Status::OK;
}

::grpc::Status TAIServiceImpl::SetAttribute(::grpc::ServerContext* context, const ::tai::SetAttributeRequest* request, ::tai::SetAttributeResponse* response) {
    auto oid = request->oid();
    auto a = request->attribute();
    auto id = a.attr_id();
    auto v = a.value();
    auto type = tai_object_type_query(oid);
    auto meta = tai_metadata_get_attr_metadata(type, id);
    tai_attribute_t attr = {0};
    attr.id = id;
    tai_serialize_option_t option{true};
    if( tai_metadata_alloc_attr_value(meta, &attr, nullptr) != TAI_STATUS_SUCCESS ) {
        return Status(StatusCode::UNKNOWN, "failed to alloc value");
    }

    auto ret = tai_deserialize_attribute_value(v.c_str(), meta, &attr.value, &option);
    if ( ret < 0 ) {
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
    if ( ret < 0 ) {
        std::stringstream ss;
        ss << "failed to set attribute(" << std::hex << id << "): ret:" << -ret;
        return Status(StatusCode::UNKNOWN, ss.str());
    }
    return Status::OK;
}

int TAINotifier::notify(const tai_notification_t& n) {
    auto oid = n.oid;
    auto type = tai_object_type_query(oid);
    auto meta = tai_metadata_get_attr_metadata(type, n.attr->id);
    tai_alloc_info_t alloc_info;
    alloc_info.reference = n.attr;

    std::unique_lock<std::mutex> lk(mtx);
    for ( auto& s : m ) {
        tai_attribute_t *attr = new tai_attribute_t();
        attr->id = n.attr->id;

        if ( tai_metadata_alloc_attr_value(meta, attr, &alloc_info) != TAI_STATUS_SUCCESS ) {
            return -1;
        }

        if ( tai_metadata_deepcopy_attr_value(meta, n.attr, attr) != TAI_STATUS_SUCCESS ) {
            return -1;
        }

        tai_notification_t nn{ oid, attr };

        auto v = s.second;
        std::unique_lock<std::mutex> lk(v->mtx);
        v->q.push(nn);
        v->cv.notify_one();
    }
    return 0;
}

void monitor_callback(void* context, tai_object_id_t oid, tai_attribute_t const * const attribute) {
    if ( context == nullptr ) {
        return;
    }
    auto impl = static_cast<TAIServiceImpl*>(context);
    impl->notify(oid, attribute);
}

void TAIServiceImpl::notify(tai_object_id_t oid, tai_attribute_t const * const attribute) {
    std::unique_lock<std::mutex> lk(m_mtx);
    auto n = m_notifiers[oid];
    if ( n != nullptr ) {
        n->notify(tai_notification_t{oid, attribute});
    }
}

::grpc::Status TAIServiceImpl::Monitor(::grpc::ServerContext* context, const ::tai::MonitorRequest* request, ::grpc::ServerWriter< ::tai::MonitorResponse>* writer) {
    auto oid = request->oid();
    auto type = tai_object_type_query(oid);
    tai_attribute_t attr = {0};
    tai_status_t ret;

    switch (type) {
    case TAI_OBJECT_TYPE_NETWORKIF:
        attr.id = TAI_NETWORK_INTERFACE_ATTR_NOTIFY;
        ret = m_api->netif_api->get_network_interface_attribute(oid, &attr);
        break;
    default:
        return Status(StatusCode::UNKNOWN, "unsupported object type");
    }

    if ( ret != TAI_STATUS_SUCCESS ) {
        std::stringstream ss;
        ss << "failed to get notify attribute: ret:" << std::hex << -ret;
        return Status(StatusCode::UNKNOWN, ss.str());
    }

    auto notifier = get_notifier(oid);

    if ( attr.value.notification.notify == nullptr ) {
        attr.value.notification.notify = monitor_callback;
        attr.value.notification.context = this;
        ret = m_api->netif_api->set_network_interface_attribute(oid, &attr);
        if ( ret != TAI_STATUS_SUCCESS ) {
            std::stringstream ss;
            ss << "failed to set notify attribute: ret:" << std::hex << -ret;
            return Status(StatusCode::UNKNOWN, ss.str());
        }
    } else if ( attr.value.notification.notify != nullptr && notifier->size() == 0 ) {
        return Status(StatusCode::UNKNOWN, "notify attribute is set by others");
    }

    tai_subscription_t s;

    if ( notifier->subscribe(writer, &s) < 0 ) {
        return Status(StatusCode::UNKNOWN, "failed to subscribe");
    }

    std::unique_lock<std::mutex> lk(s.mtx);
    while(true) {
        std::chrono::seconds sec(1);
        auto pred = s.cv.wait_for(lk, sec, [&]{ return !s.q.empty(); });
        if ( !pred ) { // queue is empty
            if ( context->IsCancelled() ) {
                break;
            }
            // writer is still alive continue
            continue;
        }
        auto n = s.q.front();
        s.q.pop();


        ::tai::MonitorResponse res;
        auto a = res.mutable_attribute();
        auto meta = tai_metadata_get_attr_metadata(type, n.attr->id);

        a->set_attr_id(n.attr->id);
        std::string value;
        _serialize_attribute(meta, n.attr, value);
        a->set_value(value);

        tai_attribute_t *attr = const_cast<tai_attribute_t*>(n.attr);
        if ( tai_metadata_free_attr_value(meta, attr, nullptr) < 0 ) {
            return Status(StatusCode::UNKNOWN, "failed to free attribute");
        }

        delete attr;

        if (!writer->Write(res)) {
            break;
        }
    }

    if ( notifier->desubscribe(writer) < 0 ) {
        return Status(StatusCode::UNKNOWN, "failed to desubscribe");
    }

    if ( notifier->size() == 0 ) {
        std::unique_lock<std::mutex> lk(m_mtx);
        delete notifier;
        m_notifiers.erase(oid);
        attr.value.notification.notify = nullptr;
        attr.value.notification.context = nullptr;
        ret = m_api->netif_api->set_network_interface_attribute(oid, &attr);
        if ( ret != TAI_STATUS_SUCCESS ) {
            std::stringstream ss;
            ss << "failed to clear notify attribute: ret:" << std::hex << -ret;
            return Status(StatusCode::UNKNOWN, ss.str());
        }
    }

    return Status::OK;
}

::grpc::Status TAIServiceImpl::SetLogLevel(::grpc::ServerContext* context, const ::tai::SetLogLevelRequest* request, ::tai::SetLogLevelResponse* response) {
    auto ret = tai_log_set(static_cast<tai_api_t>(request->api()), static_cast<tai_log_level_t>(request->level()));
    if ( ret != TAI_STATUS_SUCCESS ) {
        return Status(StatusCode::UNKNOWN, "failed to set loglevel");
    }
    return Status::OK;
}
