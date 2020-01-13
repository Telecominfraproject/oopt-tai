/**
 * @file    taimetadatautils.c
 *
 * @brief   This module implements TAI Metadata Utilities
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <tai.h>
#include "taimetadatautils.h"
#include "taimetadata.h"

bool tai_metadata_is_allowed_object_type(
        _In_ const tai_attr_metadata_t* metadata,
        _In_ tai_object_type_t object_type)
{
    if (metadata == NULL || metadata->allowedobjecttypes == NULL)
    {
        return false;
    }

    size_t i = 0;

    for (; i < metadata->allowedobjecttypeslength; ++i)
    {
        if (metadata->allowedobjecttypes[i] == object_type)
        {
            return true;
        }
    }

    return false;
}

bool tai_metadata_is_allowed_enum_value(
        _In_ const tai_attr_metadata_t* metadata,
        _In_ int value)
{
    if (metadata == NULL || metadata->enummetadata == NULL)
    {
        return false;
    }

    size_t i = 0;

    const tai_enum_metadata_t *emd = metadata->enummetadata;

    for (; i < emd->valuescount; ++i)
    {
        if (emd->values[i] == value)
        {
            return true;
        }
    }

    return false;
}

const tai_attr_metadata_t* tai_metadata_get_attr_metadata(
        _In_ tai_object_type_t objecttype,
        _In_ tai_attr_id_t attrid)
{
    if (tai_metadata_is_object_type_valid(objecttype))
    {
        const tai_attr_metadata_t* const* const md = tai_metadata_attr_by_object_type[objecttype];

        /*
         * Most obejct attributes are not flags, so we can use direct index to
         * find attribute metadata, this should speed up search.
         */

        const tai_object_type_info_t* oi = tai_metadata_all_object_type_infos[objecttype];

        if (!oi->enummetadata->containsflags && attrid < oi->enummetadata->valuescount)
        {
            return md[attrid];
        }

        /* otherwise search one by one */

        size_t index = 0;

        for (; md[index] != NULL; index++)
        {
            if (md[index]->attrid == attrid)
            {
                return md[index];
            }
        }
    }

    return NULL;
}

const tai_attr_metadata_t* tai_metadata_get_attr_metadata_by_attr_id_name(
        _In_ const char *attr_id_name)
{
    if (attr_id_name == NULL)
    {
        return NULL;
    }

    /* use binary search */

    ssize_t first = 0;
    ssize_t last = (ssize_t)(tai_metadata_attr_sorted_by_id_name_count - 1);

    while (first <= last)
    {
        ssize_t middle = (first + last) / 2;

        int res = strcmp(attr_id_name, tai_metadata_attr_sorted_by_id_name[middle]->attridname);

        if (res > 0)
        {
            first = middle + 1;
        }
        else if (res < 0)
        {
            last = middle - 1;
        }
        else
        {
            /* found */

            return tai_metadata_attr_sorted_by_id_name[middle];
        }
    }

    /* not found */

    return NULL;
}

const char* tai_metadata_get_enum_value_name(
        _In_ const tai_enum_metadata_t* metadata,
        _In_ int value)
{
    if (metadata == NULL)
    {
        return NULL;
    }

    size_t i = 0;

    for (; i < metadata->valuescount; ++i)
    {
        if (metadata->values[i] == value)
        {
            return metadata->valuesnames[i];
        }
    }

    return NULL;
}

const tai_attribute_t* tai_metadata_get_attr_by_id(
        _In_ tai_attr_id_t id,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list)
{
    if (attr_list == NULL)
    {
        return NULL;
    }

    uint32_t i = 0;

    for (; i < attr_count; ++i)
    {
        if (attr_list[i].id == id)
        {
            return &attr_list[i];
        }
    }

    return NULL;
}

const tai_object_type_info_t* tai_metadata_get_object_type_info(
        _In_ tai_object_type_t object_type)
{
    if (tai_metadata_is_object_type_valid(object_type))
    {
        return tai_metadata_all_object_type_infos[object_type];
    }

    return NULL;
}

bool tai_metadata_is_object_type_valid(
        _In_ tai_object_type_t object_type)
{
    return object_type > TAI_OBJECT_TYPE_NULL && object_type < TAI_OBJECT_TYPE_MAX;
}

