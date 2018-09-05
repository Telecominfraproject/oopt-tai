/**
 *  @file	tai_shell.cpp
 *  @brief	The shell to execute the TAI APIs
 *  @author	Yuji Hata <yuji.hata@ipinfusion.com>
 *  		Tetsuya Murakami <tetsuya.murakami@ipinfusion.com>
 *  
 *  @copywrite	Copyright (C) 2018 IP Infusion, Inc. All rights reserved.
 *
 *  @remark	This source code is licensed under the Apache license found
 *  		in the LICENSE file in the root directory of this source tree.
 */

#include <thread>
#include <chrono>
#include <iostream>
#include <map>
#include <vector>
#include <thread>

#include <mutex>
#include <queue>

#include <sys/eventfd.h>

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <dlfcn.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <ext/stdio_filebuf.h> 
#include <poll.h> 
#include <arpa/inet.h>

#include "tai.h"
#include "tai_shell.hpp"

tai_api *p_tai_api;

std::map<std::string, tai_command_fn> tai_cli_shell::cmd2handler = {
   {"?", tai_command_help},
   {"help", tai_command_help},
   {"load", tai_command_load},
   {"init", tai_command_init},
   {"quit", tai_command_quit},
   {"exit", tai_command_quit},
   {"logset", tai_command_logset}
};

std::vector <std::string> help_msgs = {
  {"?: show help messages for all commands\n"},
  {"help: show help messages for all commands\n"},
  {"load: load a TAI librarary: Usage: load <TAI librabry file name> \n"},
  {"init: Initialize TAI API.: Usage: init\n"},
  {"quit: Quit this telnet session.\n"},
  {"exit: Exit this telnet session.\n"},
  {"logset: Set log level.: Usage: logset <debug|info|notice|warn:error|critical> \n"},
};

tai_module_api_t *module_api;
tai_network_interface_api_t *netif_api;
tai_host_interface_api_t *hostif_api;

int fd;
std::queue<std::pair<bool, std::string>> q;
std::mutex m;

class module {
    public:
        module(tai_object_id_t id) : m_id(id) {
            std::vector<tai_attribute_t> list;
            tai_attribute_t attr;
            attr.id = TAI_MODULE_ATTR_NUM_HOST_INTERFACES;
            list.push_back(attr);
            attr.id = TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES;
            list.push_back(attr);
            auto status = module_api->get_module_attributes(id, list.size(), list.data());
            if ( status != TAI_STATUS_SUCCESS ) {
                throw std::runtime_error("faile to get attribute");
            }
            std::cout << "num hostif: " << list[0].value.u32 << std::endl;
            std::cout << "num netif: " << list[1].value.u32 << std::endl;
            create_hostif(list[0].value.u32);
            create_netif(list[1].value.u32);
        }
    private:
        tai_object_id_t m_id;
        std::vector<tai_object_id_t> netifs;
        std::vector<tai_object_id_t> hostifs;
        int create_hostif(uint32_t num);
        int create_netif(uint32_t num);
        int loop();
};

int module::create_netif(uint32_t num) {
    for ( int i = 0; i < num; i++ ) {
        tai_object_id_t id;
        std::vector<tai_attribute_t> list;
        tai_attribute_t attr;
        attr.id = TAI_NETWORK_INTERFACE_ATTR_INDEX;
        attr.value.u32 = i;
        list.push_back(attr);
        auto status = netif_api->create_network_interface(&id, m_id, list.size(), list.data());
        if ( status != TAI_STATUS_SUCCESS ) {
            throw std::runtime_error("failed to create network interface");
        }
        std::cout << "netif: " << id << std::endl;
        netifs.push_back(id);

        list.clear();

        attr.id = TAI_NETWORK_INTERFACE_ATTR_TX_ENABLE;
        attr.value.booldata = true;
        list.push_back(attr);

        attr.id = TAI_NETWORK_INTERFACE_ATTR_TX_CHANNEL;
        attr.value.u16 = 10;
        list.push_back(attr);

        attr.id = TAI_NETWORK_INTERFACE_ATTR_TX_GRID_SPACING;
        attr.value.u32 = TAI_NETWORK_INTERFACE_TX_GRID_SPACING_100_GHZ;
        list.push_back(attr);

        attr.id = TAI_NETWORK_INTERFACE_ATTR_MODULATION_FORMAT;
        attr.value.u32 = TAI_NETWORK_INTERFACE_MODULATION_FORMAT_DP_16_QAM;
        list.push_back(attr);

        status = netif_api->set_network_interface_attributes(id, list.size(), list.data());
        if ( status != TAI_STATUS_SUCCESS ) {
            throw std::runtime_error("failed to set netif attribute");
        }
    }
    return 0;
}

