/**
 *  @file    stub_tai.c
 *  @brief   The main TAI interface routines
 *  @author  Scott Emery <scotte@cumulusnetworks.com>
 *
 *  @copyright Copyright (C) 2018 Cumulus Networks, Inc. All rights reserved
 *
 *  @copyright Copyright (C) 2018 Cumulus Networks, Inc. All rights reserved
 *
 *  @remark  Licensed under the Apache License, Version 2.0 (the "License"); you 
 *           may not use this file except in compliance with the License. You may
 *           obtain a copy of the License at
 *           http://www.apache.org/licenses/LICENSE-2.0
 *
 *  @remark  THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR 
 *           CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *           LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 *           FOR A PARTICULAR PURPOSE, MERCHANTABILITY OR NON-INFRINGEMENT.
 *
 *  @remark  See the Apache Version 2.0 License for specific language governing
 *           permissions and limitations under the License.
 */


#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include "tai.h"


static tai_service_method_table_t adapter_host_fns;
static bool                       initialized = false;


/*------------------------------------------------------------------------------

                              Utility Functions

------------------------------------------------------------------------------*/

#undef  __TAI_MODULE__
#define __TAI_MODULE__ TAI_API_UNSPECIFIED

/** @brief The Stub TAI adapter uses the following format for object ids */
typedef struct _stub_object_id_t {
    uint8_t type;
    uint8_t reserved;
    uint32_t value;
} stub_object_id_t;

/**
 * @brief Find an attribute in a list of attributes
 * 
 * @param [in] attr_id The attribute ID to find
 * @param [in] attr_count The number of attributes in the list
 * @param [in] attr_list A list of attributes
 * 
 * @return tai_attribute_value_t* A pointer to the attribute's value, or NULL if
 *         not found.
 */
const tai_attribute_value_t * find_attribute_in_list(
   _In_ tai_attr_id_t              attr_id,
   _In_ uint32_t                   attr_count,
   _In_ const tai_attribute_t     *attr_list)
{
    while (attr_count--) {
        if (attr_list->id == attr_id) {
            return &attr_list->value;
        }
        attr_list++;
    }
    return NULL;
}

/**
 * @brief Convert a TAI status code to an indexed status code
 *
 * If the status code in 'err' is one of the codes for a list of attributes, the
 * index value is added to that code. Otherwise the 'err' code is returned.
 *
 * @param err A TAI_STATUS_* code
 * @param idx An index into a list of attributes
 * 
 * @return tai_status_t 
 */
tai_status_t convert_tai_error_to_list( _In_ tai_status_t err, _In_ uint32_t idx)
{
    if (TAI_STATUS_IS_INVALID_ATTRIBUTE(err)    || 
        TAI_STATUS_IS_INVALID_ATTR_VALUE(err)   ||
        TAI_STATUS_IS_ATTR_NOT_IMPLEMENTED(err) ||
        TAI_STATUS_IS_UNKNOWN_ATTRIBUTE(err)    ||
        TAI_STATUS_IS_ATTR_NOT_SUPPORTED(err)) {
        return err + idx;
    }
    return err;
}

#define TAI_SYSLOG(lvl, ...)     tai_syslog(__TAI_MODULE__, lvl, __VA_ARGS__)
#define TAI_SYSLOG_DEBUG(...)    TAI_SYSLOG(TAI_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define TAI_SYSLOG_INFO(...)     TAI_SYSLOG(TAI_LOG_LEVEL_INFO, __VA_ARGS__)
#define TAI_SYSLOG_NOTICE(...)   TAI_SYSLOG(TAI_LOG_LEVEL_NOTICE, __VA_ARGS__)
#define TAI_SYSLOG_WARN(...)     TAI_SYSLOG(TAI_LOG_LEVEL_WARN, __VA_ARGS__)
#define TAI_SYSLOG_ERROR(...)    TAI_SYSLOG(TAI_LOG_LEVEL_ERROR, __VA_ARGS__)
#define TAI_SYSLOG_CRITICAL(...) TAI_SYSLOG(TAI_LOG_LEVEL_CRITICAL, __VA_ARGS__)

/**
 * @brief Given a TAI logging level, convert to a syslog level.
 */
