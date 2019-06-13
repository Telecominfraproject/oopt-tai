/**
 * @file    taigrpc.hpp
 *
 * @brief   This module defines TAI gRPC server
 *
 * @copyright Copyright (c) 2018 Nippon Telegraph and Telephone Corporation
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef __TAIGRPC_HPP__
#define __TAIGRPC_HPP__

#include "tai.h"
#include "tai.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <map>
#include <iostream>
#include <string>
#include <memory>

struct tai_api_module_t
{
    std::string       location;
    bool              present;
    tai_object_id_t   id;
    tai_object_list_t hostifs;
    tai_object_list_t netifs;
};

struct tai_api_module_list_t
{
    uint32_t count;
    tai_api_module_t *list;
};

typedef tai_status_t (*tai_api_list_module_fn)(_Inout_ tai_api_module_list_t* const list);

// this hook function will be called when taish library created or removed a TAI object
// type : the object type, we can't use tai_object_type_query() since the object with the oid is
//        already removed when this function is called
// oid : the object id of the created or removed object
// is_create : a flag to indicated whether the update is create or remove
typedef void (*tai_object_update_fn)(tai_object_type_t type, tai_object_id_t oid, bool is_create);

struct tai_api_method_table_t
{
    tai_module_api_t* module_api;
    tai_host_interface_api_t* hostif_api;
    tai_network_interface_api_t* netif_api;
    tai_api_list_module_fn list_module;
    tai_object_update_fn object_update;
};

class TAIAPIModuleList {
    public:
        TAIAPIModuleList(uint32_t module_size = 8, uint32_t hostif_size = 2, uint32_t netif_size = 1);
        ~TAIAPIModuleList();
        tai_api_module_list_t* const list() { return &m_list; }
    private:
        tai_api_module_list_t m_list;
        uint32_t m_module_size;
        uint32_t m_hostif_size;
        uint32_t m_netif_size;
};

struct tai_notification_t {
    tai_object_id_t oid;
    tai_attribute_t const * const attr;
};

struct tai_subscription_t {
    std::mutex mtx;
    std::queue<tai_notification_t> q;
    std::condition_variable cv;
};

class TAINotifier {
    public:
        TAINotifier() {};
        ~TAINotifier();
        int notify(const tai_notification_t& n);
        int subscribe(void* id, tai_subscription_t* s) {
            std::unique_lock<std::mutex> lk(mtx);
            if ( m.find(id) != m.end() ) {
                return -1;
            }
            m[id] = s;
            return 0;
        }
        int desubscribe(void* id) {
            std::unique_lock<std::mutex> lk(mtx);
            if ( m.find(id) == m.end() ) {
                return -1;
            }
            m.erase(id);
            return 0;
        }
        int size() {
            std::unique_lock<std::mutex> lk(mtx);
            return m.size();
        }
    private:
        std::map<void*, tai_subscription_t*> m;
        std::mutex mtx;
};

class TAIServiceImpl final : public tai::TAI::Service {
    public:
        TAIServiceImpl(const tai_api_method_table_t* const api) : m_api(api) {};
        ::grpc::Status ListModule(::grpc::ServerContext* context, const ::tai::ListModuleRequest* request, ::grpc::ServerWriter< ::tai::ListModuleResponse>* writer);
        ::grpc::Status ListAttributeMetadata(::grpc::ServerContext* context, const ::tai::ListAttributeMetadataRequest* request, ::grpc::ServerWriter< ::tai::ListAttributeMetadataResponse>* writer);
        ::grpc::Status GetAttributeMetadata(::grpc::ServerContext* context, const ::tai::GetAttributeMetadataRequest* request, ::tai::GetAttributeMetadataResponse* response);
        ::grpc::Status GetAttribute(::grpc::ServerContext* context, const ::tai::GetAttributeRequest* request, ::tai::GetAttributeResponse* response);
        ::grpc::Status SetAttribute(::grpc::ServerContext* context, const ::tai::SetAttributeRequest* request, ::tai::SetAttributeResponse* response);
        ::grpc::Status Monitor(::grpc::ServerContext* context, const ::tai::MonitorRequest* request, ::grpc::ServerWriter< ::tai::MonitorResponse>* writer);
        ::grpc::Status SetLogLevel(::grpc::ServerContext* context, const ::tai::SetLogLevelRequest* request, ::tai::SetLogLevelResponse* response);
        ::grpc::Status Create(::grpc::ServerContext* context, const ::tai::CreateRequest* request, ::tai::CreateResponse* response);
        ::grpc::Status Remove(::grpc::ServerContext* context, const ::tai::RemoveRequest* request, ::tai::RemoveResponse* response);
    private:
        std::shared_ptr<TAINotifier> get_notifier(tai_object_id_t oid) {
            if ( m_notifiers.find(oid) == m_notifiers.end() ) {
                m_notifiers[oid] = std::make_shared<TAINotifier>();
            }
            return m_notifiers[oid];
        }
        const tai_api_method_table_t* const m_api;
        std::map<tai_object_id_t, std::shared_ptr<TAINotifier>> m_notifiers;
        std::mutex m_mtx; // mutex for m_notifiers
};

#endif // __TAIGRPC_HPP__
