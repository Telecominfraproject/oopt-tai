/**
 * @file    main.cpp
 *
 * @brief   This module implements taish server
 *
 * @copyright Copyright (c) 2018 Nippon Telegraph and Telephone Corporation
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <thread>
#include <queue>
#include <mutex>
#include <iostream>
#include <sys/eventfd.h>
#include "unistd.h"
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <cstdarg>

#include "json.hpp"

#include "taigrpc.hpp"
#include "taimetadata.h"

using grpc::ServerBuilder;
using grpc::ServerContext;

using json = nlohmann::json;

static const std::string TAI_RPC_DEFAULT_IP = "0.0.0.0";
static const uint16_t TAI_RPC_DEFAULT_PORT = 50051;

tai_api_method_table_t g_api;

int event_fd;
std::queue<std::pair<bool, std::string>> q;
std::mutex m;

class module;

std::map<std::string, module*> g_modules;

static int load_config(const json& config, std::vector<tai_attribute_t>& list, tai_object_type_t t) {
    int32_t attr_id;
    tai_serialize_option_t option{true, true, true};
    tai_alloc_info_t alloc_info = { .list_size = 16 };
    tai_attribute_t attr;

    if ( !config.is_object() ) {
        return -1;
    }

    for ( auto& a: config["attrs"].items() ) {
        int ret;
        option.json = false;
        switch ( t ) {
        case TAI_OBJECT_TYPE_MODULE:
            ret = tai_deserialize_module_attr(a.key().c_str(), &attr_id, &option);
            break;
        case TAI_OBJECT_TYPE_HOSTIF:
            ret = tai_deserialize_host_interface_attr(a.key().c_str(), &attr_id, &option);
            break;
        case TAI_OBJECT_TYPE_NETWORKIF:
            ret = tai_deserialize_network_interface_attr(a.key().c_str(), &attr_id, &option);
            break;
        default:
            throw std::runtime_error("unsupported object type");
        }
        if ( ret <  0 ) {
            std::stringstream ss;
            ss << "failed to deserialize attribute name: " << a.key();
            throw std::runtime_error(ss.str());
        }
        attr.id = attr_id;
        auto meta = tai_metadata_get_attr_metadata(t, attr_id);
        if ( meta == nullptr ) {
            std::stringstream ss;
            ss << "failed to get metadata for " << a.key();
            throw std::runtime_error(ss.str());
        }

        if ( tai_metadata_alloc_attr_value(meta, &attr, &alloc_info) != TAI_STATUS_SUCCESS ) {
            throw std::runtime_error("failed allocation");
        }

        option.json = true;

        auto value = a.value().dump();
        if ( tai_deserialize_attribute_value(value.c_str(), meta, &attr.value, &option) < 0 ) {
            throw std::runtime_error("failed to deserialize attribute value");
        }
        list.push_back(attr);
    }
    return 0;
}

class module {
    public:
        module(std::string location, const json& config, bool auto_creation) : m_id(0), m_location(location) {
            std::vector<tai_attribute_t> list;
            tai_attribute_t attr;

            if (!auto_creation) {
                return;
            }

            attr.id = TAI_MODULE_ATTR_LOCATION;
            attr.value.charlist.count = location.size();
            attr.value.charlist.list = (char*)location.c_str();
            list.push_back(attr);

            load_config(config, list, TAI_OBJECT_TYPE_MODULE);

            auto status = g_api.module_api->create_module(&m_id, list.size(), list.data());
            if ( status != TAI_STATUS_SUCCESS ) {
                std::cout << "failed to create module whose location is " << location << ", err: " << status << std::endl;
                return;
            }

            std::cout << "created module id: 0x" << std::hex << m_id << std::endl;

            list.clear();

            attr.id = TAI_MODULE_ATTR_NUM_HOST_INTERFACES;
            list.push_back(attr);
            attr.id = TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES;
            list.push_back(attr);
            status = g_api.module_api->get_module_attributes(m_id, list.size(), list.data());
            if ( status != TAI_STATUS_SUCCESS ) {
                throw std::runtime_error("faile to get attribute");
            }
            std::cout << "num hostif: " << list[0].value.u32 << std::endl;
            std::cout << "num netif: " << list[1].value.u32 << std::endl;
            create_hostif(list[0].value.u32, config);
            create_netif(list[1].value.u32, config);
        }

        const std::string& location() {
            return m_location;
        }

        tai_object_id_t id() {
            return m_id;
        }

        void set_id(tai_object_id_t id) {
            m_id = id;
            return;
        }

        std::vector<tai_object_id_t> netifs;
        std::vector<tai_object_id_t> hostifs;

        bool present;
    private:
        tai_object_id_t m_id;
        std::string m_location;
        int create_hostif(uint32_t num, const json& config);
        int create_netif(uint32_t num, const json& config);
};

void module_presence(bool present, char* location) {
    uint64_t v = 1;
    std::lock_guard<std::mutex> g(m);
    q.push(std::pair<bool, std::string>(present, std::string(location)));
    write(event_fd, &v, sizeof(uint64_t));
}

int module::create_hostif(uint32_t num, const json& config) {
    auto c = config.find("hostif");
    for ( uint32_t i = 0; i < num; i++ ) {
        tai_object_id_t id;
        std::vector<tai_attribute_t> list;
        tai_attribute_t attr;
        attr.id = TAI_HOST_INTERFACE_ATTR_INDEX;
        attr.value.u32 = i;
        list.push_back(attr);

        if ( c != config.end() && c->is_object() ) {
            std::stringstream ss;
            ss << i;
            auto cc = c->find(ss.str());
            if ( cc != c->end() ) {
                load_config(*cc, list, TAI_OBJECT_TYPE_HOSTIF);
            }
        }

        auto status = g_api.hostif_api->create_host_interface(&id, m_id, list.size(), list.data());
        if ( status != TAI_STATUS_SUCCESS ) {
            throw std::runtime_error("failed to create host interface");
        }
        std::cout << "hostif: 0x" << std::hex << id << std::endl;
        hostifs.push_back(id);
    }
    return 0;
}

int module::create_netif(uint32_t num, const json& config) {
    auto c = config.find("netif");
    for ( uint32_t i = 0; i < num; i++ ) {
        tai_object_id_t id;
        std::vector<tai_attribute_t> list;
        tai_attribute_t attr;

        attr.id = TAI_NETWORK_INTERFACE_ATTR_INDEX;
        attr.value.u32 = i;
        list.push_back(attr);

        if ( c != config.end() && c->is_object() ) {
            std::stringstream ss;
            ss << i;
            auto cc = c->find(ss.str());
            if ( cc != c->end() ) {
                load_config(*cc, list, TAI_OBJECT_TYPE_NETWORKIF);
            }
        }

        auto status = g_api.netif_api->create_network_interface(&id, m_id, list.size(), list.data());
        if ( status != TAI_STATUS_SUCCESS ) {
            throw std::runtime_error("failed to create network interface");
        }
        std::cout << "netif: 0x" << std::hex << id << std::endl;
        netifs.push_back(id);
    }
    return 0;
}

void grpc_thread(std::string addr) {
    TAIServiceImpl service(&g_api);

    ServerBuilder builder;
    builder.AddListeningPort(grpc::string(addr), grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    auto server = builder.BuildAndStart();
    std::cout << "Server listening on " << addr << std::endl;
    server->Wait();
}

int start_grpc_server(std::string addr) {
    std::thread th(grpc_thread, addr);
    th.detach();
    return 0;
}

void object_update(tai_object_type_t type, tai_object_id_t oid, bool is_create) {
    if ( type == TAI_OBJECT_TYPE_MODULE ) {
        if ( is_create ) {
            tai_attribute_t attr;
            char l[32];
            attr.id = TAI_MODULE_ATTR_LOCATION;
            attr.value.charlist.list = l;
            attr.value.charlist.count = 32;
            if ( g_api.module_api->get_module_attribute(oid, &attr) != TAI_STATUS_SUCCESS ) {
                return;
            }
            std::string loc(l, attr.value.charlist.count);
            auto m = g_modules[loc];
            if ( m == nullptr ) {
                return;
            }
            m->set_id(oid);
        } else {
            for ( auto& m : g_modules ) {
                if ( m.second->id() == oid ) {
                    m.second->set_id(TAI_NULL_OBJECT_ID);
                    break;
                }
            }
        }
        return;
    }

    auto mid = tai_module_id_query(oid);

    std::vector<tai_object_id_t> *v;

    for ( auto& m : g_modules ) {
        if ( type == TAI_OBJECT_TYPE_HOSTIF ) {
            v = &m.second->hostifs;
        } else if ( type == TAI_OBJECT_TYPE_NETWORKIF ) {
            v = &m.second->netifs;
        }

        if ( is_create ) {
            if ( m.second->id() == mid ) {
                v->emplace_back(oid);
                return;
            }
        } else {
            auto it = std::find(v->begin(), v->end(), oid);
            if ( it != v->end() ) {
                v->erase(it);
                return;
            }
        }
    }
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
        list[i].location = v.first;
        auto m = v.second;
        list[i].present = m->present;
        list[i].id =  m->id();
        if ( list[i].hostifs.count < m->hostifs.size() ) {
            list[i].hostifs.count = m->hostifs.size();
            return TAI_STATUS_BUFFER_OVERFLOW;
        }
        list[i].hostifs.count = m->hostifs.size();
        for ( auto j = 0; j < static_cast<int>(m->hostifs.size()); j++ ) {
            list[i].hostifs.list[j] = m->hostifs[j];
        }

        if ( list[i].netifs.count < m->netifs.size() ) {
            list[i].netifs.count = m->netifs.size();
            return TAI_STATUS_BUFFER_OVERFLOW;
        }
        list[i].netifs.count = m->netifs.size();
        for ( auto j = 0; j < static_cast<int>(m->netifs.size()); j++ ) {
            list[i].netifs.list[j] = m->netifs[j];
        }
        i++;
    }
    return TAI_STATUS_SUCCESS;
}

static const std::string to_string(tai_log_level_t level) {
    switch (level) {
    case TAI_LOG_LEVEL_DEBUG:
        return "DEBUG";
    case TAI_LOG_LEVEL_INFO:
        return "INFO";
    case TAI_LOG_LEVEL_NOTICE:
        return "NOTICE";
    case TAI_LOG_LEVEL_WARN:
        return "WARN";
    case TAI_LOG_LEVEL_ERROR:
        return "ERROR";
    case TAI_LOG_LEVEL_CRITICAL:
        return "CRITICAL";
    default:
        return std::to_string(static_cast<int>(level));
    }
}

static void log_cb(tai_log_level_t lvl, const char *file, int line, const char *function, const char *format, ...) {
    std::cout << to_string(lvl) << " [" << function << "@" << std::dec << line << "] ";
    std::va_list va;
    va_start(va, format);
    std::vprintf(format, va);
    va_end(va);
    std::cout << std::endl;
}

int main(int argc, char *argv[]) {

    auto ip = TAI_RPC_DEFAULT_IP;
    auto port = TAI_RPC_DEFAULT_PORT;
    std::string config_file;
    char c;
    tai_log_level_t level = TAI_LOG_LEVEL_INFO;
    auto auto_creation = true;

    while ((c = getopt (argc, argv, "i:p:f:vn")) != -1) {
      switch (c) {
      case 'i':
        ip = std::string(optarg);
        break;

      case 'p':
        port = atoi(optarg);
        break;

      case 'f':
        config_file = std::string(optarg);
        break;

      case 'v':
        level = TAI_LOG_LEVEL_DEBUG;
        break;

      case 'n':
        auto_creation = false;
        break;

      default:
        std::cerr << "Usage: taish -i <IP address> -p <Port number> -f <Config file> -v -n" << std::endl;
        return 1;
      }
    }

    tai_service_method_table_t services = {0};
    services.module_presence = module_presence;
    event_fd = eventfd(0, 0);

    auto status = tai_api_initialize(0, &services);
    if ( status != TAI_STATUS_SUCCESS ) {
        std::cout << "failed to initialize" << std::endl;
        return -1;
    }

    status = tai_log_set(TAI_API_UNSPECIFIED, level, log_cb);
    if ( status != TAI_STATUS_SUCCESS ) {
        std::cout << "failed to set log level" << std::endl;
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
    g_api.object_update = object_update;

    std::stringstream ss;
    ss << ip << ":" << port;
    start_grpc_server(ss.str());

    json config;
    if ( config_file != "" ) {
        std::ifstream ifs(config_file);
        if ( !ifs ) {
            std::cout << "failed to open config file: " << config_file << std::endl;
            return -1;
        }

        std::istreambuf_iterator<char> it(ifs), last;
        std::string c(it, last);
        config = json::parse(c);
        if ( !config.is_object() ) {
            std::cout << "invalid configuration. config is not object" << std::endl;
            return -1;
        }
    }

    while (true) {
        uint64_t v;
        read(event_fd, &v, sizeof(uint64_t));
        {
            std::lock_guard<std::mutex> g(m);
            while ( !q.empty() ) {
                auto p = q.front();
                auto loc = p.second;
                std::cout << "present: " << p.first << ", loc: " << loc << std::endl;
                if ( g_modules.find(loc) == g_modules.end() ) {
                    auto m = new module(loc, config[loc], auto_creation);
                    g_modules[loc] = m;
                }
                g_modules[loc]->present = p.first;
                q.pop();
            }
        }
    }
}
