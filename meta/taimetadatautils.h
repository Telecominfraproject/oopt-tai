/**
 * @file    taimetadatautils.h
 *
 * @brief   This module defines TAI Metadata Utilities
 *
 * @copyright Copyright (c) 2014 Microsoft Open Technologies, Inc.
 * @copyright Copyright (c) 2018 Nippon Telegraph and Telephone Corporation
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * This source code includes software licensed by Microsoft under the
 * Apache License, Version 2.0
 *
 */

#ifndef __TAIMETADATAUTILS_H_
#define __TAIMETADATAUTILS_H_

#include "taimetadatatypes.h"

/**
 * @defgroup TAIMETADATAUTILS TAI - Metadata Utilities Definitions
 *
 * @{
 */

/**
 * @brief Is allowed object type
 *
 * @param[in] metadata Attribute metadata
 * @param[in] object_type Object type to be checked
 *
 * @return True if object is allowed on this attribute, false otherwise
 */
extern bool tai_metadata_is_allowed_object_type(
        _In_ const tai_attr_metadata_t *metadata,
        _In_ tai_object_type_t object_type);

/**
 * @brief Is allowed enum value
 *
 * @param[in] metadata Attribute metadata
 * @param[in] value Enum value to be checked
 *
 * @return True if enum value is allowed on this attribute, false otherwise
 */
extern bool tai_metadata_is_allowed_enum_value(
        _In_ const tai_attr_metadata_t *metadata,
        _In_ int value);

/**
 * @brief Gets attribute metadata based on object type and attribute id
 *
 * @param[in] object_type Object type
 * @param[in] attr_id Attribute Id
 *
 * @return Pointer to object metadata or NULL in case of failure
 */
extern const tai_attr_metadata_t* tai_metadata_get_attr_metadata(
        _In_ tai_object_type_t object_type,
        _In_ tai_attr_id_t attr_id);

/**
 * @brief Gets attribute metadata based on attribute id name
 *
 * @param[in] attr_id_name Attribute id name
 *
 * @return Pointer to object metadata or NULL in case of failure
 */
extern const tai_attr_metadata_t* tai_metadata_get_attr_metadata_by_attr_id_name(
        _In_ const char *attr_id_name);

/**
 * @brief Gets string representation of enum value
 *
 * @param[in] metadata Enum metadata
 * @param[in] value Enum value to be converted to string
 *
 * @return String representation of enum value or NULL if value was not found
 */
extern const char* tai_metadata_get_enum_value_name(
        _In_ const tai_enum_metadata_t *metadata,
        _In_ int value);

/**
 * @brief Gets attribute from attribute list by attribute id.
 *
 * @param[in] id Attribute id to be found.
 * @param[in] attr_count Total number of attributes.
 * @param[in] attr_list List of attributes to search.
 *
 * @return Attribute pointer with requested ID or NULL if not found.
 * When multiple attributes with the same id are passed, only first
 * attribute is returned.
 */
extern const tai_attribute_t* tai_metadata_get_attr_by_id(
        _In_ tai_attr_id_t id,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list);

/**
 * @brief Gets object type info
 *
 * @param[in] object_type Object type
 *
 * @return Object type info structure or NULL if not found
 */
extern const tai_object_type_info_t* tai_metadata_get_object_type_info(
        _In_ tai_object_type_t object_type);

/**
 * @brief Checks if object type is valid
 *
 * @param[in] object_type Object type
 *
 * @return True if object type is valid, false otherwise
 */
extern bool tai_metadata_is_object_type_valid(
        _In_ tai_object_type_t object_type);

/**
 * @brief Check if condition met.
 *
 * List of attributes will be examined in terms of conditions. This is
 * convenient since user can pass list when calling create API. If
 * condition attribute is not on the list, then default value will be
 * examined.
 *
 * NOTE: When multiple attributes with the same ID are passed,
 * tai_metadata_get_attr_by_id will select only first one.
 * Function will not be able to handle multiple attributes
 *
 * @param[in] metadata Metadata of attribute that we need to check.
 * @param[in] attr_count Number of attributes.
 * @param[in] attr_list Attribute list to check. All attributes must
 * belong to the same object type as metadata parameter.
 *
 * @return True if condition is in force, false otherwise. False will be also
 * returned if any of input pointers is NULL or attribute is not conditional.
 */
extern bool tai_metadata_is_condition_met(
        _In_ const tai_attr_metadata_t *metadata,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list);

/**
 * @brief Allocation info
 *
 * hint for the allocation
 *
 */

typedef struct _tai_alloc_info_t {
    uint32_t list_size;

   /**
     * @brief reference attribute for size information
     */
    const tai_attribute_t* reference;
} tai_alloc_info_t;

/**
 * @brief Allocate tai_attribute_t value
 *
 * By passing info == NULL, it will use default list size for the list value
 * allocation
 *
 * @param[in] metadata Attribute metadata
 * @param[in] attr Attribute to allocate
 * @param[in] info Allocation information
 *
 * @return TAI_STATUS_SUCCESS on success,
 * TAI_STATUS_INVALID_PARAMETER/TAI_STATUS_NO_MEMORY on failure
 */
extern tai_status_t tai_metadata_alloc_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ tai_attribute_t* const attr,
        _In_ const tai_alloc_info_t* const info);

/**
 * @brief Free tai_attribute_t value
 *
 * @param[in] metadata Attribute metadata
 * @param[in] attr Attribute to free
 * @param[in] info Allocation information
 *
 * @return TAI_STATUS_SUCCESS on success,
 * TAI_STATUS_INVALID_PARAMETER on failure
 */
extern tai_status_t tai_metadata_free_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ tai_attribute_t* const attr,
        _In_ const tai_alloc_info_t* const info);

/**
 * @brief Clear tai_attribute_t value
 *
 * @param[in] metadata Attribute metadata
 * @param[in] attr Attribute to clear
 *
 * @return TAI_STATUS_SUCCESS on success,
 * TAI_STATUS_INVALID_PARAMETER on failure
 */
extern tai_status_t tai_metadata_clear_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ tai_attribute_t* const attr);

/**
 * @brief Deep copy tai_attribute_t value
 *
 * @param[in] metadata Attribute metadata
 * @param[in] in  original attribute for the copy
 * @param[in] out destination for the copy
 *
 * @return TAI_STATUS_SUCCESS on success,
 * TAI_STATUS_INVALID_PARAMETER on failure
 */
extern tai_status_t tai_metadata_deepcopy_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ const tai_attribute_t* const in,
        _Out_ tai_attribute_t* const out);

/**
 * @brief Deep equal tai_attribute_t value
 *
 * @param[in] metadata Attribute metadata
 * @param[in] lhs
 * @param[in] rhs
 * @param[out] result
 *
 * @return TAI_STATUS_SUCCESS on success,
 * TAI_STATUS_INVALID_PARAMETER on failure
 */
extern tai_status_t tai_metadata_deepequal_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ const tai_attribute_t* const lhs,
        _In_ const tai_attribute_t* const rhs,
        _Out_ bool* result);

/**
 * @}
 */
#endif /** __TAIMETADATAUTILS_H_ */
