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
    TAI_MODULE_OPER_STATUS_RESET,               /**< Reset */
    TAI_MODULE_OPER_STATUS_INITIALIZE,          /**< Initialize */
    TAI_MODULE_OPER_STATUS_LOW_POWER,           /**< Low Power */
    TAI_MODULE_OPER_STATUS_HIGH_POWER_UP,       /**< High Power Up */
    TAI_MODULE_OPER_STATUS_TX_OFF,              /**< TX Off */
    TAI_MODULE_OPER_STATUS_TX_TURN_ON,          /**< TX Turn On */
    TAI_MODULE_OPER_STATUS_READY,               /**< Ready */
    TAI_MODULE_OPER_STATUS_TX_TURN_OFF,         /**< TX Turn Off */
    TAI_MODULE_OPER_STATUS_HIGH_POWER_DOWN,     /**< High Power Down */
    TAI_MODULE_OPER_STATUS_FAULT,               /**< Fault */
    TAI_MODULE_OPER_STATUS_MAX,                 /**< Number of states */

} tai_module_oper_status_t;

/**
 * @brief A bitmap of supported laser grid spacing
 */
typedef enum _tai_module_laser_grid_spacing_t
{
    TAI_MODULE_LASER_GRID_SPACING_6_25_GHZ = 0x20,
    TAI_MODULE_LASER_GRID_SPACING_12_5_GHZ = 0x10,
    TAI_MODULE_LASER_GRID_SPACING_25_GHZ   = 0x08,
    TAI_MODULE_LASER_GRID_SPACING_33_GHZ   = 0x04,
    TAI_MODULE_LASER_GRID_SPACING_50_GHZ   = 0x02,
    TAI_MODULE_LASER_GRID_SPACING_100_GHZ  = 0x01
} tai_module_laser_grid_spacing_t;

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
     * @type #tai_char_list_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_VENDOR_NAME,

    /**
     * @brief The module vendor's part number
     * @type #tai_char_list_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_VENDOR_PART_NUMBER,

    /**
     * @brief The module vendor's serial number
     * @type #tai_char_list_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_VENDOR_SERIAL_NUMBER,

    /**
     * @brief The module firmware versions
     *
     * The firmware versions for firmware A (list index 0) and firmware B (list
     * index 1) in x.y format.
     *
     * @type #tai_float_list_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_FIRMWARE_VERSIONS,

    /**
     * @brief The TX/RX minimum laser frequency in Hz
     *
     * @type #tai_uint64_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_MIN_LASER_FREQ,

    /**
     * @brief The TX/RX maximum laster frequency in Hz
     *
     * @type #tai_uint64_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_MAX_LASER_FREQ,

    /**
     * @brief The TX laser fine tune frequency range in Hz
     *
     * @type #tai_uint64_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_FINE_TUNE_LASER_FREQ,

    /**
     * @brief The laser grid spacing support. A bitfield of the supported grid
     *        spacing.
     *
     * @type #tai_module_laser_grid_spacing_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_LASER_GRID_SUPPORT,

    /**
     * @brief The maximum number of laser tuning channels
     *
     * @type #tai_uint32_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_MAX_LASER_CHANNELS,

    /**
     * @brief The operational state of the module
     *
     * @type #tai_module_oper_status_t
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
     * @brief End of attributes
     */
    TAI_MODULE_ATTR_END,

    /** Custom range base value */
    TAI_MODULE_ATTR_CUSTOM_RANGE_START = 0x10000000,

    /** End of custom range base */
    TAI_MODULE_ATTR_CUSTOM_RANGE_END

} tai_module_attr_t;

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
 *  @brief Module notification table. Functions are provided by adapter host to
 *         adapter in create_module function.
 */
typedef struct _tai_module_notification_t
{
    tai_module_shutdown_request_notification_fn shutdown_request;
    tai_module_state_change_notification_fn     state_change;
} tai_module_notification_t;

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
        _In_ const tai_attribute_t *attr_list,
        _In_ tai_module_notification_t *notifications);

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
