/**
 *  @file    stub_tai.c
 *  @brief   The main TAI interface routines
 *  @author  Scott Emery <scotte@cumulusnetworks.com>
 *
 *  @copyright Copyright (C) 2018 Cumulus Networks, Inc. All rights reserved
 *  @copyright Copyright (c) 2018 Nippon Telegraph and Telephone Corporation
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */


#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include "tai.h"
#include "taimetadata.h"


static tai_service_method_table_t adapter_host_fns;
static bool                       initialized = false;

#define STUB_NUM_MODULE 4
#define STUB_NUM_HOSTIF 2
#define STUB_NUM_NETIF 1

typedef struct _tai_attr_node_t {
    tai_attribute_t attr;
    struct _tai_attr_node_t *next;
} tai_attr_node_t;

typedef struct _stub_object_t {
    tai_object_id_t oid;
    tai_attr_node_t *head;
} stub_object_t;

typedef struct _stub_module_t {
    stub_object_t module;
    stub_object_t hostifs[STUB_NUM_HOSTIF];
    stub_object_t netifs[STUB_NUM_NETIF];
} stub_module_t;

static stub_module_t g_modules[STUB_NUM_MODULE];

/*------------------------------------------------------------------------------

                              Utility Functions

------------------------------------------------------------------------------*/

#undef  __TAI_MODULE__
#define __TAI_MODULE__ TAI_API_UNSPECIFIED