int module::create_hostif(uint32_t num) {
    for ( int i = 0; i < num; i++ ) {
        tai_object_id_t id;
        std::vector<tai_attribute_t> list;
        tai_attribute_t attr;
        attr.id = TAI_HOST_INTERFACE_ATTR_INDEX;
        attr.value.u32 = i;
        list.push_back(attr);
        auto status = hostif_api->create_host_interface(&id, m_id, list.size(), list.data());
        if ( status != TAI_STATUS_SUCCESS ) {
            throw std::runtime_error("failed to create host interface");
        }
        std::cout << "hostif: " << id << std::endl;
        hostifs.push_back(id);
    }
    return 0;
}

int module::loop() {
    return 0;
}

std::map<tai_object_id_t, module*> modules;

void module_presence(bool present, char* location) {
    uint64_t v;
    std::lock_guard<std::mutex> g(m);
    q.push(std::pair<bool, std::string>(present, std::string(location)));
    write(fd, &v, sizeof(uint64_t));
}

tai_status_t create_module(const std::string& location, tai_object_id_t& m_id) {
    std::vector<tai_attribute_t> list;
    tai_attribute_t attr;
    attr.id = TAI_MODULE_ATTR_LOCATION;
    attr.value.charlist.count = location.size();
    attr.value.charlist.list = (char*)location.c_str();
    list.push_back(attr);
    return module_api->create_module(&m_id, list.size(), list.data(), nullptr);
}

int main(int argc, char *argv[]) {
    tai_cli_server *cli_server;
    struct pollfd fds[3];
    nfds_t nfds;
    std::string ip_str;
    uint16_t port;
    int num_of_fds = 0;
    int timeout = 10 * 1000;
    struct sockaddr_in addr;
    int c;

    fd = eventfd(0, 0);

    ip_str = std::string(TAI_CLI_DEFAULT_IP);
    port = TAI_CLI_DEFAULT_PORT;

    while ((c = getopt (argc, argv, "i:p:")) != -1) {
      switch (c) {
      case 'i':
        ip_str = std::string(optarg);
        break;

      case 'p':
        port = atoi(optarg);
        break;

      default:
        std::cerr << "Usage: taish -i <IP address> -p <Port number>" << std::endl;
        return 1;
      }
    }

    memset (&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip_str.c_str(), &(addr.sin_addr));

    cli_server = new tai_cli_server(addr);
    if (cli_server == nullptr) {
      return -1;
    }

    fds[0].fd = fd;
    fds[0].events = POLLIN;
    fds[1].fd = cli_server->start();
    fds[1].events = POLLIN;
   
    nfds = 2;

    while (true) {
       int rc = poll(fds, nfds, -1);
       if (rc < 0) {
           if ((errno == EAGAIN) ||
               (errno == EINTR))
               continue;
           return -1;
        }
        if (fds[0].revents == POLLIN) {
            uint64_t v;
            read(fds[0].fd, &v, sizeof(uint64_t));
            {
                std::lock_guard<std::mutex> g(m);
                while ( ! q.empty() ) {
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
                        modules[m_id] = new module(m_id);
                    }
                    q.pop();
                }
            }
        }

        if (fds[1].revents == POLLIN) {
            int tmp_fd = cli_server->accept_();
            if (tmp_fd > 0) {
                fds[2].fd = tmp_fd;
                nfds = 3;
            } else if (tmp_fd != 0) {
                if (nfds == 3) {
                    cli_server->disconnect();
                    nfds = 2;
                }

                tmp_fd = cli_server->restart();
                if (tmp_fd > 0) {
                    fds[1].fd = tmp_fd;
                } else {
                    std::cerr << "restarting cli server failed " << std::endl;
                    return -1;
                }
            }
        } else if (fds[1].revents != 0) {
            if (nfds == 3) {
                cli_server->disconnect();
                nfds = 2;
            }

            int tmp_fd = cli_server->restart();
            if (tmp_fd > 0) {
                fds[1].fd = tmp_fd;
            } else {
                std::cerr << "restarting cli server failed " << std::endl;
                return -1;
            }
        }

        if ((fds[2].revents == POLLIN) && (nfds == 3)) {
            rc = cli_server->recv();
            if (rc == -10) {
                cli_server->disconnect();
                nfds = 2;
            }
        } else if (fds[2].revents != 0 && (nfds == 3)) {
            cli_server->disconnect();
            nfds = 2;
        }

        for (int i = 0; i < nfds; i++) {
            fds[i].events = POLLIN;
        }
    }
  return 0;
}

