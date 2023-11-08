/**
 * @file    taimeta.h
 * @brief   This module defines the meta interface for the Transponder
 *          Abstraction Interface (TAI)
 *
 * @copyright Copyright (c) 2021 Nippon Telegraph and Telephone Corporation
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *
 */

#if !defined (__TAIMETA_H_)
#define __TAIMETA_H_

#include <taitypes.h>
#include <taimetadatatypes.h>

/**
 * @defgroup TAIMETA TAI - Meta specific API definitions
 *
 * @{
 */

/**
 * @brief List metadata of the given oid object
 *
 * Passing TAI_NULL_OBJECT_ID to oid and TAI_OBJECT_TYPE_NULL to object_type
 * returns all metadata which this TAI library has.
 *
 * @param[in] key Metadata key
 * @param[out] count Number of metadata
 * @param[out] list Array of metadata
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_meta_list_metadata_fn)(
        _In_ const tai_metadata_key_t *const key,
        _Out_ uint32_t *count,
        _Out_ const tai_attr_metadata_t * const **list);

/**
 * @brief Get attribute metadata of the given oid object
 *
 * @param[in] key Metadata key
 * @param[in] attr_id Attribute Id
 *
 * @return Pointer to attribute metadata or NULL in case of failure
 */
typedef const tai_attr_metadata_t* (*tai_meta_get_attr_metadata_fn)(
        _In_ const tai_metadata_key_t *const key,
        _In_ tai_attr_id_t attr_id);

/**
 * @brief Get object info of the given oid object
 *
 * @param[in] key Metadata key
 *
 * @return Pointer to object metadata or NULL in case of failure
 */
typedef const tai_object_type_info_t* (*tai_meta_get_object_info_fn)(
        _In_ const tai_metadata_key_t *const key);

/**
 * @brief List object info of the given metadata key
 *
 * @param[in] key Metadata key
 * @param[out] count Number of object info
 * @param[out] list Array of object info
 *
 * @return #TAI_STATUS_SUCCESS on success, failure status code on error
 */
typedef tai_status_t (*tai_meta_list_object_info_fn)(
        _In_ const tai_metadata_key_t *const key,
        _Out_ uint32_t *count,
        _Out_ const tai_object_type_info_t* const **list);

/**
 * @brief Meta methods table retrieved with tai_api_query()
 */
typedef struct _tai_meta_api_t
{
    tai_meta_list_metadata_fn     list_metadata;
    tai_meta_get_attr_metadata_fn get_attr_metadata;
    tai_meta_get_object_info_fn   get_object_info;
    tai_meta_list_object_info_fn  list_object_info;
} tai_meta_api_t;

/**
 * @}
 */


#endif /** __TAIMETA_H_ */
