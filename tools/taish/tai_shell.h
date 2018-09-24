#ifndef __TAI_SHELL_H__
#define __TAI_SHELL_H__

#ifdef __cplusplus 
extern "C" {
#endif

/* TAI API */
typedef struct tai_sh_api_s {
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

  pthread_mutex_t *lock;
} tai_sh_api_t;


/* TAI Shell API */
int tai_shell_start (uint16_t port, char *ip_addr);

int tai_shell_cmd_load (char *library_fiLe_name, tai_sh_api_t *tai_api);

int tai_shell_cmd_init (void);

#ifdef __cplusplus 
}
#endif

#endif /*  __TAI_SHELL_H__ */
