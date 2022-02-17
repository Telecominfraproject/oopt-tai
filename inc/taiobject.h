/**
 * @file    taiobject.h
 * @brief   This module defines the TAI Object interface
 *
 * @copyright Copyright (c) 2021 Nippon Telegraph and Telephone Corporation
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *
 */

#if !defined (__TAIOBJECT_H_)
#define __TAIOBJECT_H_

#include <taitypes.h>

/**
 * @brief Create object
 *
 * SDK initialization/connect to SDK. After the call the capability attributes
 * should be ready for retrieval via tai_get_module_attribute(). Returned Module
 * Object id should be used in subsequent TAI function calls in order to
 * identify the module.
 *
 * @param[out] oid The Object ID
 * @param[in] object_type The Object Type
 * @param[in] module_id The Object ID of the module that this object belongs to
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes to set during initialization
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_create_object_fn)(
        _Out_ tai_object_id_t *oid,
        _In_ tai_object_type_t object_type,
        _In_ tai_object_id_t module_id,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list);

/**
 * @brief Remove/disconnect object
 *
 * Release all resources associated with a currently created object
 *
 * @param[in] oid The Object id
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_remove_object_fn)(
        _In_ tai_object_id_t oid);

/**
 * @brief Set multiple object attribute values
 *
 * @param[in] oid Object id
 * @param[in] attr_count Number of attributes
 * @param[in] attr_list Array of attributes
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_set_object_attributes_fn)(
        _In_ tai_object_id_t oid,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list);

/**
 * @brief Get multiple object attribute values
 *
 * @param[in] oid Object id
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Array of attributes
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_get_object_attributes_fn)(
        _In_ tai_object_id_t oid,
        _In_ uint32_t attr_count,
        _Inout_ tai_attribute_t *attr_list);

/**
 * @brief Get multiple object capabilities
 *
 * @param[in] oid Object id
 * @param[in] count Number of capabilities
 * @param[inout] list Attribute capabilities
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_get_object_capabilities_fn)(
        _In_ tai_object_id_t oid,
        _In_ uint32_t count,
        _Inout_ tai_attribute_capability_t *list);

/**
 * @brief Object method table retrieved with tai_api_query()
 */
typedef struct _tai_object_api_t
{
    tai_create_object_fn            create_object;
    tai_remove_object_fn            remove_object;
    tai_set_object_attributes_fn    set_object_attributes;
    tai_get_object_attributes_fn    get_object_attributes;
    tai_get_object_capabilities_fn  get_object_capabilities;
} tai_object_api_t;

/**
 * @}
 */
#endif /** __TAIOBJECT_H_ */
