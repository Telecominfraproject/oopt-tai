/**
 * @file    main.cpp
 *
 * @brief   This module implements taish server
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
#include <thread>
#include <queue>
#include <mutex>
#include <iostream>
#include <sys/eventfd.h>
#include "unistd.h"
#include <cstdlib>
#include <sstream>

using grpc::ServerBuilder;
using grpc::ServerContext;

static const std::string TAI_RPC_DEFAULT_IP = "0.0.0.0";
static const uint16_t TAI_RPC_DEFAULT_PORT = 50051;

tai_api_method_table_t g_api;

int event_fd;
std::queue<std::pair<bool, std::string>> q;
std::mutex m;

class module;

std::map<tai_object_id_t, module*> g_modules;

class module {
    public:
        module(tai_object_id_t id, std::string location) : m_id(id), m_location(location) {
            std::vector<tai_attribute_t> list;
            tai_attribute_t attr;
            attr.id = TAI_MODULE_ATTR_NUM_HOST_INTERFACES;
            list.push_back(attr);
            attr.id = TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES;
            list.push_back(attr);
            auto status = g_api.module_api->get_module_attributes(id, list.size(), list.data());
            if ( status != TAI_STATUS_SUCCESS ) {
                throw std::runtime_error("faile to get attribute");
            }
            std::cout << "num hostif: " << list[0].value.u32 << std::endl;
            std::cout << "num netif: " << list[1].value.u32 << std::endl;
            create_hostif(list[0].value.u32);
            create_netif(list[1].value.u32);
        }

        const std::string& location() {
            return m_location;
        }

        std::vector<tai_object_id_t> netifs;
        std::vector<tai_object_id_t> hostifs;
    private:
        tai_object_id_t m_id;
        std::string m_location;
        int create_hostif(uint32_t num);
        int create_netif(uint32_t num);
};



void module_presence(bool present, char* location) {
    uint64_t v = 1;
    std::lock_guard<std::mutex> g(m);
    q.push(std::pair<bool, std::string>(present, std::string(location)));
    auto ret = write(event_fd, &v, sizeof(uint64_t));
}

tai_status_t create_module(const std::string& location, tai_object_id_t& m_id) {
    std::vector<tai_attribute_t> list;
    tai_attribute_t attr;
    attr.id = TAI_MODULE_ATTR_LOCATION;
    attr.value.charlist.count = location.size();
    attr.value.charlist.list = (char*)location.c_str();
    list.push_back(attr);
    return g_api.module_api->create_module(&m_id, list.size(), list.data());
}

int module::create_hostif(uint32_t num) {
    for ( int i = 0; i < num; i++ ) {
        tai_object_id_t id;
        std::vector<tai_attribute_t> list;
        tai_attribute_t attr;
        attr.id = TAI_HOST_INTERFACE_ATTR_INDEX;
        attr.value.u32 = i;
        list.push_back(attr);
        auto status = g_api.hostif_api->create_host_interface(&id, m_id, list.size(), list.data());
        if ( status != TAI_STATUS_SUCCESS ) {
            throw std::runtime_error("failed to create host interface");
        }
        std::cout << "hostif: " << id << std::endl;
        hostifs.push_back(id);
    }
    return 0;
}

int module::create_netif(uint32_t num) {
    for ( int i = 0; i < num; i++ ) {
        tai_object_id_t id;
        std::vector<tai_attribute_t> list;
        tai_attribute_t attr;

        attr.id = TAI_NETWORK_INTERFACE_ATTR_INDEX;
        attr.value.u32 = i;
        list.push_back(attr);

        auto status = g_api.netif_api->create_network_interface(&id, m_id, list.size(), list.data());
        if ( status != TAI_STATUS_SUCCESS ) {
            throw std::runtime_error("failed to create network interface");
        }
        std::cout << "netif: " << id << std::endl;
        netifs.push_back(id);
    }
    return 0;
}

void grpc_thread(std::string addr) {
    TAIServiceImpl service(&g_api);

    ServerBuilder builder;
    builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    auto server = builder.BuildAndStart();
    std::cout << "Server listening on " << addr << std::endl;
    server->Wait();
}

int start_grpc_server(std::string addr) {
    std::thread th(grpc_thread, addr);
    th.detach();
}

tai_status_t list_module(tai_api_module_list_t* const l) {
    std::lock_guard<std::mutex> g(m);
    if ( l->count < g_modules.size() ) {
        l->count = g_modules.size();
        return TAI_STATUS_BUFFER_OVERFLOW;
    }
    l->count = g_modules.size();
    auto list = l->list;
    int i = 0;
    for ( auto v : g_modules ) {
        list[i].id = v.first;
        auto m = v.second;
        if ( list[i].hostifs.count < m->hostifs.size() ) {
            list[i].hostifs.count = m->hostifs.size();
            return TAI_STATUS_BUFFER_OVERFLOW;
        }
        list[i].hostifs.count = m->hostifs.size();
        for ( auto j = 0; j < m->hostifs.size(); j++ ) {
            list[i].hostifs.list[j] = m->hostifs[j];
        }

        if ( list[i].netifs.count < m->netifs.size() ) {
            list[i].netifs.count = m->netifs.size();
            return TAI_STATUS_BUFFER_OVERFLOW;
        }
        list[i].netifs.count = m->netifs.size();
        for ( auto j = 0; j < m->netifs.size(); j++ ) {
            list[i].netifs.list[j] = m->netifs[j];
        }
        i++;
    }
    return TAI_STATUS_SUCCESS;
}

int main(int argc, char *argv[]) {

    auto ip = TAI_RPC_DEFAULT_IP;
    auto port = TAI_RPC_DEFAULT_PORT;
    char c;

    while ((c = getopt (argc, argv, "i:p:")) != -1) {
      switch (c) {
      case 'i':
        ip = std::string(optarg);
        break;

      case 'p':
        port = atoi(optarg);
        break;

      default:
        std::cerr << "Usage: taish -i <IP address> -p <Port number>" << std::endl;
        return 1;
      }
    }

    tai_service_method_table_t services;
    services.module_presence = module_presence;
    event_fd = eventfd(0, 0);

    auto status = tai_api_initialize(0, &services);
    if ( status != TAI_STATUS_SUCCESS ) {
        std::cout << "failed to initialize" << std::endl;
        return -1;
    }

    status = tai_api_query(TAI_API_MODULE, (void **)(&g_api.module_api));
    if ( status != TAI_STATUS_SUCCESS ) {
        std::cout << "failed to query MODULE API" << std::endl;
        return -1;
    }

    status = tai_api_query(TAI_API_NETWORKIF, (void **)(&g_api.netif_api));
    if ( status != TAI_STATUS_SUCCESS ) {
        std::cout << "failed to query NETWORKIF API" << std::endl;
        return -1;
    }

    status = tai_api_query(TAI_API_HOSTIF, (void **)(&g_api.hostif_api));
    if ( status != TAI_STATUS_SUCCESS ) {
        std::cout << "failed to query HOSTIF API" << std::endl;
        return -1;
    }

    g_api.list_module = list_module;

    std::stringstream ss;
    ss << ip << ":" << port;
    start_grpc_server(ss.str());

    while (true) {
        uint64_t v;
        read(event_fd, &v, sizeof(uint64_t));
        {
            std::lock_guard<std::mutex> g(m);
            while ( !q.empty() ) {
                auto p = q.front();
                std::cout << "present: " << p.first << ", loc: " << p.second << std::endl;
                if ( p.first ) {
                    tai_object_id_t m_id;
                    auto status = create_module(p.second, m_id);
                    if ( status != TAI_STATUS_SUCCESS ) {
                        std::cerr << "failed to create module: " << status << std::endl;
                        return 1;
                    }
                    std::cout << "module id: " << m_id << std::endl;
                    g_modules[m_id] = new module(m_id, p.second);
                }
                q.pop();
            }
        }
    }
}
