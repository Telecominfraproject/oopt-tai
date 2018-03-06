/**
 * Copyright (c) 2018 Nippon Telegraph and Telephone Corporation.
 *
 * This source code is licensed under the BSD 3-Clause license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @file    tai.h
 *
 * @brief   This module defines an entry point into Transport Abstraction Interface (TAI)
 */

#ifndef __TAI_H__
#define __TAI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <saitypes.h>
#include "taiopticalchannel.h"
#include "taiopticalmodule.h"
#include "taidevice.h"

typedef enum _tai_api_t
{
    TAI_API_UNSPECIFIED    = 0,    /**< unspecified API */
    TAI_API_DEVICE = 1,           /**< tai_device_api_t */
    TAI_API_OPTICAL_MODULE = 2,   /**< tai_optical_module_api_t */
    TAI_API_OPTICAL_CHANNEL = 3,  /**< tai_optical_channel_api_t */
} tai_api_t;

typedef enum _tai_object_type_t
{
    TAI_OBJECT_TYPE_UNKNOWN,
    TAI_OBJECT_TYPE_DEVICE,
    TAI_OBJECT_TYPE_OPTICAL_MODULE,
    TAI_OBJECT_TYPE_OPTICAL_CHANNEL,
} tai_object_type_t;

/**
 * @brief Defines log level
 */
typedef enum _tai_log_level_t
{
    /** Log Level Debug */
    TAI_LOG_LEVEL_DEBUG            = 0,

    /** Log Level Info */
    TAI_LOG_LEVEL_INFO             = 1,

    /** Log Level Notice */
    TAI_LOG_LEVEL_NOTICE           = 2,

    /** Log level Warning */
    TAI_LOG_LEVEL_WARN             = 3,

    /** Log Level Error */
    TAI_LOG_LEVEL_ERROR            = 4,

    /** Log Level Critical */
    TAI_LOG_LEVEL_CRITICAL         = 5

} tai_log_level_t;

typedef const char* (*tai_profile_get_value_fn)(
        _In_ sai_switch_profile_id_t profile_id,
        _In_ const char *variable);

typedef int (*tai_profile_get_next_value_fn)(
        _In_ sai_switch_profile_id_t profile_id,
        _Out_ const char** variable,
        _Out_ const char** value);

/**
 * @brief Method table that contains function pointers for services exposed by
 * adapter host for adapter.
 */
typedef struct _service_method_table_t
{
    /**
     * @brief Get variable value given its name
     */
    tai_profile_get_value_fn        profile_get_value;

    /**
     * @brief Enumerate all the K/V pairs in a profile.
     *
     * Pointer to NULL passed as variable restarts enumeration. Function
     * returns 0 if next value exists, -1 at the end of the list.
     */
    tai_profile_get_next_value_fn   profile_get_next_value;

} service_method_table_t;

/**
 * @brief Adapter module initialization call. This is NOT for SDK initialization.
 *
 * @param[in] flags Reserved for future use, must be zero
 * @param[in] services Methods table with services provided by adapter host
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t tai_api_initialize(
        _In_ uint64_t flags,
        _In_ const service_method_table_t* services);

/**
 * @brief Retrieve a pointer to the C-style method table for desired TAI
 * functionality as specified by the given sai_api_id.
 *
 * @param[in] tai_api_id TAI API ID
 * @param[out] api_method_table Caller allocated method table The table must
 * remain valid until the tai_api_uninitialize() is called
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t tai_api_query(
        _In_ tai_api_t tai_api_id,
        _Out_ void** api_method_table);

/**
 * @brief Uninitialize adapter module. TAI functionalities,
 * retrieved via tai_api_query() cannot be used after this call.
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t tai_api_uninitialize(void);

/**
 * @brief Set log level for TAI API module. The default log level is #SAI_LOG_LEVEL_WARN
 *
 * @param[in] tai_api_id TAI API ID
 * @param[in] log_level Log level
 *
 * @return #SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t tai_log_set(
        _In_ tai_api_t tai_api_id,
        _In_ tai_log_level_t log_level);

#ifdef __cplusplus
}
#endif

#endif // __TAI_H__