static int tai_to_syslog_level[TAI_LOG_LEVEL_MAX] = {
    [TAI_LOG_LEVEL_DEBUG]    = LOG_DEBUG,
    [TAI_LOG_LEVEL_INFO]     = LOG_INFO,
    [TAI_LOG_LEVEL_NOTICE]   = LOG_NOTICE,
    [TAI_LOG_LEVEL_WARN]     = LOG_WARNING,
    [TAI_LOG_LEVEL_ERROR]    = LOG_ERR,
    [TAI_LOG_LEVEL_CRITICAL] = LOG_CRIT
};

/**
 * @brief Given a TAI module, retrieve the syslog level for that module. This is 
 *        set by calling the #tai_log_set function. The default is WARNING.
 */
static int api_log_level[TAI_API_MAX] = {
    [0 ... TAI_API_MAX-1] = LOG_WARNING
};

/**
 *  @brief Log a message to the syslog facility. The message may be filtered 
 *         based on the TAI API's previously set logging level.
 * 
 *  @param [in] tai_api_id The TAI API logging this message
 *  @param [in] log_level The TAI message priority
 *  @param [in] format A printf-like format string
 */
void tai_syslog(_In_ tai_api_t tai_api_id, _In_ tai_log_level_t log_level, 
                _In_ const char *format, ...)
{
    va_list arglist;

    if ((TAI_API_UNSPECIFIED > tai_api_id) || (TAI_API_MAX <= tai_api_id)) {
        tai_api_id = TAI_API_UNSPECIFIED;
    }
    if ((TAI_LOG_LEVEL_DEBUG > log_level) || (TAI_LOG_LEVEL_MAX <= log_level)) {
        log_level = TAI_LOG_LEVEL_ERROR;
    }
    setlogmask(LOG_UPTO(api_log_level[tai_api_id]));
    va_start(arglist, format);
    vsyslog(tai_to_syslog_level[log_level], format, arglist);
    va_end(arglist);

    return;
}


/*------------------------------------------------------------------------------

                        Host Interface Object Functions

------------------------------------------------------------------------------*/

#undef  __TAI_MODULE__
#define __TAI_MODULE__ TAI_API_HOSTIF

