/**
 * @file    taimodule.h
 * @brief   This module defines the TAI Module interface
 * @details A "module" in this context refers to an optical module. The
 *          #tai_create_module_fn causes a new module to be allocated and
 *          initialized, including the SDK which controls that module.
 *
 * @copyright Copyright (c) 2014 Microsoft Open Technologies, Inc.
 * @copyright Copyright (c) 2018 Nippon Telegraph and Telephone Corporation
 * @copyright Copyright (c) 2017 Cumulus Networks, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *
 */

#if !defined (__TAIMODULE_H_)
#define __TAIMODULE_H_

#include <taitypes.h>

/**
 * @defgroup TAIMODULE TAI - Module specific API definitions
 *
 * @{
 */

/**
 * @brief Maximum Number of Modules
 */
#define TAI_MAX_MODULES                         32

/**
 * @brief Maximum Hardware ID Length
 */
#define TAI_MAX_HARDWARE_ID_LEN                 255

/**
 * @brief Maximum Firmware Path Name Length
 */
#define TAI_MAX_FIRMWARE_PATH_NAME_LEN          PATH_MAX

/**
 * @brief Operational states of the module
 */
typedef enum _tai_module_oper_status_t
{
    TAI_MODULE_OPER_STATUS_UNKNOWN,             /**< Unknown */
    TAI_MODULE_OPER_STATUS_INITIALIZE,          /**< Initialize */
    TAI_MODULE_OPER_STATUS_READY,               /**< Ready */
    TAI_MODULE_OPER_STATUS_MAX,                 /**< Number of states */
} tai_module_oper_status_t;

/**
 * @brief Admin states of the module
 */
typedef enum _tai_module_admin_status_t
{
    TAI_MODULE_ADMIN_STATUS_UNKNOWN,             /**< Unknown */
    TAI_MODULE_ADMIN_STATUS_DOWN,                /**< Down */
    TAI_MODULE_ADMIN_STATUS_UP,                  /**< Up */
    TAI_MODULE_ADMIN_STATUS_MAX,                 /**< Number of states */
} tai_module_admin_status_t;

/**
 * @brief Module shutdown request callback.
 *
 * Adapter DLL may request a shutdown due to an unrecoverable failure
 * or a maintenance operation
 *
 * @param[in] module_id Module Id
 */
typedef void (*tai_module_shutdown_request_notification_fn)(
        _In_ tai_object_id_t module_id);

/**
 * @brief Module operational state change notification
 *
 * @param[in] module_id Module Id
 * @param[in] module_oper_status New module operational state
 */
typedef void (*tai_module_state_change_notification_fn)(
        _In_ tai_object_id_t module_id,
        _In_ tai_module_oper_status_t module_oper_status);

/**
 * @brief Attribute Id in tai_set_module_attribute() and
 *        tai_get_module_attribute() calls.
 */
