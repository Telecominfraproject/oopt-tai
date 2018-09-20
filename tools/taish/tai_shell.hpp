/**
 *  @file       tai_shell.hpp
 *  @brief      The shell to execute the TAI APIs
 *  @author     Yuji Hata <yuji.hata@ipinfusion.com>
 *              Tetsuya Murakami <tetsuya.murakami@ipinfusion.com>
 *  
 *  @copywrite  Copyright (C) 2018 IP Infusion, Inc. All rights reserved.
 *
 *  @remark     This source code is licensed under the Apache license found
 *              in the LICENSE file in the root directory of this source tree.
 */

#ifndef __TAI_SHELL_HPP__
#define __TAI_SHELL_HPP__

typedef int (*tai_command_fn)(
        _In_ std::ostream *ostr,
        _In_ std::vector <std::string> *args);

class tai_api {
public:
  tai_api(void *lib_handle);
  tai_status_t (*initialize)(
        _In_ uint64_t flags,
        _In_ const tai_service_method_table_t *services);

  tai_status_t (*query)(
        _In_ tai_api_t tai_api_id,
        _Out_ void** api_method_table);

  tai_status_t (*uninitialize)(void);

  tai_status_t (*log_set)(
        _In_ tai_api_t tai_api_id,
        _In_ tai_log_level_t log_level);

  tai_object_type_t (*object_type_query)(
        _In_ tai_object_id_t tai_object_id);

  tai_object_id_t (*module_id_query)(
        _In_ tai_object_id_t tai_object_id);

  tai_status_t (*dbg_generate_dump)(
        _In_ const char *dump_file_name);

  tai_log_level_t log_level[TAI_API_MAX];
};

int tai_command_load (std::ostream *ostr, std::vector <std::string> *args);
int tai_command_init (std::ostream *ostr, std::vector <std::string> *args);
int tai_command_quit (std::ostream *ostr, std::vector <std::string> *args);
int tai_command_help (std::ostream *ostr, std::vector <std::string> *args);
int tai_command_logset (std::ostream *ostr, std::vector <std::string> *args);
int tai_command_set_netif_attr (std::ostream *ostr, std::vector <std::string> *args);

class tai_cli_shell {
public:
  int cmd_parse(std::istream *istr, std::ostream *ostr);
  static std::map<std::string, tai_command_fn> cmd2handler;
};

class tai_cli_server: public tai_cli_shell {
public:
  tai_cli_server(sockaddr_in addr);
  int start();
  int restart();
  int accept();
  void disconnect();
  int recv();
private:
  int m_listen_fd;
  int m_client_fd;
  sockaddr_in m_sv_addr;
  sockaddr_in m_cl_addr;
  __gnu_cxx::stdio_filebuf<char> *m_ifilebuf;
  __gnu_cxx::stdio_filebuf<char> *m_ofilebuf;
  std::istream *m_istr;
  std::ostream *m_ostr;
};

#endif /*  __TAI_SHELL_HPP__ */