/**
 * @brief Retrieve the value of an attribute
 *
 * @param [in] host_interface_id The host interface ID handle
 * @param [in,out] attr A pointer to the attribute to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_get_host_interface_attribute(
    _In_ tai_object_id_t     host_interface_id,
    _Inout_ tai_attribute_t *attr)
{
    TAI_SYSLOG_DEBUG("Retrieving host interface attribute: %d", attr->id);
    switch (attr->id) {
        case TAI_HOST_INTERFACE_ATTR_INDEX:
        case TAI_HOST_INTERFACE_ATTR_LANE_FAULT:
        case TAI_HOST_INTERFACE_ATTR_TX_ALIGN_STATUS:
        case TAI_HOST_INTERFACE_ATTR_FEC_TYPE:
        case TAI_HOST_INTERFACE_ATTR_LOOPBACK_TYPE:
            return TAI_STATUS_SUCCESS;
    }
    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
}

/**
 * @brief Retrieve a list of attribute values
 *
 * @param [in] host_interface_id The host interface ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in,out] attr_list A list of attributes to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_get_host_interface_attributes(
    _In_ tai_object_id_t     host_interface_id,
    _In_ uint32_t            attr_count,
    _Inout_ tai_attribute_t *attr_list)
{
    uint32_t idx;
    tai_status_t ret;

    for (idx = 0; idx < attr_count; idx++) {
        ret = stub_get_host_interface_attribute(host_interface_id, attr_list++);
        if (ret) {
            return convert_tai_error_to_list(ret, idx);
        }
    }
    return TAI_STATUS_SUCCESS; 
}

/**
 * @brief Set the value of an attribute
 *
 * @param [in] host_interface_id The host interface ID handle
 * @param [in] attr A pointer to the attribute to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_set_host_interface_attribute(
   _In_ tai_object_id_t        host_interface_id,
   _In_ const tai_attribute_t *attr)
{
    TAI_SYSLOG_DEBUG("Setting host interface attribute: %d", attr->id);
    switch (attr->id) {
        case TAI_HOST_INTERFACE_ATTR_INDEX:
        case TAI_HOST_INTERFACE_ATTR_FEC_TYPE:
        case TAI_HOST_INTERFACE_ATTR_LOOPBACK_TYPE:
            return TAI_STATUS_SUCCESS;
        case TAI_HOST_INTERFACE_ATTR_LANE_FAULT:
        case TAI_HOST_INTERFACE_ATTR_TX_ALIGN_STATUS:
            return TAI_STATUS_INVALID_ATTRIBUTE_0;
    }
    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
}

/**
 * @brief Set the values from a list of attributes
 *
 * @param [in] host_interface_id The host interface ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_set_host_interface_attributes(
   _In_ tai_object_id_t        host_interface_id,
   _In_ uint32_t               attr_count,
   _In_ const tai_attribute_t *attr_list)
{
    uint32_t idx;
    tai_status_t ret;

    for (idx = 0; idx < attr_count; idx++) {
        ret = stub_set_host_interface_attribute(host_interface_id, attr_list++);
        if (ret) {
            return convert_tai_error_to_list(ret, idx);
        }
    }
    return TAI_STATUS_SUCCESS; 
}

/**
 * @brief Host interface initialization. After the call the capability 
 *        attributes should be ready for retrieval via
 *        tai_get_host_interface_attribute().
 *
 * @param [out] host_interface_id Handle which identifies the host interface
 * @param [in] module_id Handle which identifies the module on which the host 
 *        interface exists
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to set during initialization
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_create_host_interface(
    _Out_ tai_object_id_t *host_interface_id,
    _In_ tai_object_id_t module_id,
    _In_ uint32_t attr_count,
    _In_ const tai_attribute_t *attr_list)
{
    tai_status_t ret;
    const tai_attribute_value_t * hostif_addr;

    hostif_addr = find_attribute_in_list(TAI_HOST_INTERFACE_ATTR_INDEX, attr_count, attr_list);
    if (NULL == hostif_addr) {
        TAI_SYSLOG_ERROR("The required TAI_HOST_INTERFACE_ATTR_INDEX attribute was not provided");
        return TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    ret = stub_set_host_interface_attributes(*host_interface_id, attr_count, attr_list);
    if (TAI_STATUS_SUCCESS != ret) {
        TAI_SYSLOG_ERROR("Error setting host interface attributes");
        return ret;
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Release all resources associated with previously created host 
 *        interface
 *
 * @param [in] host_interface_id The host interface ID handle being removed
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_remove_host_interface(_In_ tai_object_id_t host_interface_id)
{
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief The host interface functions. This structure is retrieved via the 
 *        #tai_api_query function.
 */
tai_host_interface_api_t stub_host_interface_api = {
    .create_host_interface         = stub_create_host_interface,
    .remove_host_interface         = stub_remove_host_interface,
    .set_host_interface_attribute  = stub_set_host_interface_attribute,
    .set_host_interface_attributes = stub_set_host_interface_attributes,
    .get_host_interface_attribute  = stub_get_host_interface_attribute,
    .get_host_interface_attributes = stub_get_host_interface_attributes
};


/*------------------------------------------------------------------------------

                      Network Interface Object Functions

------------------------------------------------------------------------------*/

#undef  __TAI_MODULE__
#define __TAI_MODULE__ TAI_API_NETWORKIF