typedef enum _tai_module_attr_t
{
    /**
     * @brief Start of attributes
     */
    TAI_MODULE_ATTR_START,

    /**
     * @brief The location of the module
     *
     * Used (and required) in the tai_create_module_fn call. This allows the
     * adapter to uniquely identify the module. This could be a PCI address,
     * slot identifier, or other value that allows the adapter to determine
     * which optical module is being initialized.
     *
     * @type #tai_char_list_t
     * @flags MANDATORY_ON_CREATE | CREATE_ONLY
     */
    TAI_MODULE_ATTR_LOCATION  = TAI_MODULE_ATTR_START,

    /**
     * @brief The module vendor's name
     *
     * If the module is composed of several components, this attribute
     * contains the vendor name of all components with '/' used as
     * the delimiter
     *
     * @type #tai_char_list_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_VENDOR_NAME,

    /**
     * @brief The module vendor's part number
     *
     * If the module is composed of several components, this attribute
     * contains the part number of all components with '/' used as
     * the delimiter
     *
     * @type #tai_char_list_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_VENDOR_PART_NUMBER,

    /**
     * @brief The module vendor's serial number
     *
     * If there are several serial number exists inside the module, '/' is
     * used as the delimiter
     *
     * @type #tai_char_list_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_VENDOR_SERIAL_NUMBER,

    /**
     * @brief The module firmware version
     *
     * If there are several firmware exists inside the module, '/' is used
     * as the delimiter
     *
     * @type #tai_char_list_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_FIRMWARE_VERSION,

    /**
     * @brief The operational state of the module
     *
     * @type #tai_module_oper_status_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_OPER_STATUS,

    /**
     * @brief The internal temperature of the module
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_TEMP,

    /**
     * @brief The power supply voltage
     *
     * @type #tai_float_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_POWER,

    /**
     * @brief The number of host interfaces on the module
     *
     * @type #tai_uint32_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_NUM_HOST_INTERFACES,

    /**
     * @brief The number of network interfaces on the module
     *
     * @type #tai_uint32_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES,

    /**
     * @brief The admin state of the module
     *
     * @type #tai_module_admin_status_t
     * @flags CREATE_AND_SET
     * @default TAI_MODULE_ADMIN_STATUS_UP
     */
    TAI_MODULE_ATTR_ADMIN_STATUS,

    /**
     * @brief Tributary mapping of netif and hostif
     *
     * The key is netif oid and the value is a list of hostif oids
     * corresponds to the netif. This attribute can be changed automatically
     * by setting some netif attributes (e.g TAI_NETWORK_INTERFACE_ATTR_MODULATION_FORMAT )
     *
     * @type #tai_object_map_list_t
     * @flags CREATE_AND_SET
     * @defaults empty-list
     */
    TAI_MODULE_ATTR_TRIBUTARY_MAPPING,

    /**
     * @brief Module shutdown request callback.
     *
     * @type tai_pointer_t tai_module_shutdown_request_notification_fn
     * @flags CREATE_AND_SET
     * @default NULL
     */
    TAI_MODULE_ATTR_MODULE_SHUTDOWN_REQUEST_NOTIFY,

    /**
     * @brief Module operational state change notification
     *
     * @type tai_pointer_t tai_module_state_change_notification_fn
     * @flags CREATE_AND_SET
     * @default NULL
     */
    TAI_MODULE_ATTR_MODULE_STATE_CHANGE_NOTIFY,

    /**
     * @brief Module generic notification
     *
     * @type #tai_notification_handler_t
     * @flags CREATE_AND_SET
     * @default NULL
     */
    TAI_MODULE_ATTR_NOTIFY,

    /**
     * @brief End of attributes
     */
    TAI_MODULE_ATTR_END,

    /** Custom range base value */
    TAI_MODULE_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /** Custom range for the AC400 adapter */
    TAI_MODULE_ATTR_CUSTOM_AC400_START = TAI_MODULE_ATTR_CUSTOM_RANGE_START,
    TAI_MODULE_ATTR_CUSTOM_AC400_END   = TAI_MODULE_ATTR_CUSTOM_AC400_START + 0xFFFF,

    /** Custom range for the NLD0670APB/TRB100 adapter */
    TAI_MODULE_ATTR_CUSTOM_NLD0670_TRB100_START,
    TAI_MODULE_ATTR_CUSTOM_NLD0670_TRB100_END = TAI_MODULE_ATTR_CUSTOM_NLD0670_TRB100_START + 0xFFFF,

    /** Custom range for TAI mux */
    TAI_MODULE_ATTR_CUSTOM_MUX_START,
    TAI_MODULE_ATTR_CUSTOM_MUX_END = TAI_MODULE_ATTR_CUSTOM_MUX_START + 0xFFFF,

    /** End of custom range base */
    TAI_MODULE_ATTR_CUSTOM_RANGE_END

} tai_module_attr_t;

/**
 * @brief Create module
 *
 * SDK initialization/connect to SDK. After the call the capability attributes
 * should be ready for retrieval via tai_get_module_attribute(). Returned Module
 * Object id should be used in subsequent TAI function calls in order to
 * identify the module.
 *
 * @param[out] module_id The Module Object ID
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes to set during initialization
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_create_module_fn)(
        _Out_ tai_object_id_t *module_id,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list);

/**
 * @brief Remove/disconnect Module
 *
 * Release all resources associated with a currently created module
 *
 * @param[in] module_id The Module id
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_remove_module_fn)(
        _In_ tai_object_id_t module_id);

/**
 * @brief Set module attribute value
 *
 * @param[in] module_id Module id
 * @param[in] attr Module attribute
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_set_module_attribute_fn)(
        _In_ tai_object_id_t module_id,
        _In_ const tai_attribute_t *attr);

/**
 * @brief Set multiple module attribute values
 *
 * @param[in] module_id Module id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of module attributes
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_set_module_attributes_fn)(
        _In_ tai_object_id_t module_id,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list);

/**
 * @brief Get module attribute value
 *
 * @param[in] module_id Module id
 * @param[inout] attr Module attribute
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_get_module_attribute_fn)(
        _In_ tai_object_id_t module_id,
        _Inout_ tai_attribute_t *attr);

/**
 * @brief Get multiple module attribute values
 *
 * @param[in] module_id Module id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of module attributes
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_get_module_attributes_fn)(
        _In_ tai_object_id_t module_id,
        _In_ uint32_t attr_count,
        _Inout_ tai_attribute_t *attr_list);

/**
 * @brief Module method table retrieved with tai_api_query()
 */
typedef struct _tai_module_api_t
{
    tai_create_module_fn            create_module;
    tai_remove_module_fn            remove_module;
    tai_set_module_attribute_fn     set_module_attribute;
    tai_set_module_attributes_fn    set_module_attributes;
    tai_get_module_attribute_fn     get_module_attribute;
    tai_get_module_attributes_fn    get_module_attributes;

} tai_module_api_t;

/**
 * @}
 */
#endif /** __TAIMODULE_H_ */
