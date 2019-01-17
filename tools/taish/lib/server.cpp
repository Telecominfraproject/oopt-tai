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

using grpc::Status;
using grpc::StatusCode;

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
    auto id = request->attr_id();
    auto type = tai_object_type_query(oid);
    auto meta = tai_metadata_get_attr_metadata(type, id);
    tai_attribute_t attr = {0};
    attr.id = id;
    tai_serialize_option_t option{true, true};
    char buf[128] = {0};
    auto a = response->mutable_attribute();

    if( tai_metadata_alloc_attr_value(meta, &attr, nullptr) != TAI_STATUS_SUCCESS ) {
        return Status(StatusCode::UNKNOWN, "failed to alloc value");
    }

    tai_status_t ret;
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

    if ( ret != TAI_STATUS_SUCCESS ) {
        goto err;
    }

    if ( (ret = tai_serialize_attribute(buf, meta, &attr, &option)) < 0 ) {
        goto err;
    }

    a->set_value(buf);
err:
    if ( tai_metadata_free_attr_value(meta, &attr, nullptr) != TAI_STATUS_SUCCESS ) {
        return Status(StatusCode::UNKNOWN, "failed to free value");
    }
    if ( ret < 0 ) {
        std::stringstream ss;
        ss << "failed to get attribute(" << id << "): ret:" << ret;
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
    if ( ret == 0 ) {
        return Status::OK;
    }
    return Status(StatusCode::UNKNOWN, "failed to set attribute");
}