/**
 * @brief Retrieve the value of an attribute
 *
 * @param [in] network_interface_id The network interface ID handle
 * @param [in,out] attr A pointer to the attribute to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_get_network_interface_attribute(
    _In_ tai_object_id_t     network_interface_id,
    _Inout_ tai_attribute_t *attr)
{
    TAI_SYSLOG_DEBUG("Retrieving network interface attribute: %d", attr->id);
    switch (attr->id) {
        case TAI_NETWORK_INTERFACE_ATTR_INDEX:
        case TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS:
        case TAI_NETWORK_INTERFACE_ATTR_RX_ALIGN_STATUS:
        case TAI_NETWORK_INTERFACE_ATTR_TX_ENABLE:
        case TAI_NETWORK_INTERFACE_ATTR_TX_GRID_SPACING:
        case TAI_NETWORK_INTERFACE_ATTR_TX_CHANNEL:
        case TAI_NETWORK_INTERFACE_ATTR_OUTPUT_POWER:
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_OUTPUT_POWER:
        case TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ:
        case TAI_NETWORK_INTERFACE_ATTR_TX_FINE_TUNE_LASER_FREQ:
        case TAI_NETWORK_INTERFACE_ATTR_MODULATION_FORMAT:
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_BER:
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_BER_PERIOD:
        case TAI_NETWORK_INTERFACE_ATTR_DIFFERENTIAL_ENCODING:
        case TAI_NETWORK_INTERFACE_ATTR_OPER_STATUS:
        case TAI_NETWORK_INTERFACE_ATTR_MIN_LASER_FREQ:
        case TAI_NETWORK_INTERFACE_ATTR_MAX_LASER_FREQ:
        case TAI_NETWORK_INTERFACE_ATTR_LASER_GRID_SUPPORT:
        case TAI_NETWORK_INTERFACE_ATTR_LOOPBACK_TYPE:
            return TAI_STATUS_SUCCESS;
    }
    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
}

/**
 * @brief Retrieve a list of attribute values
 *
 * @param [in] network_interface_id The network interface ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in,out] attr_list A list of attributes to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_get_network_interface_attributes(
    _In_ tai_object_id_t     network_interface_id,
    _In_ uint32_t            attr_count,
    _Inout_ tai_attribute_t *attr_list)
{
    uint32_t idx;
    tai_status_t ret;

    for (idx = 0; idx < attr_count; idx++) {
        ret = stub_get_network_interface_attribute(network_interface_id, attr_list++);
        if (ret) {
            return convert_tai_error_to_list(ret, idx);
        }
    }
    return TAI_STATUS_SUCCESS; 
}

/**
 * @brief Set the value of an attribute
 *
 * @param [in] network_interface_id The network interface ID handle
 * @param [in] attr A pointer to the attribute to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_set_network_interface_attribute(
   _In_ tai_object_id_t        network_interface_id,
   _In_ const tai_attribute_t *attr)
{
    TAI_SYSLOG_DEBUG("Setting network interface attribute: %d", attr->id);
    switch (attr->id) {
        case TAI_NETWORK_INTERFACE_ATTR_INDEX:
            return TAI_STATUS_SUCCESS;
        case TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS:
        case TAI_NETWORK_INTERFACE_ATTR_RX_ALIGN_STATUS:
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_OUTPUT_POWER:
        case TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ:
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_BER:
        case TAI_NETWORK_INTERFACE_ATTR_CURRENT_BER_PERIOD:
        case TAI_NETWORK_INTERFACE_ATTR_OPER_STATUS:
        case TAI_NETWORK_INTERFACE_ATTR_MIN_LASER_FREQ:
        case TAI_NETWORK_INTERFACE_ATTR_MAX_LASER_FREQ:
        case TAI_NETWORK_INTERFACE_ATTR_LASER_GRID_SUPPORT:
            return TAI_STATUS_INVALID_ATTRIBUTE_0;
        case TAI_NETWORK_INTERFACE_ATTR_TX_ENABLE:
        case TAI_NETWORK_INTERFACE_ATTR_TX_GRID_SPACING:
        case TAI_NETWORK_INTERFACE_ATTR_TX_CHANNEL:
        case TAI_NETWORK_INTERFACE_ATTR_OUTPUT_POWER:
        case TAI_NETWORK_INTERFACE_ATTR_TX_FINE_TUNE_LASER_FREQ:
        case TAI_NETWORK_INTERFACE_ATTR_MODULATION_FORMAT:
        case TAI_NETWORK_INTERFACE_ATTR_DIFFERENTIAL_ENCODING:
        case TAI_NETWORK_INTERFACE_ATTR_LOOPBACK_TYPE:
            return TAI_STATUS_SUCCESS;
    }
    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
}

/**
 * @brief Set the values from a list of attributes
 *
 * @param [in] network_interface_id The network interface ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_set_network_interface_attributes(
   _In_ tai_object_id_t        network_interface_id,
   _In_ uint32_t               attr_count,
   _In_ const tai_attribute_t *attr_list)
{
    uint32_t idx;
    tai_status_t ret;

    for (idx = 0; idx < attr_count; idx++) {
        ret = stub_set_network_interface_attribute(network_interface_id, attr_list++);
        if (ret) {
            return convert_tai_error_to_list(ret, idx);
        }
    }
    return TAI_STATUS_SUCCESS; 
}

/**
 * @brief Network interface initialization. After the call the capability 
 *        attributes should be ready for retrieval via
 *        tai_get_network_interface_attribute().
 *
 * @param [out] network_interface_id Handle which identifies the network 
 *        interface
 * @param [in] module_id Module id on which the network interface exists
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to set during initialization
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_create_network_interface(
    _Out_ tai_object_id_t *network_interface_id,
    _In_ tai_object_id_t module_id,
    _In_ uint32_t attr_count,
    _In_ const tai_attribute_t *attr_list)
{
    tai_status_t ret;
    const tai_attribute_value_t * netif_addr;

    netif_addr = find_attribute_in_list(TAI_NETWORK_INTERFACE_ATTR_INDEX, attr_count, attr_list);
    if (NULL == netif_addr) {
        TAI_SYSLOG_ERROR("The required TAI_NETWORK_INTERFACE_ATTR_INDEX attribute was not provided");
        return TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    ret = stub_set_network_interface_attributes(*network_interface_id, attr_count, attr_list);
    if (TAI_STATUS_SUCCESS != ret) {
        TAI_SYSLOG_ERROR("Error setting network interface attributes");
        return ret;
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Release all resources associated with previously created network 
 *        interface
 *
 * @param [in] network_interface_id The network interface ID handle being 
 *        removed
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_remove_network_interface(_In_ tai_object_id_t network_interface_id)
{
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief The network interface functions. This structure is retrieved via the 
 *        #tai_api_query function.
 */
