/**
 * @file    taimetadatautils.c
 *
 * @brief   This module implements TAI Metadata Utilities
 *
 * @copyright Copyright (c) 2014 Microsoft Open Technologies, Inc.
 * @copyright Copyright (c) 2018 Nippon Telegraph and Telephone Corporation
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
    attr->value.valuename.count = size;\
    attr->value.valuename.list = calloc(size, sizeof(type));\
    if ( attr->value.valuename.list == NULL ) {\
        return TAI_STATUS_NO_MEMORY;\
    }\

#define _TAI_META_FREE_LIST(valuename)\
    attr->value.valuename.count = 0;\
    free(attr->value.valuename.list);

#define _TAI_META_COPY_LIST(valuename, type)\
    if( out->value.valuename.count < in->value.valuename.count ) {\
        return TAI_STATUS_BUFFER_OVERFLOW;\
    }\
    out->value.valuename.count = in->value.valuename.count;\
    memcpy(out->value.valuename.list, in->value.valuename.list, sizeof(type) * in->value.valuename.count);

tai_status_t tai_metadata_alloc_attr_value(
        _In_ const tai_attr_metadata_t *metadata,
        _In_ tai_attribute_t* const attr,
        _In_ const tai_alloc_info_t* const info) {
    if ( metadata == NULL || attr == NULL ) {
        return TAI_STATUS_FAILURE;
    }
    int size = DEFAULT_LIST_SIZE, i, j;
    if ( info != NULL ) {
        size = info->list_size;
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
        _TAI_META_ALLOC_LIST(floatlist, int32_t);
        break;
    case TAI_ATTR_VALUE_TYPE_OBJMAPLIST:
        _TAI_META_ALLOC_LIST(objmaplist, tai_object_map_list_t);
        for ( i = 0; i < size; i++ ) {
            attr->value.objmaplist.list[i].value.count = size;
            attr->value.objmaplist.list[i].value.list = calloc(size, sizeof(tai_object_map_t));
            if ( attr->value.objmaplist.list[i].value.list == NULL ) {
                for ( j = 0; j < i; j++ ) {
                    free(attr->value.objmaplist.list[i].value.list);
                }
                _TAI_META_FREE_LIST(objmaplist);
                return TAI_STATUS_NO_MEMORY;
            }
        }
        break;
    default:
        TAI_META_LOG_ERROR("unsupported value type: %d", metadata->attrvaluetype);
        return TAI_STATUS_INVALID_PARAMETER;
    }
    return TAI_STATUS_SUCCESS;
}

tai_status_t tai_metadata_free_attr_value(
        _In_ const tai_attr_metadata_t *metadata,
        _In_ tai_attribute_t* const attr,
        _In_ const tai_alloc_info_t* const info) {
    if ( metadata == NULL || attr == NULL ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    int size = DEFAULT_LIST_SIZE, i;
    if ( info != NULL ) {
        size = info->list_size;
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
        for ( i = 0; i < size; i++ ) {
            attr->value.objmaplist.list[i].value.count = 0;
            free(attr->value.objmaplist.list[i].value.list);
        }
        _TAI_META_FREE_LIST(objmaplist);
        break;
    default:
        TAI_META_LOG_ERROR("unsupported value type: %d", metadata->attrvaluetype);
        return TAI_STATUS_INVALID_PARAMETER;
    }
    return TAI_STATUS_SUCCESS;
}

tai_status_t tai_metadata_deepcopy_attr_value(
        _In_ const tai_attr_metadata_t *metadata,
        _In_ const tai_attribute_t* const in,
        _Out_ tai_attribute_t* const out) {
    if ( metadata == NULL || in == NULL || out == NULL ) {
        return TAI_STATUS_INVALID_PARAMETER;
    }
    int i;
    out->id = in->id;
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
        memcpy(&out->value, &in->value, sizeof(tai_attribute_value_t));
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
        _TAI_META_COPY_LIST(floatlist, int32_t);
        break;
    case TAI_ATTR_VALUE_TYPE_OBJMAPLIST:
        _TAI_META_COPY_LIST(objmaplist, tai_object_map_list_t);
        for ( i = 0; i < in->value.objmaplist.count ; i++ ) {
            if ( out->value.objmaplist.list[i].value.count < in->value.objmaplist.list[i].value.count ) {
                out->value.objmaplist.list[i].value.count = in->value.objmaplist.list[i].value.count;
                return TAI_STATUS_BUFFER_OVERFLOW;
            }
            out->value.objmaplist.list[i].value.count = in->value.objmaplist.list[i].value.count;
            memcpy(out->value.objmaplist.list[i].value.list, in->value.objmaplist.list[i].value.list, sizeof(tai_object_map_t) * in->value.objmaplist.list[i].value.count);
        }
        break;
    default:
        TAI_META_LOG_ERROR("unsupported value type: %d", metadata->attrvaluetype);
        return -1;
    }
    return TAI_STATUS_SUCCESS;
}