bool tai_metadata_is_condition_met(
        _In_ const tai_attr_metadata_t *metadata,
        _In_ uint32_t attr_count,
        _In_ const tai_attribute_t *attr_list)
{
    if (metadata == NULL || !metadata->isconditional || attr_list == NULL)
    {
        return false;
    }

    size_t idx = 0;

    bool met = (metadata->conditiontype == TAI_ATTR_CONDITION_TYPE_AND);

    for (; idx < metadata->conditionslength; ++idx)
    {
        const tai_attr_condition_t *condition = metadata->conditions[idx];

        /*
         * Conditons may only be on the same object type.
         *
         * Default value may not exists if conditional object is marked as
         * MANDATORY_ON_CREATE.
         */

        const tai_attr_metadata_t *cmd = tai_metadata_get_attr_metadata(metadata->objecttype, condition->attrid);

        const tai_attribute_t *cattr = tai_metadata_get_attr_by_id(condition->attrid, attr_count, attr_list);

        const tai_attribute_value_t* cvalue = NULL;

        if (cattr == NULL)
        {
            /*
             * User didn't passed conditional attribute, so check if there is
             * default value.
             */

            cvalue = cmd->defaultvalue;
        }
        else
        {
            cvalue = &cattr->value;
        }

        if (cvalue == NULL)
        {
            /*
             * There is no default value and user didn't passed attribute.
             */

            if (metadata->conditiontype == TAI_ATTR_CONDITION_TYPE_AND)
            {
                return false;
            }

            continue;
        }

        bool current = false;

        switch (cmd->attrvaluetype)
        {
            case TAI_ATTR_VALUE_TYPE_BOOLDATA:
                current = (condition->condition.booldata == cvalue->booldata);
                break;
            case TAI_ATTR_VALUE_TYPE_S8:
                current = (condition->condition.s8 == cvalue->s8);
                break;
            case TAI_ATTR_VALUE_TYPE_S16:
                current = (condition->condition.s16 == cvalue->s16);
                break;
            case TAI_ATTR_VALUE_TYPE_S32:
                current = (condition->condition.s32 == cvalue->s32);
                break;
            case TAI_ATTR_VALUE_TYPE_S64:
                current = (condition->condition.s64 == cvalue->s64);
                break;
            case TAI_ATTR_VALUE_TYPE_U8:
                current = (condition->condition.u8 == cvalue->u8);
                break;
            case TAI_ATTR_VALUE_TYPE_U16:
                current = (condition->condition.u16 == cvalue->u16);
                break;
            case TAI_ATTR_VALUE_TYPE_U32:
                current = (condition->condition.u32 == cvalue->u32);
                break;
            case TAI_ATTR_VALUE_TYPE_U64:
                current = (condition->condition.u64 == cvalue->u64);
                break;

            default:

                /*
                 * We should never get here since sanity check tests all
                 * attributes and all conditions.
                 */

                TAI_META_LOG_ERROR("condition value type %d is not supported, FIXME", cmd->attrvaluetype);

                return false;
        }

        if (metadata->conditiontype == TAI_ATTR_CONDITION_TYPE_AND)
        {
            met &= current;
        }
        else /* OR */
        {
            met |= current;
        }
    }

    return met;
}

#define DEFAULT_LIST_SIZE 16

#define _TAI_META_ALLOC_LIST(valuename, type)\
    value->valuename.count = size;\
    value->valuename.list = realloc(value->valuename.list, size * sizeof(type));\
    if ( value->valuename.list == NULL ) {\
        return TAI_STATUS_NO_MEMORY;\
    }\

#define _TAI_META_FREE_LIST(valuename)\
    value->valuename.count = 0;\
    free(value->valuename.list);\
    value->valuename.list = NULL;\

#define _TAI_META_COPY_LIST(valuename, type)\
    if( out->valuename.count < in->valuename.count ) {\
        out->valuename.count = in->valuename.count;\
        return TAI_STATUS_BUFFER_OVERFLOW;\
    }\
    out->valuename.count = in->valuename.count;\
    memcpy(out->valuename.list, in->valuename.list, sizeof(type) * in->valuename.count);

#define _TAI_META_CMP(result, valuename)                       \
    *result = ( lhs->value.valuename == rhs->value.valuename );\
    break;