tai_network_interface_api_t stub_network_interface_api = {
    .create_network_interface         = stub_create_network_interface,
    .remove_network_interface         = stub_remove_network_interface,
    .set_network_interface_attribute  = stub_set_network_interface_attribute,
    .set_network_interface_attributes = stub_set_network_interface_attributes,
    .get_network_interface_attribute  = stub_get_network_interface_attribute,
    .get_network_interface_attributes = stub_get_network_interface_attributes
};


/*------------------------------------------------------------------------------

                             Module Object Functions

------------------------------------------------------------------------------*/

#undef  __TAI_MODULE__
#define __TAI_MODULE__ TAI_API_MODULE

/**
 * @brief Retrieve the value of an attribute
 *
 * @param [in] module_id The module ID handle
 * @param [in,out] attr A pointer to the attribute to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_get_module_attribute(
    _In_ tai_object_id_t     module_id,
    _Inout_ tai_attribute_t *attr)
{
    TAI_SYSLOG_DEBUG("Retrieving module attribute: %d", attr->id);
    switch (attr->id) {
        case TAI_MODULE_ATTR_LOCATION:
        case TAI_MODULE_ATTR_VENDOR_NAME:
        case TAI_MODULE_ATTR_VENDOR_PART_NUMBER:
        case TAI_MODULE_ATTR_VENDOR_SERIAL_NUMBER:
        case TAI_MODULE_ATTR_FIRMWARE_VERSIONS:
        case TAI_MODULE_ATTR_OPER_STATUS:
        case TAI_MODULE_ATTR_ADMIN_STATUS:
        case TAI_MODULE_ATTR_TEMP:
        case TAI_MODULE_ATTR_POWER:
        case TAI_MODULE_ATTR_NUM_HOST_INTERFACES:
        case TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES:
            return TAI_STATUS_SUCCESS;
    }
    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
}

/**
 * @brief Retrieve a list of attribute values
 *
 * @param [in] module_id The module ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in,out] attr_list A list of attributes to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_get_module_attributes(
    _In_ tai_object_id_t     module_id,
    _In_ uint32_t            attr_count,
    _Inout_ tai_attribute_t *attr_list)
{
    uint32_t idx;
    tai_status_t ret;

    for (idx = 0; idx < attr_count; idx++) {
        ret = stub_get_module_attribute(module_id, attr_list++);
        if (ret) {
            return convert_tai_error_to_list(ret, idx);
        }
    }
    return TAI_STATUS_SUCCESS; 
}

/**
 * @brief Set the value of an attribute
 *
 * @param [in] module_id The module ID handle
 * @param [in] attr A pointer to the attribute to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_set_module_attribute(
   _In_ tai_object_id_t        module_id,
   _In_ const tai_attribute_t *attr)
{
    TAI_SYSLOG_DEBUG("Setting module attribute: %d", attr->id);
    switch (attr->id) {
        case TAI_MODULE_ATTR_LOCATION:
            return TAI_STATUS_SUCCESS;
        case TAI_MODULE_ATTR_VENDOR_NAME:
        case TAI_MODULE_ATTR_VENDOR_PART_NUMBER:
        case TAI_MODULE_ATTR_VENDOR_SERIAL_NUMBER:
        case TAI_MODULE_ATTR_FIRMWARE_VERSIONS:
        case TAI_MODULE_ATTR_TEMP:
        case TAI_MODULE_ATTR_POWER:
        case TAI_MODULE_ATTR_NUM_HOST_INTERFACES:
        case TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES:
        case TAI_MODULE_ATTR_OPER_STATUS:
            return TAI_STATUS_INVALID_ATTRIBUTE_0;
        case TAI_MODULE_ATTR_ADMIN_STATUS:
            return TAI_STATUS_SUCCESS;
    }
    return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
}

/**
 * @brief Set the values from a list of attributes
 *
 * @param [in] module_id The module ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_set_module_attributes(
   _In_ tai_object_id_t        module_id,
   _In_ uint32_t               attr_count,
   _In_ const tai_attribute_t *attr_list)
{
    uint32_t idx;
    tai_status_t ret;

    for (idx = 0; idx < attr_count; idx++) {
        ret = stub_set_module_attribute(module_id, attr_list++);
        if (ret) {
            return convert_tai_error_to_list(ret, idx);
        }
    }
    return TAI_STATUS_SUCCESS; 
}

/**
 * @brief Module initialization. After the call the capability attributes should
 *        be ready for retrieval via tai_get_module_attribute().
 *
 * @param [out] module_id Handle which identifies the module
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to set during initialization
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_create_module(
    _Out_ tai_object_id_t          *module_id,
    _In_ uint32_t                   attr_count,
    _In_ const tai_attribute_t     *attr_list)
{
    tai_status_t ret;
    const tai_attribute_value_t * mod_addr;

    mod_addr = find_attribute_in_list(TAI_MODULE_ATTR_LOCATION, attr_count, attr_list);
    if (NULL == mod_addr) {
        TAI_SYSLOG_ERROR("The required TAI_MODULE_ATTR_LOCATION attribute was not provided");
        return TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    ret = stub_set_module_attributes(*module_id, attr_count, attr_list);
    if (TAI_STATUS_SUCCESS != ret) {
        TAI_SYSLOG_ERROR("Error setting module attributes");
        return ret;
    }

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Release all resources associated with previously created module
 *
 * @param [in] module_id The module ID handle being removed
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t stub_remove_module(_In_ tai_object_id_t module_id)
{
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief The module interface functions. This structure is retrieved via the 
 *        #tai_api_query function.
 */