tai_cli_server::tai_cli_server(struct sockaddr_in addr) {
  m_sv_addr = addr;
}

int tai_cli_server::start() {
  int    len, rc, on = 1;
  m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (m_listen_fd < 0)
  {
    return -1;
  }

  rc = setsockopt(m_listen_fd, SOL_SOCKET,  SO_REUSEADDR,
                  (char *)&on, sizeof(on));
  if (rc < 0)
  {
    close(m_listen_fd);
    m_listen_fd = -1;
    return -1;
  }

  rc = bind(m_listen_fd,
            (struct sockaddr *)&m_sv_addr, sizeof(m_sv_addr));
  if (rc < 0)
  {
    close(m_listen_fd);
    m_listen_fd = -1;
    return -1;
  }

  rc = listen(m_listen_fd, 0);
  if (rc < 0)
  {
    close(m_listen_fd);
    m_listen_fd = -1;
    return -1;
  }

  return m_listen_fd;
}

int tai_cli_server::restart() {
  close(m_listen_fd);
  m_listen_fd = -1;
  return start();
}

int tai_cli_server::accept_() {
  int val;
  int ret;
  socklen_t length;

  if (m_client_fd > 0) {
    return m_client_fd;
  }

  length = sizeof(m_cl_addr);
  m_client_fd = accept(m_listen_fd, (sockaddr *)&m_cl_addr, &length);
  if (m_client_fd < 0) {
    if ((errno == EWOULDBLOCK) ||
        (errno == EINTR)) {
      return 0;
    }
    return -1;
  }

  m_ifilebuf = new __gnu_cxx::stdio_filebuf<char>(m_client_fd, std::ios::in);
  m_ofilebuf = new __gnu_cxx::stdio_filebuf<char>(m_client_fd, std::ios::out);
  m_istr = new std::istream(m_ifilebuf);
  m_ostr = new std::ostream(m_ofilebuf);
  return m_client_fd;
}

void tai_cli_server::disconnect() {
  delete m_istr;
  delete m_ostr;
  delete m_ifilebuf;
  delete m_ofilebuf;
  close (m_client_fd);
  m_client_fd = -1;
}

int tai_cli_server::recv() {
  return cmd_parse (m_istr, m_ostr);
}

int tai_cli_shell::cmd_parse(std::istream *istr, std::ostream *ostr) {
  int ret = 0;
  std::string buf;
  std::vector <std::string> *args;
  std::map<std::string, tai_command_fn>::iterator cmd;

  std::getline (*istr, buf);
  if (istr->eof() || istr->fail())
    return -10;

  if (buf.back() == '\r') { // remove CR
    buf.resize(buf.size()-1);
  }

  args = make_args (buf);
  if (args != nullptr && args->size() != 0) {
    cmd = cmd2handler.find((*args)[0]);
    if (cmd != cmd2handler.end()) {
      ret = cmd->second(ostr, args);
    } else {
      *ostr << "unknown command(" << (*args)[0] << ") was speified!!\n";
    }
  }

  if (args != nullptr) {
    delete args;
  }

  *ostr << "> ";
  ostr->flush();
  return ret;
}

std::vector <std::string> *tai_cli_shell::make_args (std::string cmd) {
  int length;
  char *arg;
  std::vector <std::string> *args;
  char *saveptr = nullptr;

  args = new std::vector <std::string>;
  if (args == nullptr) {
    return nullptr;
  }
  arg = strtok_r ((char *)cmd.c_str(), " ", &saveptr);
  while (arg != nullptr) {
    args->push_back (std::string(arg, strlen(arg)));
    arg = strtok_r (NULL, " ", &saveptr);
  }

  return args;
}