#define _TAI_META_CMP_LIST(result, valuename)                                 \
    {                                                                         \
    if( lhs->value.valuename.count != rhs->value.valuename.count ) {          \
        *result = false;                                                      \
        return TAI_STATUS_SUCCESS;                                            \
    }                                                                         \
    for ( int i = 0; i < lhs->value.valuename.count; i++ ) {                  \
        if ( lhs->value.valuename.list[i] != rhs->value.valuename.list[i] ) { \
            *result = false;                                                  \
            return TAI_STATUS_SUCCESS;                                        \
        }                                                                     \
    }                                                                         \
    *result = true;                                                           \
    }                                                                         \
    break;

tai_status_t _tai_metadata_free_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ tai_attribute_value_t* const value,
        _In_ const tai_alloc_info_t* const info) {
    if ( metadata == NULL || value == NULL ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    int i;
    tai_attr_metadata_t m = *metadata;
    m.attrvaluetype = m.attrlistvaluetype;
    m.attrlistvaluetype = TAI_ATTR_VALUE_TYPE_UNSPECIFIED;

    switch( metadata->attrvaluetype ) {
    case TAI_ATTR_VALUE_TYPE_BOOLDATA:
    case TAI_ATTR_VALUE_TYPE_CHARDATA:
    case TAI_ATTR_VALUE_TYPE_U8:
    case TAI_ATTR_VALUE_TYPE_S8:
    case TAI_ATTR_VALUE_TYPE_U16:
    case TAI_ATTR_VALUE_TYPE_S16:
    case TAI_ATTR_VALUE_TYPE_U32:
    case TAI_ATTR_VALUE_TYPE_S32:
    case TAI_ATTR_VALUE_TYPE_U64:
    case TAI_ATTR_VALUE_TYPE_S64:
    case TAI_ATTR_VALUE_TYPE_FLT:
    case TAI_ATTR_VALUE_TYPE_PTR:
    case TAI_ATTR_VALUE_TYPE_OID:
    case TAI_ATTR_VALUE_TYPE_U32RANGE:
    case TAI_ATTR_VALUE_TYPE_S32RANGE:
    case TAI_ATTR_VALUE_TYPE_NOTIFICATION:
        return TAI_STATUS_SUCCESS;
    case TAI_ATTR_VALUE_TYPE_OBJLIST:
        _TAI_META_FREE_LIST(objlist);
        break;
    case TAI_ATTR_VALUE_TYPE_CHARLIST:
        _TAI_META_FREE_LIST(charlist);
        break;
    case TAI_ATTR_VALUE_TYPE_U8LIST:
        _TAI_META_FREE_LIST(u8list);
        break;
    case TAI_ATTR_VALUE_TYPE_S8LIST:
        _TAI_META_FREE_LIST(s8list);
        break;
    case TAI_ATTR_VALUE_TYPE_U16LIST:
        _TAI_META_FREE_LIST(u16list);
        break;
    case TAI_ATTR_VALUE_TYPE_S16LIST:
        _TAI_META_FREE_LIST(s16list);
        break;
    case TAI_ATTR_VALUE_TYPE_U32LIST:
        _TAI_META_FREE_LIST(u32list);
        break;
    case TAI_ATTR_VALUE_TYPE_S32LIST:
        _TAI_META_FREE_LIST(s32list);
        break;
    case TAI_ATTR_VALUE_TYPE_FLOATLIST:
        _TAI_META_FREE_LIST(floatlist);
        break;
    case TAI_ATTR_VALUE_TYPE_OBJMAPLIST:
        // we must use alloced field instead of count field since
        // count field is not always equal to the allocated number
        for ( i = 0; i < value->objmaplist._alloced; i++ ) {
            value->objmaplist.list[i].value.count = 0;
            free(value->objmaplist.list[i].value.list);
            value->objmaplist.list[i].value.list = NULL;
        }
        _TAI_META_FREE_LIST(objmaplist);
        break;
    case TAI_ATTR_VALUE_TYPE_ATTRLIST:
        // we must use alloced field instead of count field since
        // count field is not always equal to the allocated number
        for ( i = 0; i < value->attrlist._alloced; i++ ) {
            if ( _tai_metadata_free_attr_value(&m, &value->attrlist.list[i], info) < 0 ) {
                return TAI_STATUS_FAILURE;
            }
        }
        _TAI_META_FREE_LIST(attrlist);
        break;
    default:
        TAI_META_LOG_ERROR("unsupported value type: %d", metadata->attrvaluetype);
        return TAI_STATUS_NOT_SUPPORTED;
    }
    return TAI_STATUS_SUCCESS;
}

static tai_status_t _tai_metadata_clear_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ tai_attribute_value_t* const value) {
    if ( metadata == NULL || value == NULL ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    switch( metadata->attrvaluetype ) {
    case TAI_ATTR_VALUE_TYPE_BOOLDATA:
    case TAI_ATTR_VALUE_TYPE_CHARDATA:
    case TAI_ATTR_VALUE_TYPE_U8:
    case TAI_ATTR_VALUE_TYPE_S8:
    case TAI_ATTR_VALUE_TYPE_U16:
    case TAI_ATTR_VALUE_TYPE_S16:
    case TAI_ATTR_VALUE_TYPE_U32:
    case TAI_ATTR_VALUE_TYPE_S32:
    case TAI_ATTR_VALUE_TYPE_U64:
    case TAI_ATTR_VALUE_TYPE_S64:
    case TAI_ATTR_VALUE_TYPE_FLT:
    case TAI_ATTR_VALUE_TYPE_PTR:
    case TAI_ATTR_VALUE_TYPE_OID:
    case TAI_ATTR_VALUE_TYPE_U32RANGE:
    case TAI_ATTR_VALUE_TYPE_S32RANGE:
    case TAI_ATTR_VALUE_TYPE_NOTIFICATION:
        memset(value, 0, sizeof(tai_attribute_value_t));
        break;
    case TAI_ATTR_VALUE_TYPE_OBJLIST:
        value->objlist.count = 0;
        break;
    case TAI_ATTR_VALUE_TYPE_CHARLIST:
        value->charlist.count = 0;
        break;
    case TAI_ATTR_VALUE_TYPE_U8LIST:
        value->u8list.count = 0;
        break;
    case TAI_ATTR_VALUE_TYPE_S8LIST:
        value->s8list.count = 0;
        break;
    case TAI_ATTR_VALUE_TYPE_U16LIST:
        value->u16list.count = 0;
        break;
    case TAI_ATTR_VALUE_TYPE_S16LIST:
        value->s16list.count = 0;
        break;
    case TAI_ATTR_VALUE_TYPE_U32LIST:
        value->u32list.count = 0;
        break;
    case TAI_ATTR_VALUE_TYPE_S32LIST:
        value->s32list.count = 0;
        break;
    case TAI_ATTR_VALUE_TYPE_FLOATLIST:
        value->floatlist.count = 0;
        break;
    case TAI_ATTR_VALUE_TYPE_OBJMAPLIST:
        value->objmaplist.count = 0;
        break;
    case TAI_ATTR_VALUE_TYPE_ATTRLIST:
        value->attrlist.count = 0;
        break;
    default:
        TAI_META_LOG_ERROR("unsupported value type: %d", metadata->attrvaluetype);
        return TAI_STATUS_NOT_SUPPORTED;
    }
    return TAI_STATUS_SUCCESS;
}

static int _tai_list_size(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ const tai_attribute_value_t* const value) {

    switch( metadata->attrvaluetype ) {
    case TAI_ATTR_VALUE_TYPE_BOOLDATA:
    case TAI_ATTR_VALUE_TYPE_CHARDATA:
    case TAI_ATTR_VALUE_TYPE_U8:
    case TAI_ATTR_VALUE_TYPE_S8:
    case TAI_ATTR_VALUE_TYPE_U16:
    case TAI_ATTR_VALUE_TYPE_S16:
    case TAI_ATTR_VALUE_TYPE_U32:
    case TAI_ATTR_VALUE_TYPE_S32:
    case TAI_ATTR_VALUE_TYPE_U64:
    case TAI_ATTR_VALUE_TYPE_S64:
    case TAI_ATTR_VALUE_TYPE_FLT:
    case TAI_ATTR_VALUE_TYPE_PTR:
    case TAI_ATTR_VALUE_TYPE_OID:
    case TAI_ATTR_VALUE_TYPE_U32RANGE:
    case TAI_ATTR_VALUE_TYPE_S32RANGE:
    case TAI_ATTR_VALUE_TYPE_NOTIFICATION:
        return 0;
    case TAI_ATTR_VALUE_TYPE_OBJLIST:
        return value->objlist.count;
    case TAI_ATTR_VALUE_TYPE_CHARLIST:
        return value->charlist.count;
    case TAI_ATTR_VALUE_TYPE_U8LIST:
        return value->u8list.count;
    case TAI_ATTR_VALUE_TYPE_S8LIST:
        return value->s8list.count;
    case TAI_ATTR_VALUE_TYPE_U16LIST:
        return value->u16list.count;
    case TAI_ATTR_VALUE_TYPE_S16LIST:
        return value->s16list.count;
    case TAI_ATTR_VALUE_TYPE_U32LIST:
        return value->u32list.count;
    case TAI_ATTR_VALUE_TYPE_S32LIST:
        return value->s32list.count;
    case TAI_ATTR_VALUE_TYPE_FLOATLIST:
        return value->floatlist.count;
    case TAI_ATTR_VALUE_TYPE_OBJMAPLIST:
        return value->objmaplist.count;
    case TAI_ATTR_VALUE_TYPE_ATTRLIST:
        return value->attrlist.count;
    default:
        TAI_META_LOG_ERROR("unsupported value type: %d", metadata->attrvaluetype);
        return -1;
    }
}


static tai_status_t _tai_metadata_alloc_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ tai_attribute_value_t* const value,
        _In_ const tai_alloc_info_t* const info) {
    if ( metadata == NULL || value == NULL ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }

    int size = DEFAULT_LIST_SIZE, i, j;
    if ( info != NULL ) {
        size = info->list_size;
        if ( info->reference != NULL ) {
            size = _tai_list_size(metadata, &info->reference->value);
        }
    }
    if ( size == 0 ) {
        size = DEFAULT_LIST_SIZE;
    }
    switch( metadata->attrvaluetype ) {
    case TAI_ATTR_VALUE_TYPE_BOOLDATA:
    case TAI_ATTR_VALUE_TYPE_CHARDATA:
    case TAI_ATTR_VALUE_TYPE_U8:
    case TAI_ATTR_VALUE_TYPE_S8:
    case TAI_ATTR_VALUE_TYPE_U16:
    case TAI_ATTR_VALUE_TYPE_S16:
    case TAI_ATTR_VALUE_TYPE_U32:
    case TAI_ATTR_VALUE_TYPE_S32:
    case TAI_ATTR_VALUE_TYPE_U64:
    case TAI_ATTR_VALUE_TYPE_S64:
    case TAI_ATTR_VALUE_TYPE_FLT:
    case TAI_ATTR_VALUE_TYPE_PTR:
    case TAI_ATTR_VALUE_TYPE_OID:
    case TAI_ATTR_VALUE_TYPE_U32RANGE:
    case TAI_ATTR_VALUE_TYPE_S32RANGE:
    case TAI_ATTR_VALUE_TYPE_NOTIFICATION:
        return TAI_STATUS_SUCCESS;
    case TAI_ATTR_VALUE_TYPE_OBJLIST:
        _TAI_META_ALLOC_LIST(objlist, tai_object_id_t);
        break;
    case TAI_ATTR_VALUE_TYPE_CHARLIST:
        _TAI_META_ALLOC_LIST(charlist, char);
        break;
    case TAI_ATTR_VALUE_TYPE_U8LIST:
        _TAI_META_ALLOC_LIST(u8list, uint8_t);
        break;
    case TAI_ATTR_VALUE_TYPE_S8LIST:
        _TAI_META_ALLOC_LIST(s8list, int8_t);
        break;
    case TAI_ATTR_VALUE_TYPE_U16LIST:
        _TAI_META_ALLOC_LIST(u16list, uint16_t);
        break;
    case TAI_ATTR_VALUE_TYPE_S16LIST:
        _TAI_META_ALLOC_LIST(s16list, int16_t);
        break;
    case TAI_ATTR_VALUE_TYPE_U32LIST:
        _TAI_META_ALLOC_LIST(u32list, uint32_t);
        break;
    case TAI_ATTR_VALUE_TYPE_S32LIST:
        _TAI_META_ALLOC_LIST(s32list, int32_t);
        break;
    case TAI_ATTR_VALUE_TYPE_FLOATLIST:
        _TAI_META_ALLOC_LIST(floatlist, float);
        break;
    case TAI_ATTR_VALUE_TYPE_OBJMAPLIST:
        _TAI_META_ALLOC_LIST(objmaplist, tai_object_map_list_t);
        value->attrlist._alloced = size;
        for ( i = 0; i < size; i++ ) {
            int ssize = size;
            if ( info != NULL && info->reference != NULL ) {
                int s = info->reference->value.objmaplist.list[i].value.count;
                ssize = (s > 0) ? s : DEFAULT_LIST_SIZE;
            }
            value->objmaplist.list[i].value.count = ssize;
            value->objmaplist.list[i].value.list = calloc(ssize, sizeof(tai_object_map_t));
            if ( value->objmaplist.list[i].value.list == NULL ) {
                for ( j = 0; j < i; j++ ) {
                    free(value->objmaplist.list[j].value.list);
                    value->objmaplist.list[j].value.list = NULL;
                    value->objmaplist.list[j].value.count = 0;
                }
                _TAI_META_FREE_LIST(objmaplist);
                value->attrlist._alloced = 0;
                return TAI_STATUS_NO_MEMORY;
            }
        }
        break;
    case TAI_ATTR_VALUE_TYPE_ATTRLIST:
        {
            _TAI_META_ALLOC_LIST(attrlist, tai_attribute_value_t);
            memset(value->attrlist.list, 0, size * sizeof(tai_attribute_value_t));\
            value->attrlist._alloced = size;

            tai_attr_metadata_t nested_meta = *metadata;
            nested_meta.attrvaluetype = nested_meta.attrlistvaluetype;
            nested_meta.attrlistvaluetype = TAI_ATTR_VALUE_TYPE_UNSPECIFIED;

           for ( i = 0; i < size; i++ ) {
                int ssize = size;
                if ( info != NULL && info->reference != NULL ) {
                    int s = _tai_list_size(&nested_meta, &info->reference->value.attrlist.list[i]);
                    ssize = (s > 0) ? s : DEFAULT_LIST_SIZE;
                }
                tai_alloc_info_t nested_info = { .list_size = ssize };
                if ( _tai_metadata_alloc_attr_value(&nested_meta, &(value->attrlist.list[i]), &nested_info) < 0 ) {
                    for ( j = 0; j < i; j++ ) {
                        _tai_metadata_free_attr_value(&nested_meta, &value->attrlist.list[j], &nested_info);
                    }
                    _TAI_META_FREE_LIST(attrlist);
                    value->attrlist._alloced = 0;
                    return TAI_STATUS_NO_MEMORY;
                }
            }
        }
        break;
    default:
        TAI_META_LOG_ERROR("unsupported value type: %d", metadata->attrvaluetype);
        return TAI_STATUS_NOT_SUPPORTED;
    }
    return TAI_STATUS_SUCCESS;
}