tai_module_api_t stub_module_api = {
    .create_module         = stub_create_module,
    .remove_module         = stub_remove_module,
    .set_module_attribute  = stub_set_module_attribute,
    .set_module_attributes = stub_set_module_attributes,
    .get_module_attribute  = stub_get_module_attribute,
    .get_module_attributes = stub_get_module_attributes
};


/*------------------------------------------------------------------------------

                               TAI API Functions

------------------------------------------------------------------------------*/

/**
 *  @brief  Adapter module initialization call. This is NOT for SDK
 *          initialization.
 *
 *  @param [in] flags Reserved for future use, must be zero
 *  @param [in] services Methods table with services provided by adapter host
 *
 *  @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_api_initialize(_In_ uint64_t flags,
                                _In_ const tai_service_method_table_t* services)
{
    openlog("stub_tai_adapter", LOG_PID, LOG_USER);
    if (0 != flags) {
        TAI_SYSLOG_ERROR("Invalid flags passed to TAI API initialize");
        return TAI_STATUS_INVALID_PARAMETER;
    }

    if (NULL == services) {
        TAI_SYSLOG_ERROR("Invalid services handle passed to TAI API initialize");
        return TAI_STATUS_INVALID_PARAMETER;
    }

    memcpy(&adapter_host_fns, services, sizeof(adapter_host_fns));
    initialized = true; 

    return TAI_STATUS_SUCCESS;
}

/**
 *  @brief  Retrieve a pointer to the C-style method table for desired TAI 
 *          functionality as specified by the given tai_api_id.
 *
 *  @param [in] tai_api_id TAI api ID 
 *  @param [out] api_method_table Caller allocated method table. The table must
 *               remain valid until the tai_api_uninitialize() is called.
 *  @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_api_query(_In_ tai_api_t tai_api_id,
                           _Out_ void** api_method_table)
{
    if (!initialized) {
        TAI_SYSLOG_ERROR("TAI API not initialized before calling API query");
        return TAI_STATUS_UNINITIALIZED;
    }
    if (NULL == api_method_table) {
        TAI_SYSLOG_ERROR("NULL method table passed to TAI API initialize");
        return TAI_STATUS_INVALID_PARAMETER;
    }

    switch (tai_api_id) {
        case TAI_API_MODULE:
            *(const tai_module_api_t**)api_method_table = &stub_module_api;
            return TAI_STATUS_SUCCESS;

        case TAI_API_HOSTIF:
            *(const tai_host_interface_api_t**)api_method_table = 
                &stub_host_interface_api;
            return TAI_STATUS_SUCCESS;

        case TAI_API_NETWORKIF:
            *(const tai_network_interface_api_t**)api_method_table = 
                &stub_network_interface_api;
            return TAI_STATUS_SUCCESS;

        default:
            TAI_SYSLOG_ERROR("Invalid API type %d", tai_api_id);
            return TAI_STATUS_INVALID_PARAMETER;
    }
}

/**
 *  @brief  Uninitialization of the adapter module. TAI functionalities,
 *          retrieved via tai_api_query() cannot be used after this call.
 *
 *  @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_api_uninitialize(void)
{
    initialized = false;
    memset(&adapter_host_fns, 0, sizeof(adapter_host_fns));
    closelog();

    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Query tai object type.
 *
 * @param [in] tai_object_id
 *
 * @return Return #TAI_OBJECT_TYPE_NULL when tai_object_id is not valid.
 *         Otherwise, return a valid tai object type TAI_OBJECT_TYPE_XXX
 */