tai_api::tai_api (void *lib_handle) {
  initialize        = (tai_status_t (*)( _In_ uint64_t, _In_ const tai_service_method_table_t *))
                         dlsym(lib_handle, "tai_api_initialize");
  uninitialize      = (tai_status_t (*)(void))
                         dlsym(lib_handle, "tai_api_uninitialize");
  query             = (tai_status_t (*)( _In_ tai_api_t, _Out_ void**))
                         dlsym(lib_handle, "tai_api_query");
  log_set           = (tai_status_t (*)( _In_ tai_api_t, _In_ tai_log_level_t))
                         dlsym(lib_handle, "tai_log_set");
  object_type_query = (tai_object_type_t (*)( _In_ tai_object_id_t))
                         dlsym(lib_handle, "tai_object_type_query");
  module_id_query   = (tai_object_id_t (*)( _In_ tai_object_id_t))
                         dlsym(lib_handle, "tai_module_id_query");
  dbg_generate_dump = (tai_status_t (*)(_In_ const char *))
                         dlsym(lib_handle, "tai_dbg_generate_dump");
  log_level = TAI_LOG_LEVEL_INFO;
}

int tai_command_load (std::ostream *ostr, std::vector <std::string> *args) {
  void *lib_handle;

  if (args->size() != 2) {
    *ostr << "Invalid parameters were specified!!\n";
    return -1;
  }

  if (p_tai_api != nullptr) {
    *ostr << "TAI library is already loaded!!\n";
    return -1;
  }

  lib_handle = dlopen ((*args)[1].c_str(), RTLD_NOW | RTLD_GLOBAL);

  if (lib_handle == nullptr) {
    *ostr << "Loading " << (*args)[1] << " failed!!\n";
    *ostr << dlerror();
    return -1;
  }
  
  p_tai_api = new tai_api(lib_handle);

  return 0;
}

int tai_command_init (std::ostream *ostr, std::vector <std::string> *args) {
  tai_service_method_table_t services;

  if (args->size() != 1) {
    *ostr << "Invalid parameters were specified!!\n";
    return -1;
  }

  if (p_tai_api == nullptr) {
    *ostr << "TAI library has not been loaded yet!!\n";
    return -1;
  }

  services.module_presence = module_presence;

  if (p_tai_api->log_set) {
    p_tai_api->log_set(tai_api_t(0), p_tai_api->log_level);
  }

  if (p_tai_api->initialize) {
    auto status = p_tai_api->initialize(0, &services);
    if ( status != TAI_STATUS_SUCCESS ) {
      return -1;
    }
  }

  if (p_tai_api->query) {
    auto status = p_tai_api->query(TAI_API_MODULE, (void **)(&module_api));
    if ( status != TAI_STATUS_SUCCESS ) {
      return -1;
    }

    status = p_tai_api->query(TAI_API_NETWORKIF, (void **)(&netif_api));
    if ( status != TAI_STATUS_SUCCESS ) {
      return -1;
    }

    status = p_tai_api->query(TAI_API_HOSTIF, (void **)(&hostif_api));
    if ( status != TAI_STATUS_SUCCESS ) {
      return -1;
    }
  }

  return 0;
}

int tai_command_help (std::ostream *ostr, std::vector <std::string> *args) {
  for (std::vector <std::string>::iterator it = help_msgs.begin();
       it != help_msgs.end(); it++) {
    *ostr << *it;
  }

  return 0;
}

int tai_command_quit (std::ostream *ostr, std::vector <std::string> *args) {
  return -10;
}

int tai_command_logset (std::ostream *ostr, std::vector <std::string> *args) {
  if (args->size() != 2) {
    *ostr << "Invalid parameters were specified!!\n";
    return -1;
  }

  if (p_tai_api == nullptr) {
    *ostr << "TAI library has not been loaded yet!!\n";
    return -1;
  }

  if ((*args)[1].compare("debug") == 0) {
    p_tai_api->log_level = TAI_LOG_LEVEL_DEBUG;

  } else if ((*args)[1].compare("info") == 0) {
    p_tai_api->log_level = TAI_LOG_LEVEL_INFO;

  } else if ((*args)[1].compare("notice") == 0) {
    p_tai_api->log_level = TAI_LOG_LEVEL_NOTICE;

  } else if ((*args)[1].compare("warn") == 0) {
    p_tai_api->log_level = TAI_LOG_LEVEL_WARN;

  } else if ((*args)[1].compare("error") == 0) {
    p_tai_api->log_level = TAI_LOG_LEVEL_ERROR;

  } else if ((*args)[1].compare("critical") == 0) {
    p_tai_api->log_level = TAI_LOG_LEVEL_CRITICAL;

  } else {
    *ostr << "Invalid log-level(" << (*args)[1] << ") was specified!!\n";
    return -1;
  }

  if (p_tai_api->log_set) {
    p_tai_api->log_set(tai_api_t(0), p_tai_api->log_level);
  }

  return 0;
}