tai_status_t tai_metadata_alloc_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ tai_attribute_t* const attr,
        _In_ const tai_alloc_info_t* const info) {
    if ( attr == NULL ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    return _tai_metadata_alloc_attr_value(metadata, &attr->value, info);
}

tai_status_t tai_metadata_free_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ tai_attribute_t* const attr,
        _In_ const tai_alloc_info_t* const info) {
    if ( attr == NULL ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    return _tai_metadata_free_attr_value(metadata, &attr->value, info);
}

tai_status_t tai_metadata_clear_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ tai_attribute_t* const attr) {
    if ( attr == NULL ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    return _tai_metadata_clear_attr_value(metadata, &attr->value);
}

static tai_status_t _tai_metadata_deepcopy_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ const tai_attribute_value_t* const in,
        _Out_ tai_attribute_value_t* const out) {
    if ( metadata == NULL || in == NULL || out == NULL ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    tai_attr_metadata_t m = *metadata;
    m.attrvaluetype = m.attrlistvaluetype;
    m.attrlistvaluetype = TAI_ATTR_VALUE_TYPE_UNSPECIFIED;

    int i;
    tai_status_t ret;
    switch( metadata->attrvaluetype ) {
    case TAI_ATTR_VALUE_TYPE_BOOLDATA:
    case TAI_ATTR_VALUE_TYPE_CHARDATA:
    case TAI_ATTR_VALUE_TYPE_U8:
    case TAI_ATTR_VALUE_TYPE_S8:
    case TAI_ATTR_VALUE_TYPE_U16:
    case TAI_ATTR_VALUE_TYPE_S16:
    case TAI_ATTR_VALUE_TYPE_U32:
    case TAI_ATTR_VALUE_TYPE_S32:
    case TAI_ATTR_VALUE_TYPE_U64:
    case TAI_ATTR_VALUE_TYPE_S64:
    case TAI_ATTR_VALUE_TYPE_FLT:
    case TAI_ATTR_VALUE_TYPE_PTR:
    case TAI_ATTR_VALUE_TYPE_OID:
    case TAI_ATTR_VALUE_TYPE_U32RANGE:
    case TAI_ATTR_VALUE_TYPE_S32RANGE:
    case TAI_ATTR_VALUE_TYPE_NOTIFICATION:
        memcpy(out, in, sizeof(tai_attribute_value_t));
        break;
    case TAI_ATTR_VALUE_TYPE_OBJLIST:
        _TAI_META_COPY_LIST(objlist, tai_object_id_t);
        break;
    case TAI_ATTR_VALUE_TYPE_CHARLIST:
        _TAI_META_COPY_LIST(charlist, char);
        break;
    case TAI_ATTR_VALUE_TYPE_U8LIST:
        _TAI_META_COPY_LIST(u8list, uint8_t);
        break;
    case TAI_ATTR_VALUE_TYPE_S8LIST:
        _TAI_META_COPY_LIST(s8list, int8_t);
        break;
    case TAI_ATTR_VALUE_TYPE_U16LIST:
        _TAI_META_COPY_LIST(u16list, uint16_t);
        break;
    case TAI_ATTR_VALUE_TYPE_S16LIST:
        _TAI_META_COPY_LIST(s16list, int16_t);
        break;
    case TAI_ATTR_VALUE_TYPE_U32LIST:
        _TAI_META_COPY_LIST(u32list, uint32_t);
        break;
    case TAI_ATTR_VALUE_TYPE_S32LIST:
        _TAI_META_COPY_LIST(s32list, int32_t);
        break;
    case TAI_ATTR_VALUE_TYPE_FLOATLIST:
        _TAI_META_COPY_LIST(floatlist, float);
        break;
    case TAI_ATTR_VALUE_TYPE_OBJMAPLIST:
        _TAI_META_COPY_LIST(objmaplist, tai_object_map_list_t);
        for ( i = 0; i < in->objmaplist.count ; i++ ) {
            if ( out->objmaplist.list[i].value.count < in->objmaplist.list[i].value.count ) {
                out->objmaplist.list[i].value.count = in->objmaplist.list[i].value.count;
                return TAI_STATUS_BUFFER_OVERFLOW;
            }
            out->objmaplist.list[i].value.count = in->objmaplist.list[i].value.count;
            memcpy(out->objmaplist.list[i].value.list, in->objmaplist.list[i].value.list, sizeof(tai_object_map_t) * in->objmaplist.list[i].value.count);
        }
        break;
    case TAI_ATTR_VALUE_TYPE_ATTRLIST:
        if( out->attrlist.count < in->attrlist.count ) {
            return TAI_STATUS_BUFFER_OVERFLOW;
        }
        out->attrlist.count = in->attrlist.count;
        for ( i = 0; i < in->attrlist.count; i++ ) {
            ret = _tai_metadata_deepcopy_attr_value(&m, &in->attrlist.list[i], &out->attrlist.list[i]);
            if ( ret != TAI_STATUS_SUCCESS ) {
                return ret;
            }
        }
        break;
    default:
        TAI_META_LOG_ERROR("unsupported value type: %d", metadata->attrvaluetype);
        return TAI_STATUS_NOT_SUPPORTED;
    }
    return TAI_STATUS_SUCCESS;
}

tai_status_t tai_metadata_deepcopy_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ const tai_attribute_t* const in,
        _Out_ tai_attribute_t* const out) {
    if ( metadata == NULL || in == NULL || out == NULL ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    out->id = in->id;
    return _tai_metadata_deepcopy_attr_value(metadata, &in->value, &out->value);
}

tai_status_t tai_metadata_deepequal_attr_value(
        _In_ const tai_attr_metadata_t* const metadata,
        _In_ const tai_attribute_t* const lhs,
        _In_ const tai_attribute_t* const rhs,
        _Out_ bool* result) {
    if ( metadata == NULL || result == NULL ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    if ( (lhs == NULL && rhs != NULL) || (lhs != NULL && rhs == NULL) ) {
        *result = false;
        return TAI_STATUS_SUCCESS;
    } else if ( lhs == NULL && rhs == NULL ) {
        *result = true;
        return TAI_STATUS_SUCCESS;
    }
    if ( lhs->id != rhs->id ) {
        *result = false;
        return TAI_STATUS_SUCCESS;
    }
    switch( metadata->attrvaluetype ) {
    case TAI_ATTR_VALUE_TYPE_BOOLDATA:
        _TAI_META_CMP(result, booldata)
    case TAI_ATTR_VALUE_TYPE_CHARDATA:
        *result = !memcmp(lhs->value.chardata, rhs->value.chardata, 32);
        break;
    case TAI_ATTR_VALUE_TYPE_U8:
        _TAI_META_CMP(result, u8)
    case TAI_ATTR_VALUE_TYPE_S8:
        _TAI_META_CMP(result, s8)
    case TAI_ATTR_VALUE_TYPE_U16:
        _TAI_META_CMP(result, u16)
    case TAI_ATTR_VALUE_TYPE_S16:
        _TAI_META_CMP(result, s16)
    case TAI_ATTR_VALUE_TYPE_U32:
        _TAI_META_CMP(result, u32)
    case TAI_ATTR_VALUE_TYPE_S32:
        _TAI_META_CMP(result, s32)
    case TAI_ATTR_VALUE_TYPE_U64:
        _TAI_META_CMP(result, u64)
    case TAI_ATTR_VALUE_TYPE_S64:
        _TAI_META_CMP(result, s64)
    case TAI_ATTR_VALUE_TYPE_FLT:
        _TAI_META_CMP(result, flt)
    case TAI_ATTR_VALUE_TYPE_PTR:
        _TAI_META_CMP(result, ptr)
    case TAI_ATTR_VALUE_TYPE_OID:
        _TAI_META_CMP(result, oid)
    case TAI_ATTR_VALUE_TYPE_U32RANGE:
        *result = !memcmp(&lhs->value, &rhs->value, sizeof(tai_u32_range_t));
        break;
    case TAI_ATTR_VALUE_TYPE_S32RANGE:
        *result = !memcmp(&lhs->value, &rhs->value, sizeof(tai_s32_range_t));
        break;
    case TAI_ATTR_VALUE_TYPE_NOTIFICATION:
        *result = !memcmp(&lhs->value, &rhs->value, sizeof(tai_notification_handler_t));
        break;
    case TAI_ATTR_VALUE_TYPE_OBJLIST:
        _TAI_META_CMP_LIST(result, objlist)
    case TAI_ATTR_VALUE_TYPE_CHARLIST:
        _TAI_META_CMP_LIST(result, charlist)
    case TAI_ATTR_VALUE_TYPE_U8LIST:
        _TAI_META_CMP_LIST(result, u8list)
    case TAI_ATTR_VALUE_TYPE_S8LIST:
        _TAI_META_CMP_LIST(result, s8list)
    case TAI_ATTR_VALUE_TYPE_U16LIST:
        _TAI_META_CMP_LIST(result, u16list)
    case TAI_ATTR_VALUE_TYPE_S16LIST:
        _TAI_META_CMP_LIST(result, s16list)
    case TAI_ATTR_VALUE_TYPE_U32LIST:
        _TAI_META_CMP_LIST(result, u32list)
    case TAI_ATTR_VALUE_TYPE_S32LIST:
        _TAI_META_CMP_LIST(result, s32list)
    case TAI_ATTR_VALUE_TYPE_FLOATLIST:
        _TAI_META_CMP_LIST(result, floatlist)
    default:
        TAI_META_LOG_ERROR("unsupported value type: %d", metadata->attrvaluetype);
        return TAI_STATUS_NOT_SUPPORTED;
    }
    return TAI_STATUS_SUCCESS;
}