tai_object_type_t tai_object_type_query(_In_ tai_object_id_t tai_object_id)
{
    tai_object_type_t type = ((stub_object_id_t*)&tai_object_id)->type;
  
    if (TAI_OBJECT_TYPE_MAX > type) {
        return type;
    } else {
        TAI_SYSLOG_ERROR("Unknown type %d", type);
        return TAI_OBJECT_TYPE_NULL;
    }
}

/**
 * @brief Set log level for a tai api module. The default log level is 
 *        TAI_LOG_WARN.
 *
 * @param [in] tai_api_id - TAI api ID 
 * @param [in] log_level - log level
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_log_set(_In_ tai_api_t tai_api_id,
                         _In_ tai_log_level_t log_level)
{
    if ((TAI_API_UNSPECIFIED > tai_api_id) || (TAI_API_MAX <= tai_api_id)) {
        TAI_SYSLOG_ERROR("Invalid API type %d", tai_api_id);
        return TAI_STATUS_INVALID_PARAMETER;
    }

    if ((TAI_LOG_LEVEL_DEBUG > log_level) || (TAI_LOG_LEVEL_MAX <= log_level)) {
        TAI_SYSLOG_ERROR("Invalid log level %d\n", log_level);
        return TAI_STATUS_INVALID_PARAMETER;
    }

    api_log_level[tai_api_id] = tai_to_syslog_level[log_level];
    return TAI_STATUS_SUCCESS;
}

