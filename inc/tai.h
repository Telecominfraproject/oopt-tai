/**
 * @file    tai.h
 * @brief   This module defines an entry point into Transponder Abstraction
 *          Interface (TAI)
 *
 * @copyright Copyright (c) 2014 Microsoft Open Technologies, Inc.
 * @copyright Copyright (c) 2018 Nippon Telegraph and Telephone Corporation
 * @copyright Copyright (c) 2017 Cumulus Networks, Inc.
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
 * @remark  Microsoft would like to thank the following companies for their
 *          review and assistance with these files: Intel Corporation, Mellanox
 *          Technologies Ltd, Dell Products, L.P., Facebook, Inc., Marvell
 *          International Ltd.
 */

#if !defined (__TAI_H_)
#define __TAI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "taihostif.h"
#include "taimodule.h"
#include "tainetworkif.h"
#include "taistatus.h"
#include "taitypes.h"

/**
 * @defgroup TAI TAI - Entry point specific API definitions.
 *
 * @{
 */

/**
 * @brief Defined API sets have assigned IDs.
 *
 * If specific API method table changes in any way (method signature, number of
 * methods), a new ID needs to be created (e.g. HOSTIF2) and old API still may
 * need to be supported for compatibility with older adapter hosts.
 */
typedef enum _tai_api_t
{
    TAI_API_UNSPECIFIED,        /**< unspecified API */
    TAI_API_MODULE,             /**< #tai_module_api_t */
    TAI_API_HOSTIF,             /**< #tai_host_interface_api_t */
    TAI_API_NETWORKIF,          /**< #tai_network_interface_api_t */
    TAI_API_MAX                 /**< total number of APIs */
} tai_api_t;

/**
 * @brief Defines the logging level.
 */
typedef enum _tai_log_level_t
{
    TAI_LOG_LEVEL_DEBUG,        /**< Debug logging level */
    TAI_LOG_LEVEL_INFO,         /**< Info logging level */
    TAI_LOG_LEVEL_NOTICE,       /**< Notice logging level */
    TAI_LOG_LEVEL_WARN,         /**< Warning logging level */
    TAI_LOG_LEVEL_ERROR,        /**< Error logging level */
    TAI_LOG_LEVEL_CRITICAL,     /**< Critical logging level */
    TAI_LOG_LEVEL_MAX           /**< Number of logging levels */
} tai_log_level_t;

/**
 * @brief The adapter calls this function, whose address is provided by the
 *        adapter host, whenever there is a change in an optical module's
 *        presence. This function will be called once for each module present
 *        toward the end of the tai_api_initialze function, and then whenever
 *        there is a change. The adapter host should be ready to accept calls to
 *        this function at any time after invoking the tai_api_initialize
 *        function. Because this function may be called in different contexts
 *        (such as interrupt context or from a different thread/process) the
 *        adapter host must not invoke other TAI interfaces directly from this
 *        function.
 */
typedef void (*tai_module_presence_event_fn)(
        _In_ bool present,
        _In_ char * module_location);

/**
 * @brief Method table that contains function pointers for services exposed by
 * the adapter host for the adapter. This is currently a single service: module
 * presence, which is called whenever a module is inserted or removed.
 */
typedef struct _tai_service_method_table_t
{
    /**
     * @brief Notification of module insertion/removal
     */
    tai_module_presence_event_fn    module_presence;

} tai_service_method_table_t;

/**
 * @brief Adapter module initialization call
 *
 * This is NOT for SDK initialization. This function allows the adapter to
 * initialize any data/control structures that may be necessary during
 * subsequent TAI operations.
 *
 * @param[in] flags Reserved for future use, must be zero
 * @param[in] services Methods table with services provided by adapter host
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_api_initialize(
        _In_ uint64_t flags,
        _In_ const tai_service_method_table_t *services);

/**
 * @brief Retrieve a pointer to the C-style method table for desired TAI
 * functionality as specified by the given tai_api_id.
 *
 * @param[in] tai_api_id The API ID whose method table is being retrieved.
 * @param[out] api_method_table Caller allocated method table. The table must
 * remain valid until the tai_api_uninitialize() is called.
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_api_query(
        _In_ tai_api_t tai_api_id,
        _Out_ void** api_method_table);

/**
 * @brief Uninitialize adapter module. TAI functionalities,
 * retrieved via tai_api_query() cannot be used after this call.
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_api_uninitialize(void);

/**
 * @brief Set log level for TAI API module
 *
 * The default log level is #TAI_LOG_LEVEL_WARN.
 *
 * @param[in] tai_api_id The API ID whose logging level is being set
 * @param[in] log_level Log level
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_log_set(
        _In_ tai_api_t tai_api_id,
        _In_ tai_log_level_t log_level);

/**
 * @brief Query TAI object type.
 *
 * @param[in] tai_object_id Object id
 *
 * @return #TAI_OBJECT_TYPE_NULL when tai_object_id is not valid.
 * Otherwise, return a valid TAI object type TAI_OBJECT_TYPE_XXX.
 */
tai_object_type_t tai_object_type_query(
        _In_ tai_object_id_t tai_object_id);

/**
 * @brief Query TAI module id.
 *
 * @param[in] tai_object_id Object id
 *
 * @return #TAI_NULL_OBJECT_ID when tai_object_id is not valid.
 * Otherwise, return a valid TAI_OBJECT_TYPE_MODULE object on which provided
 * object id belongs. If valid module id object is provided as input parameter
 * it should return itself.
 */
tai_object_id_t tai_module_id_query(
        _In_ tai_object_id_t tai_object_id);

/**
 * @brief Generate dump file. The dump file may include TAI state information
 *        and vendor SDK information.
 *
 * @param[in] dump_file_name Full path for dump file
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_dbg_generate_dump(
        _In_ const char *dump_file_name);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */
#endif /** __TAI_H_ */