/** @brief The Stub TAI adapter uses the following format for object ids */
typedef struct _stub_object_id_t {
    uint16_t value;
    uint16_t module;
    uint32_t :24;
    uint8_t  type;
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

/**
 * @brief Generic helper function for setting an attribute
 *
 * @param [in] head head of tai_attr_node list
 * @param [in] meta metadata of the attribute to set
 * @param [in] attr attribute to set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t _set_attribute(tai_attr_node_t** const head, const tai_attr_metadata_t* const meta, const tai_attribute_t* const attr) {
    tai_attr_node_t *p = *head, **update = head;
    tai_status_t ret;
    if ( p == NULL ) {
        goto alloc;
    }
    do {
        if ( p->attr.id == attr->id ) {
            return tai_metadata_deepcopy_attr_value(meta, attr, &p->attr);
        }
        update = &p->next;
        p = p->next;
    } while ( p != NULL );
alloc:
    p = (tai_attr_node_t*)calloc(1, sizeof(tai_attr_node_t));
    ret = tai_metadata_alloc_attr_value(meta, &p->attr, NULL);
    if ( ret != TAI_STATUS_SUCCESS ) {
        return ret;
    }
    ret = tai_metadata_deepcopy_attr_value(meta, attr, &p->attr);
    if ( ret != TAI_STATUS_SUCCESS ) {
        return ret;
    }
    *update = p;
    return TAI_STATUS_SUCCESS;
}

/**
 * @brief Generic helper function for getting an attribute
 *
 * @param [in] head head of tai_attr_node list
 * @param [in] meta metadata of the attribute to set
 * @param [in,out] attr A pointer to the attribute to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t _get_attribute(const tai_attr_node_t* const head, const tai_attr_metadata_t* const meta, tai_attribute_t* const attr) {
    const tai_attr_node_t* p = head;
    if ( p == NULL ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }
    do {
        if ( p->attr.id == attr->id ) {
            return tai_metadata_deepcopy_attr_value(meta, &p->attr, attr);
        }
        p = p->next;
    } while ( p != NULL );
    return TAI_STATUS_ITEM_NOT_FOUND;
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
    stub_object_id_t id = *(stub_object_id_t*)&host_interface_id;
    stub_object_t object;
    const tai_attr_metadata_t* meta;
    if ( id.type != TAI_OBJECT_TYPE_HOST_INTERFACE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.module >= STUB_NUM_MODULE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.value >= STUB_NUM_HOSTIF ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    object = g_modules[id.module].hostifs[id.value];
    if ( object.oid == 0 ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }

    TAI_SYSLOG_DEBUG("Retrieving host interface attribute: %d", attr->id);
    meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_HOST_INTERFACE, attr->id);
    if ( meta == NULL ) {
        return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
    }
    return _get_attribute(object.head, meta, attr);
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
    stub_object_id_t id = *(stub_object_id_t*)&host_interface_id;
    stub_object_t* object;
    const tai_attr_metadata_t* meta;
    if ( id.type != TAI_OBJECT_TYPE_HOST_INTERFACE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.module >= STUB_NUM_MODULE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.value >= STUB_NUM_HOSTIF ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    object = &g_modules[id.module].hostifs[id.value];
    if ( object->oid == 0 ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }

    TAI_SYSLOG_DEBUG("Setting host interface attribute: %d", attr->id);
    meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_HOST_INTERFACE, attr->id);
    if ( meta == NULL ) {
        return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
    }
    return _set_attribute(&object->head, meta, attr);
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
    int i;
    bool found = false;
    stub_object_id_t id = { .type = TAI_OBJECT_TYPE_HOST_INTERFACE };

    hostif_addr = find_attribute_in_list(TAI_HOST_INTERFACE_ATTR_INDEX, attr_count, attr_list);
    if (NULL == hostif_addr) {
        TAI_SYSLOG_ERROR("The required TAI_HOST_INTERFACE_ATTR_INDEX attribute was not provided");
        return TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    for ( i = 0; i < STUB_NUM_MODULE; i++ ) {
        if ( g_modules[i].module.oid == module_id ) {
            found = true;
            break;
        }
    }

    if ( !found ) {
        TAI_SYSLOG_ERROR("failed to create hostif: module %x not found", module_id);
        return TAI_STATUS_ITEM_NOT_FOUND;
    }

    if ( hostif_addr->u32 >= STUB_NUM_HOSTIF ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }

    if ( g_modules[i].hostifs[hostif_addr->u32].oid != 0 ) {
        return TAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    id.module = i;
    id.value = hostif_addr->u32;
    *host_interface_id = *(tai_object_id_t*)&id;
    g_modules[i].hostifs[hostif_addr->u32].oid = *host_interface_id;

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
    stub_object_id_t id = *(stub_object_id_t*)&host_interface_id;
    stub_object_t* object;
    tai_attr_node_t *p, *next;
    const tai_attr_metadata_t* meta;
    tai_status_t ret;
    if ( id.type != TAI_OBJECT_TYPE_HOST_INTERFACE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.module >= STUB_NUM_MODULE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.value >= STUB_NUM_HOSTIF ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    object = &g_modules[id.module].hostifs[id.value];
    if ( object->oid == 0 ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }
    p = object->head;
    while ( p != NULL ) {
        meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_HOST_INTERFACE, p->attr.id);
        ret = tai_metadata_free_attr_value(meta, &p->attr, NULL);
        if ( ret != TAI_STATUS_SUCCESS ) {
            return ret;
        }
        next = p->next;
        free(p);
        p = next;
    }
    object->oid = 0;
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
    stub_object_id_t id = *(stub_object_id_t*)&network_interface_id;
    stub_object_t object;
    const tai_attr_metadata_t* meta;
    if ( id.type != TAI_OBJECT_TYPE_NETWORK_INTERFACE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.module >= STUB_NUM_MODULE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.value >= STUB_NUM_NETIF ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    object = g_modules[id.module].netifs[id.value];
    if ( object.oid == 0 ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }

    TAI_SYSLOG_DEBUG("Retrieving network interface attribute: %d", attr->id);
    meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_NETWORK_INTERFACE, attr->id);
    if ( meta == NULL ) {
        return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
    }
    return _get_attribute(object.head, meta, attr);
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
    stub_object_id_t id = *(stub_object_id_t*)&network_interface_id;
    stub_object_t *object;
    const tai_attr_metadata_t* meta;
    if ( id.type != TAI_OBJECT_TYPE_NETWORK_INTERFACE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.module >= STUB_NUM_MODULE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.value >= STUB_NUM_NETIF ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    object = &g_modules[id.module].netifs[id.value];
    if ( object->oid == 0 ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }

    TAI_SYSLOG_DEBUG("Setting network interface attribute: %d", attr->id);
    meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_NETWORK_INTERFACE, attr->id);
    if ( meta == NULL ) {
        return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
    }
    return _set_attribute(&object->head, meta, attr);
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
    int i;
    bool found = false;
    stub_object_id_t id = { .type = TAI_OBJECT_TYPE_NETWORK_INTERFACE };

    netif_addr = find_attribute_in_list(TAI_NETWORK_INTERFACE_ATTR_INDEX, attr_count, attr_list);
    if (NULL == netif_addr) {
        TAI_SYSLOG_ERROR("The required TAI_NETWORK_INTERFACE_ATTR_INDEX attribute was not provided");
        return TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    for ( i = 0; i < STUB_NUM_MODULE; i++ ) {
        if ( g_modules[i].module.oid == module_id ) {
            found = true;
            break;
        }
    }

    if ( !found ) {
        TAI_SYSLOG_ERROR("failed to create netif: module %x not found", module_id);
        return TAI_STATUS_ITEM_NOT_FOUND;
    }

    if ( netif_addr->u32 >= STUB_NUM_NETIF ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }

    if ( g_modules[i].netifs[netif_addr->u32].oid != 0 ) {
        return TAI_STATUS_ITEM_ALREADY_EXISTS;
    }

    id.module = i;
    id.value = netif_addr->u32;
    *network_interface_id = *(tai_object_id_t*)&id;
    g_modules[i].netifs[netif_addr->u32].oid = *network_interface_id;

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
    stub_object_id_t id = *(stub_object_id_t*)&network_interface_id;
    stub_object_t* object;
    tai_attr_node_t *p, *next;
    const tai_attr_metadata_t* meta;
    tai_status_t ret;
    if ( id.type != TAI_OBJECT_TYPE_NETWORK_INTERFACE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.module >= STUB_NUM_MODULE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.value >= STUB_NUM_NETIF ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    object = &g_modules[id.module].netifs[id.value];
    if ( object->oid == 0 ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }
    p = object->head;
    while ( p != NULL ) {
        meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_NETWORK_INTERFACE, p->attr.id);
        ret = tai_metadata_free_attr_value(meta, &p->attr, NULL);
        if ( ret != TAI_STATUS_SUCCESS ) {
            return ret;
        }
        next = p->next;
        free(p);
        p = next;
    }
    object->oid = 0;
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
    stub_object_id_t id = *(stub_object_id_t*)&module_id;
    stub_object_t object;
    const tai_attr_metadata_t* meta;
    if ( id.type != TAI_OBJECT_TYPE_MODULE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.value >= STUB_NUM_MODULE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    object = g_modules[id.value].module;
    if ( object.oid == 0 ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }

    TAI_SYSLOG_DEBUG("Retrieving module attribute: %d", attr->id);
    meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_MODULE, attr->id);
    if ( meta == NULL ) {
        return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
    }
    switch (attr->id) {
        case TAI_MODULE_ATTR_NUM_HOST_INTERFACES:
            attr->value.u32 = STUB_NUM_HOSTIF;
            return TAI_STATUS_SUCCESS;
        case TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES:
            attr->value.u32 = STUB_NUM_NETIF;
            return TAI_STATUS_SUCCESS;
    }
    return _get_attribute(object.head, meta, attr);
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
    stub_object_id_t id = *(stub_object_id_t*)&module_id;
    stub_object_t *object;
    const tai_attr_metadata_t* meta = NULL;
    if ( id.type != TAI_OBJECT_TYPE_MODULE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.value >= STUB_NUM_MODULE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    object = &g_modules[id.value].module;
    if ( object->oid == 0 ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }

    TAI_SYSLOG_DEBUG("Setting module attribute: %d", attr->id);
    meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_MODULE, attr->id);
    if ( meta == NULL ) {
        return TAI_STATUS_ATTR_NOT_SUPPORTED_0;
    }
    return _set_attribute(&object->head, meta, attr);
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
    stub_object_id_t id = { .type = TAI_OBJECT_TYPE_MODULE };
    int i = -1;

    mod_addr = find_attribute_in_list(TAI_MODULE_ATTR_LOCATION, attr_count, attr_list);
    if (NULL == mod_addr) {
        TAI_SYSLOG_ERROR("The required TAI_MODULE_ATTR_LOCATION attribute was not provided");
        return TAI_STATUS_MANDATORY_ATTRIBUTE_MISSING;
    }

    sscanf(mod_addr->charlist.list, "%d", &i);
    if ( i < 0 || i > STUB_NUM_MODULE ) {
        TAI_SYSLOG_ERROR("Invalid module location: %s", mod_addr->charlist.list);
        return TAI_STATUS_FAILURE;
    }

    id.value = i;
    *module_id = *(tai_object_id_t*)&id;
    g_modules[i].module.oid = *module_id;

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
    stub_object_id_t id = *(stub_object_id_t*)&module_id;
    stub_object_t *object;
    tai_attr_node_t *p, *next;
    const tai_attr_metadata_t* meta;
    tai_status_t ret;
    int i;
    if ( id.type != TAI_OBJECT_TYPE_MODULE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( id.value >= STUB_NUM_MODULE ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    object = &g_modules[id.value].module;
    if ( object->oid == 0 ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }
    for ( i = 0; i < STUB_NUM_HOSTIF; i++ ) {
        if ( g_modules[id.value].hostifs[i].oid != 0 ) {
            return TAI_STATUS_OBJECT_IN_USE;
        }
    }
    for ( i = 0; i < STUB_NUM_NETIF; i++ ) {
        if ( g_modules[id.value].netifs[i].oid != 0 ) {
            return TAI_STATUS_OBJECT_IN_USE;
        }
    }
    p = object->head;
    while ( p != NULL ) {
        meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_MODULE, p->attr.id);
        ret = tai_metadata_free_attr_value(meta, &p->attr, NULL);
        if ( ret != TAI_STATUS_SUCCESS ) {
            return ret;
        }
        next = p->next;
        free(p);
        p = next;
    }
    object->oid = 0;
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
    memset(g_modules, 0, sizeof(g_modules));
    initialized = true; 

    if ( services->module_presence != NULL ) {
        int i;
        char name[16];
        for ( i = 0; i < STUB_NUM_MODULE; i++ ) {
            sprintf(name, "%d", i);
            services->module_presence(true, name);
        }
    }

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
 * @brief Query TAI module id.
 *
 * @param [in] tai_object_id Object id
 *
 * @return #TAI_NULL_OBJECT_ID when tai_object_id is not valid.
 * Otherwise, return a valid TAI_OBJECT_TYPE_MODULE object on which provided
 * object id belongs. If valid module id object is provided as input parameter
 * it should return itself.
 */
tai_object_id_t tai_module_id_query(_In_ tai_object_id_t tai_object_id)
{
    stub_object_id_t id = *(stub_object_id_t*)&tai_object_id;
    if ( id.type != TAI_OBJECT_TYPE_NETWORK_INTERFACE && id.type != TAI_OBJECT_TYPE_HOST_INTERFACE ) {
        return TAI_OBJECT_TYPE_NULL;
    }
    id.type = TAI_OBJECT_TYPE_MODULE;
    id.value = id.module;
    id.module = 0;
    return *(tai_object_id_t*)&id;
}

/**
 * @brief Set log level for a tai api module. The default log level is 
 *        TAI_LOG_WARN.
 *
 * @param [in] tai_api_id - TAI api ID 
 * @param [in] log_level - log level
 * @param [in] log_fn - log fn
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
tai_status_t tai_log_set(_In_ tai_api_t tai_api_id,
                         _In_ tai_log_level_t log_level,
                         _In_ tai_log_fn log_fn)
{
    if ((TAI_API_UNSPECIFIED > tai_api_id) || (TAI_API_MAX <= tai_api_id)) {
        TAI_SYSLOG_ERROR("Invalid API type %d", tai_api_id);
        return TAI_STATUS_INVALID_PARAMETER;
    }

    if ((TAI_LOG_LEVEL_DEBUG > log_level) || (TAI_LOG_LEVEL_MAX <= log_level)) {
        TAI_SYSLOG_ERROR("Invalid log level %d\n", log_level);
        return TAI_STATUS_INVALID_PARAMETER;
    }

    if ( log_fn != NULL ) {
        TAI_SYSLOG_ERROR("setting log handler is not supported");
        return TAI_STATUS_INVALID_PARAMETER;
    }

    api_log_level[tai_api_id] = tai_to_syslog_level[log_level];
    return TAI_STATUS_SUCCESS;
}

