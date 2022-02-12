#include "tai.h"
#include "taimetadata.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int testSerializeModuleOperStatus() {
    int ret;
    char buf[128] = {0};
    ret = tai_serialize_module_oper_status(buf, 128, TAI_MODULE_OPER_STATUS_READY, NULL);
    if ( ret < 0 ) {
        return ret;
    }
    ret = strcmp(buf, "TAI_MODULE_OPER_STATUS_READY");
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testDeserializeModuleOperStatus() {
    int ret, value;
    ret = tai_deserialize_module_oper_status("TAI_MODULE_OPER_STATUS_INITIALIZE", &value, NULL);
    if ( ret < 0 ) {
        return ret;
    }
    if ( TAI_MODULE_OPER_STATUS_INITIALIZE != value ) {
        return -1;
    }
    return 0;
}

int testSerializeAttributeEnum() {
    int ret;
    char buf[128] = {0};
    tai_attribute_t attr = {0};
    tai_serialize_option_t option = {
        .valueonly = true,
    };
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_MODULE, TAI_MODULE_ATTR_OPER_STATUS);
    attr.id = TAI_MODULE_ATTR_OPER_STATUS;
    attr.value.s32 = TAI_MODULE_OPER_STATUS_READY;
    ret = tai_serialize_attribute(buf, 128, meta, &attr, &option);
    if ( ret <  0 ) {
        return -1;
    }
    ret = strcmp(buf, "TAI_MODULE_OPER_STATUS_READY");
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testSerializeAttributeFloat() {
    int ret;
    char buf[128] = {0};
    tai_attribute_t attr = {0};
    tai_serialize_option_t option = {
        .valueonly = true,
    };
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_MODULE, TAI_MODULE_ATTR_TEMP);
    attr.id = TAI_MODULE_ATTR_TEMP;
    attr.value.flt = 1.10;
    ret = tai_serialize_attribute(buf, 128, meta, &attr, &option);
    if ( ret < 0 ) {
        return -1;
    }
    ret = strcmp(buf, "1.100000");
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testSerializeAttributeEnumList() {
    int ret;
    char buf[128] = {0};
    tai_attribute_t attr = {0};
    tai_serialize_option_t option = {
        .valueonly = false,
        .human = true,
    };
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_NETWORKIF, TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS);
    int32_t list[] = {
        TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_LOSS,
        TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_OUT
    };
    attr.id = TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS;
    attr.value.s32list.count = 2;
    attr.value.s32list.list = (int32_t*)&list;
    ret = tai_serialize_attribute(buf, 128, meta, &attr, &option);
    if ( ret < 0 ) {
        return -1;
    }
    ret = strcmp(buf, "tx-align-status | loss|out");
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testGetAttrMetadataByAttrIdName() {
    const tai_attr_metadata_t* meta;
    meta = tai_metadata_get_attr_metadata_by_attr_id_name("TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ");
    if ( meta == NULL ) {
        return -1;
    }
    if ( meta->attrid != TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ ) {
        return -1;
    }
 
    return 0;
}

int testDeserializeNetworkInterfaceAttr() {
    int32_t value;
    int ret;
    tai_serialize_option_t option = {
        .human = true,
    };
    ret = tai_deserialize_network_interface_attr("tx-laser-freq", &value, &option);
    if ( ret < 0 ) {
        return -1;
    }
    if ( value != TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ ) {
        return -1;
    }
    return 0;
}

int testDeepcopyAttrValue() {
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_NETWORKIF, TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS);
    tai_attribute_t src = {0}, dst = {0};
    tai_status_t status;
    status = tai_metadata_alloc_attr_value(meta, &src, NULL);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("failed to alloc attr value: %d\n", status);
        return -1;
    }
    status = tai_metadata_alloc_attr_value(meta, &dst, NULL);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("failed to alloc attr value: %d\n", status);
        return -1;
    }
    src.value.s32list.count = 2;
    src.value.s32list.list[0] = TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_TIMING;
    src.value.s32list.list[1] = TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_OUT;

    status = tai_metadata_deepcopy_attr_value(meta, &src, &dst);
    if ( status != TAI_STATUS_SUCCESS ) {
        return -1;
    }

    if ( dst.value.s32list.count != 2 ) {
        return -1;
    }
    if ( dst.value.s32list.list[0] != TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_TIMING ) {
        return -1;
    }
    if ( dst.value.s32list.list[1] != TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_OUT ) {
        return -1;
    }

    status = tai_metadata_free_attr_value(meta, &src, NULL);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("failed to free attr value: %d\n", status);
        return -1;
    }
    status = tai_metadata_free_attr_value(meta, &dst, NULL);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("failed to free attr value: %d\n", status);
        return -1;
    }
    if ( dst.value.s32list.count != 0 ) {
        return -1;
    }
    if ( src.value.s32list.count != 0 ) {
        return -1;
    }
    return 0;
}

int testSerializeUnsignedRange() {
    char buf[128] = {0};
    tai_attribute_value_t value = {0};
    tai_attr_metadata_t meta = { .attrvaluetype = TAI_ATTR_VALUE_TYPE_U32RANGE };
    tai_serialize_option_t option = {
        .valueonly = true,
        .human = true,
    };
    int ret = tai_deserialize_attribute_value("100,1000", &meta, &value, NULL);
    tai_attribute_t attr = { .value = value };
    tai_serialize_attribute(buf, 128, &meta, &attr, &option);
    ret = strcmp(buf, "100,1000");
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testSerializeSignedRange() {
    char buf[128] = {0};
    tai_attribute_value_t value = {0};
    tai_attr_metadata_t meta = { .attrvaluetype = TAI_ATTR_VALUE_TYPE_S32RANGE };
    tai_serialize_option_t option = {
        .valueonly = true,
        .human = true,
    };
    int ret = tai_deserialize_attribute_value("-100,-1000", &meta, &value, NULL);
    tai_attribute_t attr = { .value = value };
    tai_serialize_attribute(buf, 128, &meta, &attr, &option);
    ret = strcmp(buf, "-100,-1000");
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testSerializeValueAttrList() {
    char buf[128] = {0};
    const tai_attr_metadata_t* meta;
    tai_attribute_t attr = {0};
    int ret;
    int32_t value[] = {
        TAI_HOST_INTERFACE_LANE_FAULT_LOSS_OF_LOCK,
        TAI_HOST_INTERFACE_LANE_FAULT_TX_FIFO_ERR,
    };
    tai_serialize_option_t option = {
        .human = true,
    };
    tai_attribute_value_t lane_fault;
    lane_fault.s32list.count = 2;
    lane_fault.s32list.list = value;
    tai_attribute_value_t list[] = {
        lane_fault,
        lane_fault,
    };
    meta = tai_metadata_get_attr_metadata_by_attr_id_name("TAI_HOST_INTERFACE_ATTR_LANE_FAULT");
    if ( meta == NULL ) {
        return -1;
    }
    attr.id = TAI_HOST_INTERFACE_ATTR_LANE_FAULT;
    attr.value.attrlist.count = 2;
    attr.value.attrlist.list = (tai_attribute_value_t*)&list;
    ret = tai_serialize_attribute(buf, 128, meta, &attr, &option);
    if ( ret < 0 ) {
        return -1;
    }
    ret = strcmp(buf, "lane-fault | loss-of-lock|tx-fifo-err, loss-of-lock|tx-fifo-err");
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testDeserializeU8list() {
    uint8_t list[10] = {0};
    tai_u8_list_t value;
    tai_serialize_option_t option = {
        .json = false,
    };
    value.count = 10;
    value.list = list;
    int ret = tai_deserialize_u8list("1,2,3,4", &value, &option);
    if ( ret != 0 ) {
        return -1;
    }
    if ( value.count != 4 || list[0] != 1 || list[1] != 2 || list[2] != 3 || list[3] != 4) {
        return -1;
    }
    ret = tai_deserialize_u8list("", &value, &option);
    if ( ret != 0 ) {
        return -1;
    }
    if ( value.count != 0 ) {
        return -1;
    }
    return 0;
}

int testDeserializeU32list() {
    uint32_t list[10] = {0};
    tai_u32_list_t value;
    tai_serialize_option_t option = {
        .json = false,
    };
    value.count = 10;
    value.list = list;
    int ret = tai_deserialize_u32list("4294967295,4294967295,4294967295,4294967295", &value, &option);
    if ( ret != 0 ) {
        return -1;
    }
    if ( value.count != 4 || list[0] != 0xffffffff || list[1] != 0xffffffff || list[2] != 0xffffffff || list[3] != 0xffffffff) {
        return -1;
    }
    ret = tai_deserialize_u32list("-1,-1,-1,-1", &value, &option);
    if ( ret >= 0 ) {
        return -1;
    }
    return 0;
}

int testDeserializeObjlist() {
    tai_object_id_t list[10] = {0};
    {
        tai_object_list_t value;
        tai_serialize_option_t option = {
            .json = false,
        };
        value.count = 10;
        value.list = list;
        int ret = tai_deserialize_objlist("oid:0x1,oid:0x2,oid:0x3,oid:0x4", &value, &option);
        if ( ret != 0 ) {
            return -1;
        }
        if (value.count != 4 || list[0] != 0x1 || list[1] != 0x2 || list[2] != 0x3 || list[3] != 0x4) {
            return -1;
        }
        ret = tai_deserialize_objlist("-1,-1,-1,-1", &value, &option);
        if ( ret >= 0 ) {
            return -1;
        }
    }
    {
        tai_attribute_value_t value;
        value.objlist.count = 10;
        value.objlist.list = list;
        tai_attr_metadata_t meta = { .attrvaluetype = TAI_ATTR_VALUE_TYPE_OBJLIST };
        int ret = tai_deserialize_attribute_value("oid:0x10,oid:0x20,oid:0x30", &meta, &value, NULL);
        if ( ret != 0 ) {
            return -1;
        }
        if (value.objlist.count != 3 || list[0] != 0x10 || list[1] != 0x20 || list[2] != 0x30) {
            return -1;
        }
    }
    return 0;
}



int testDeserializeU8listLengthCheck() {
    tai_u8_list_t value;
    tai_serialize_option_t option = {
        .json = false,
    };
    value.count = 0;
    value.list = NULL;
    int ret = tai_deserialize_u8list("1,2,3,4", &value, &option);
    if ( ret == 0 ) {
        return -1;
    }
    if ( value.count != 4 ) {
        return -1;
    }
    return 0;
}

int testDeserializeU8listInvalidValue() {
    tai_u8_list_t value;
    tai_serialize_option_t option = {
        .json = false,
    };
    value.count = 0;
    value.list = NULL;
    int ret = tai_deserialize_u8list("1,2,a,4", &value, &option);
    if ( ret == 0 ) {
        return -1;
    }
    return 0;
}

int testDeserializeFloatlist() {
    float list[10] = {0};
    tai_float_list_t value;
    tai_serialize_option_t option = {
        .json = false,
    };
    value.count = 10;
    value.list = list;
    int ret = tai_deserialize_floatlist("1.1,2.1234,3.1,4.5634", &value, &option);
    if ( ret != 0 ) {
        return -1;
    }
    if ( value.count != 4 || list[0] < 1 || list[1] < 2.1 || list[2] > 3.2 || list[3] > 4.6 ) {
        return -1;
    }
    return 0;
}

int testSerializeListJSON() {
    char buf[128] = {0};
    const tai_attr_metadata_t* meta;
    tai_attribute_t attr = {0};
    int ret;
    int32_t value[] = {
        TAI_HOST_INTERFACE_LANE_FAULT_LOSS_OF_LOCK,
        TAI_HOST_INTERFACE_LANE_FAULT_TX_FIFO_ERR,
    };
    tai_serialize_option_t option = {
        .human = true,
        .json = true,
        .valueonly = true,
    };
    tai_attribute_value_t lane_fault;
    lane_fault.s32list.count = 2;
    lane_fault.s32list.list = value;
    tai_attribute_value_t list[] = {
        lane_fault,
        lane_fault,
    };
    meta = tai_metadata_get_attr_metadata_by_attr_id_name("TAI_HOST_INTERFACE_ATTR_LANE_FAULT");
    if ( meta == NULL ) {
        return -1;
    }
    attr.id = TAI_HOST_INTERFACE_ATTR_LANE_FAULT;
    attr.value.attrlist.count = 2;
    attr.value.attrlist.list = (tai_attribute_value_t*)&list;
    ret = tai_serialize_attribute(buf, 128, meta, &attr, &option);
    if ( ret < 0 ) {
        return -1;
    }
    ret = strcmp(buf, "[[\"loss-of-lock\",\"tx-fifo-err\"], [\"loss-of-lock\",\"tx-fifo-err\"]]");
    if ( ret != 0 ) {
        printf("%s\n", buf);
        return -1;
    }
    return 0;
}

int testDeepcopyAttrlist() {
    tai_attr_metadata_t meta = {0};
    meta.attrvaluetype = TAI_ATTR_VALUE_TYPE_ATTRLIST;
    meta.attrlistvaluetype = TAI_ATTR_VALUE_TYPE_FLOATLIST;
    tai_attribute_t attr = {0}, dst = {0};
    tai_serialize_option_t option = {
        .human = true,
        .json = true,
        .valueonly = true,
    };
    char buf[10000] = {0};

    if ( tai_metadata_alloc_attr_value(&meta, &attr, NULL) < 0 ) {
        printf("failed to alloc\n");
        return -1;
    }

    if ( tai_metadata_alloc_attr_value(&meta, &dst, NULL) < 0 ) {
        printf("failed to alloc\n");
        return -1;
    }

    attr.value.attrlist.count = 4;
    attr.value.attrlist.list[0].floatlist.count = 4;
    attr.value.attrlist.list[1].floatlist.count = 4;
    attr.value.attrlist.list[2].floatlist.count = 4;
    attr.value.attrlist.list[3].floatlist.count = 4;

    attr.value.attrlist.list[0].floatlist.list[0] = 0.11;
    attr.value.attrlist.list[1].floatlist.list[0] = 0.11;
    attr.value.attrlist.list[2].floatlist.list[0] = 0.11;
    attr.value.attrlist.list[3].floatlist.list[0] = 0.11;

    if ( tai_serialize_attribute(buf, 10000, &meta, &attr, &option) < 0 ) {
        return -1;
    }
    printf("%s\n", buf);

    if ( tai_metadata_deepcopy_attr_value(&meta, &attr, &dst) < 0 ) {
        return -1;
    }

    if ( tai_serialize_attribute(buf, 10000, &meta, &dst, &option) < 0 ) {
        return -1;
    }
    printf("%s\n", buf);

    if ( tai_metadata_free_attr_value(&meta, &attr, NULL) < 0 ) {
        printf("failed to free\n");
        return -1;
    }

    if ( tai_metadata_free_attr_value(&meta, &dst, NULL) < 0 ) {
        printf("failed to free\n");
        return -1;
    }

    return 0;
}

int testSerializeObjectMapList() {
    char buf[128] = {0};
    tai_serialize_option_t option = {
        .human = true,
        .json = true,
        .valueonly = true,
    };
    tai_attr_metadata_t meta = {0};
    tai_attribute_t attr = {0};
    tai_object_map_t list[2];
    tai_object_id_t l[4];
    l[0] = 0x2000;
    l[1] = 0x3000;
    l[2] = 0x4000;
    l[3] = 0x5000;
    list[0].key = 0x1000;
    list[0].value.count = 4;
    list[0].value.list = l;
    list[1].key = 0x1100;
    list[1].value.count = 2;
    list[1].value.list = l;
    meta.attrvaluetype = TAI_ATTR_VALUE_TYPE_OBJMAPLIST;
    attr.value.objmaplist.count = 2;
    attr.value.objmaplist.list = list;
    int ret = tai_serialize_attribute(buf, 128, &meta, &attr, &option);
    if ( ret < 0 ) {
        return -1;
    }
    ret = strcmp(buf, "[{\"oid:0x1000\": [\"oid:0x2000\", \"oid:0x3000\", \"oid:0x4000\", \"oid:0x5000\"]}, {\"oid:0x1100\": [\"oid:0x2000\", \"oid:0x3000\"]}]");
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testDeserializeObjectMapList() {
    tai_object_id_t ids[10][10];
    tai_object_map_t lists[10];
    for ( int i = 0; i < 10; i++ ) {
        lists[i].value.count = 10;
        lists[i].value.list = ids[i];
    }
    tai_object_map_list_t value;
    value.count = 10;
    value.list = lists;
    tai_serialize_option_t option = {
        .json = true,
    };
    int ret = tai_deserialize_objmaplist("[{\"oid:0x1000\": [\"oid:0x2000\", \"oid:0x3000\", \"oid:0x4000\", \"oid:0x5000\"]}, {\"oid:0x1100\": [\"oid:0x2000\", \"oid:0x3000\"]}]", &value, &option);
    if ( ret != 0 ) {
        return -1;
    }
    if ( value.count != 2 ) {
        return -1;
    }
    if ( value.list[0].key != 0x1000 ) {
        return -1;
    }
    if ( value.list[0].value.count != 4 ) {
        return -1;
    }
    if ( value.list[0].value.list[0] != 0x2000 ) {
        return -1;
    }
    if ( value.list[0].value.list[1] != 0x3000 ) {
        return -1;
    }
    if ( value.list[0].value.list[2] != 0x4000 ) {
        return -1;
    }
    if ( value.list[0].value.list[3] != 0x5000 ) {
        return -1;
    }

    if ( value.list[1].key != 0x1100 ) {
        return -1;
    }
    if ( value.list[1].value.count != 2 ) {
        return -1;
    }
    if ( value.list[0].value.list[0] != 0x2000 ) {
        return -1;
    }
    if ( value.list[0].value.list[1] != 0x3000 ) {
        return -1;
    }
    return 0;
}

int testAllocObjectMapList() {
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_MODULE, TAI_MODULE_ATTR_TRIBUTARY_MAPPING);
    tai_attribute_t attr = {};
    int ret = tai_metadata_alloc_attr_value(meta, &attr, NULL);
    if ( ret != 0 ) {
        return -1;

    }
    tai_serialize_option_t option = {
        .json = true,
    };
    ret = tai_deserialize_objmaplist("[{\"oid:0x1000\": [\"oid:0x2000\", \"oid:0x3000\", \"oid:0x4000\", \"oid:0x5000\"]}, {\"oid:0x1100\": [\"oid:0x2000\", \"oid:0x3000\"]}]", &attr.value.objmaplist, &option);
    if ( ret != 0 ) {
        return -1;
    }
    ret = tai_metadata_free_attr_value(meta, &attr, NULL);
    if ( ret != 0 ) {
        return -1;
    }
    tai_alloc_info_t info = {.list_size = 1};
    ret = tai_metadata_alloc_attr_value(meta, &attr, &info);
    if ( ret != 0 ) {
        return -1;
    }
    if ( attr.value.objmaplist.count != 1 ) {
        return -1;
    }
    if ( attr.value.objmaplist._alloced != 1 ) {
        return -1;
    }
    ret = tai_deserialize_objmaplist("[{\"oid:0x1000\": [\"oid:0x2000\", \"oid:0x3000\", \"oid:0x4000\", \"oid:0x5000\"]}, {\"oid:0x1100\": [\"oid:0x2000\", \"oid:0x3000\"]}]", &attr.value.objmaplist, &option);
    if ( ret != TAI_STATUS_BUFFER_OVERFLOW ) {
        return -1;
    }
    if ( attr.value.objmaplist.count != 2 ) {
        return -1;
    }
    ret = tai_metadata_free_attr_value(meta, &attr, NULL);
    if ( ret != 0 ) {
        return -1;
    }
    ret = tai_metadata_alloc_attr_value(meta, &attr, NULL);
    if ( ret != 0 ) {
        return -1;
    }
    ret = tai_deserialize_objmaplist("[{\"oid:0x1000\": [\"oid:0x2000\", \"oid:0x3000\", \"oid:0x4000\", \"oid:0x5000\"]}, {\"oid:0x1100\": [\"oid:0x2000\", \"oid:0x3000\"]}]", &attr.value.objmaplist, &option);
    if ( ret != 0 ) {
        return -1;
    }

    tai_attribute_t attr2 = {};
    info.reference = &attr;
    ret = tai_metadata_alloc_attr_value(meta, &attr2, &info);
    if ( ret != 0 ) {
        return -1;
    }

    if ( attr2.value.objmaplist.count != 2 ) {
        return -1;
    }
    if ( attr2.value.objmaplist.list[0].value.count != 4 ) {
        return -1;
    }
    if ( attr2.value.objmaplist.list[1].value.count != 2 ) {
        return -1;
    }
    ret = tai_metadata_free_attr_value(meta, &attr, NULL);
    if ( ret != 0 ) {
        return -1;
    }
    ret = tai_metadata_free_attr_value(meta, &attr2, NULL);
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testDeepcopyObjectMapList() {
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_MODULE, TAI_MODULE_ATTR_TRIBUTARY_MAPPING);
    tai_attribute_t src = {}, dst = {};
    int ret = tai_metadata_alloc_attr_value(meta, &src, NULL);
    if ( ret != 0 ) {
        return -1;
    }
    tai_serialize_option_t option = {
        .json = true,
    };
    ret = tai_deserialize_objmaplist("[{\"oid:0x1000\": [\"oid:0x2000\", \"oid:0x3000\", \"oid:0x4000\", \"oid:0x5000\"]}, {\"oid:0x1100\": [\"oid:0x2000\", \"oid:0x3000\"]}]", &src.value.objmaplist, &option);
    if ( ret != 0 ) {
        return -1;
    }
    ret = tai_metadata_deepcopy_attr_value(meta, &src, &dst);
    if ( ret != TAI_STATUS_BUFFER_OVERFLOW ) {
        return -1;
    }
    ret = tai_metadata_alloc_attr_value(meta, &dst, NULL);
    if ( ret != 0 ) {
        return -1;
    }
    ret = tai_metadata_deepcopy_attr_value(meta, &src, &dst);
    if ( ret != 0 ) {
        return -1;
    }
    if ( src.value.objmaplist.count != dst.value.objmaplist.count ) {
        return -1;
    }
    for ( int i = 0; i < src.value.objmaplist.count; i++ ) {
        if ( src.value.objmaplist.list[i].key != dst.value.objmaplist.list[i].key ) {
            return -1;
        }
        if ( src.value.objmaplist.list[i].value.count != dst.value.objmaplist.list[i].value.count ) {
            return -1;
        }
        for ( int j = 0; j < src.value.objmaplist.list[i].value.count; j++ ) {
            if ( src.value.objmaplist.list[i].value.list[j] != dst.value.objmaplist.list[i].value.list[j] ) {
                return -1;
            }
        }
    }
    return 0;
}

int testDeepequalObjectMapList() {
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_MODULE, TAI_MODULE_ATTR_TRIBUTARY_MAPPING);
    tai_attribute_t src = {}, dst = {};
    int ret = tai_metadata_alloc_attr_value(meta, &src, NULL);
    if ( ret != 0 ) {
        return -1;
    }
    ret = tai_metadata_alloc_attr_value(meta, &dst, NULL);
    if ( ret != 0 ) {
        return -1;
    }
    tai_serialize_option_t option = {
        .json = true,
    };
    ret = tai_deserialize_objmaplist("[{\"oid:0x1000\": [\"oid:0x2000\", \"oid:0x3000\", \"oid:0x4000\", \"oid:0x5000\"]}, {\"oid:0x1100\": [\"oid:0x2000\", \"oid:0x3000\"]}]", &src.value.objmaplist, &option);
    if ( ret != 0 ) {
        return -1;
    }
    dst.value.objmaplist.count = 2;
    dst.value.objmaplist.list[0].key = 0x1000;
    dst.value.objmaplist.list[0].value.count = 4;
    dst.value.objmaplist.list[0].value.list[0] = 0x2000;
    dst.value.objmaplist.list[0].value.list[1] = 0x3000;
    dst.value.objmaplist.list[0].value.list[2] = 0x4000;
    dst.value.objmaplist.list[0].value.list[3] = 0x5000;

    dst.value.objmaplist.list[1].key = 0x1100;
    dst.value.objmaplist.list[1].value.count = 2;
    dst.value.objmaplist.list[1].value.list[0] = 0x2000;
    dst.value.objmaplist.list[1].value.list[1] = 0x3000;

    bool result;

    ret = tai_metadata_deepequal_attr_value(meta, &src, &dst, &result);
    if ( ret != 0 ) {
        return -1;
    }
    if ( result != true ) {
        return -1;
    }

    dst.value.objmaplist.list[1].value.list[1] = 0x3001;
    ret = tai_metadata_deepequal_attr_value(meta, &src, &dst, &result);
    if ( ret != 0 ) {
        return -1;
    }
    if ( result != false ) {
        return -1;
    }
    return 0;
}

int testDeserializeJSONU8List() {
    uint8_t list[10] = {0};
    tai_u8_list_t value;
    tai_serialize_option_t option = {
        .json = true,
    };
    value.count = 10;
    value.list = list;
    int ret = tai_deserialize_u8list("[1,2,3,4]", &value, &option);
    if ( ret != 0 ) {
        return -1;
    }
    if ( value.count != 4 || list[0] != 1 || list[1] != 2 || list[2] != 3 || list[3] != 4) {
        return -1;
    }
    return 0;
}

int testDeserializeJSONFloatList() {
    float list[10] = {0};
    tai_float_list_t value;
    value.count = 10;
    value.list = list;
    tai_serialize_option_t option = {
        .json = true,
    };
    int ret = tai_deserialize_floatlist("[1.1,2.1234,3.1,4.5634]", &value, &option);
    if ( ret != 0 ) {
        return -1;
    }
    if ( value.count != 4 || list[0] < 1 || list[1] < 2.1 || list[2] > 3.2 || list[3] > 4.6 ) {
        return -1;
    }
    return 0;
}

int testDeserializeJSONEnumList() {
    int32_t list[10] = {0};
    tai_s32_list_t value;
    value.count = 10;
    value.list = list;
    tai_serialize_option_t option = {
        .json = true,
        .human = true,
    };
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_NETWORKIF, TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS);
    int ret = tai_deserialize_enumlist("[ \"loss\", \"out\" ]", meta->enummetadata, &value, &option);
    if ( ret != 0 ) {
        return -1;
    }
    if ( value.count != 2 || list[0] != TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_LOSS || list[1] != TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_OUT ) {
        return -1;
    }
    return 0;
}

int testDeserializeJSONAttrList() {
    tai_attr_metadata_t meta = {0};
    meta.attrvaluetype = TAI_ATTR_VALUE_TYPE_ATTRLIST;
    meta.attrlistvaluetype = TAI_ATTR_VALUE_TYPE_FLOATLIST;
    tai_attribute_t attr = {0};
    tai_serialize_option_t option = {
        .human = true,
        .json = true,
    };

    if ( tai_metadata_alloc_attr_value(&meta, &attr, NULL) < 0 ) {
        printf("failed to alloc\n");
        return -1;
    }

    int ret = tai_deserialize_attrlist("[[0.11, 0.22], [0.33, 0.44, 0.55]]", &meta, &attr.value.attrlist, &option);
    if ( ret != 0 ) {
        return -1;
    }
    if ( attr.value.attrlist.count != 2 || attr.value.attrlist.list[0].floatlist.count != 2 || attr.value.attrlist.list[1].floatlist.count != 3 ) {
        return -1;
    }
    ret = tai_metadata_free_attr_value(&meta, &attr, NULL);
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testDeserializeJSONAttrList2() {
    tai_attribute_t attr = {0};
    tai_serialize_option_t option = {
        .human = true,
        .json = true,
    };
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_HOSTIF, TAI_HOST_INTERFACE_ATTR_LANE_FAULT);

    if ( tai_metadata_alloc_attr_value(meta, &attr, NULL) < 0 ) {
        printf("failed to alloc\n");
        return -1;
    }

    int ret = tai_deserialize_attrlist("[[\"loss-of-lock\", \"tx-fifo-err\"], [\"loss-of-lock\"], []]", meta, &attr.value.attrlist, &option);
    if ( ret != 0 ) {
        return -1;
    }
    if ( attr.value.attrlist.count != 3 || attr.value.attrlist.list[0].s32list.count != 2 || attr.value.attrlist.list[1].s32list.count != 1 || attr.value.attrlist.list[2].s32list.count != 0) {
        return -1;
    }
    if ( attr.value.attrlist.list[0].s32list.list[0] != TAI_HOST_INTERFACE_LANE_FAULT_LOSS_OF_LOCK ) {
        return -1;
    }
    if ( attr.value.attrlist.list[0].s32list.list[1] != TAI_HOST_INTERFACE_LANE_FAULT_TX_FIFO_ERR ) {
        return -1;
    }
    if ( attr.value.attrlist.list[1].s32list.list[0] != TAI_HOST_INTERFACE_LANE_FAULT_LOSS_OF_LOCK ) {
        return -1;
    }
    ret = tai_metadata_free_attr_value(meta, &attr, NULL);
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testDeserializeJSONBufferOverflow() {
    uint8_t list[10] = {0};
    tai_u8_list_t value;
    tai_serialize_option_t option = {
        .json = true,
    };
    value.count = 1;
    value.list = list;
    int ret = tai_deserialize_u8list("[1,2,3,4]", &value, &option);
    if ( ret != TAI_STATUS_BUFFER_OVERFLOW || value.count != 4 ) {
        return -1;
    }
    return 0;
}

int testDeserializeJSONBufferOverflow2() {
    int32_t list[10] = {0};
    tai_s32_list_t value;
    value.count = 1;
    value.list = list;
    tai_serialize_option_t option = {
        .json = true,
        .human = true,
    };
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_NETWORKIF, TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS);
    int ret = tai_deserialize_enumlist("[ \"loss\", \"out\" ]", meta->enummetadata, &value, &option);
    if ( ret != TAI_STATUS_BUFFER_OVERFLOW || value.count != 2 ) {
        return -1;
    }
    return 0;
}

int testSerializeStatus() {
    char buf[128] = {0};
    tai_serialize_option_t option = {
        .human = true,
        .json = true,
    };
    int ret = tai_serialize_status(buf, 128, TAI_STATUS_SUCCESS, &option);
    if ( ret < 0 ) {
        return -1;
    }
    ret = strcmp(buf, "\"success\"");
    if ( ret != 0 ) {
        return -1;
    }
    option.json = false;
    ret = tai_serialize_status(buf, 128, TAI_STATUS_SUCCESS, &option);
    if ( ret < 0 ) {
        return -1;
    }
    ret = strcmp(buf, "success");
    if ( ret != 0 ) {
        return -1;
    }
    option.human = false;
    ret = tai_serialize_status(buf, 128, TAI_STATUS_SUCCESS, &option);
    if ( ret < 0 ) {
        return -1;
    }
    ret = strcmp(buf, "TAI_STATUS_SUCCESS");
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testSerializeAttrValueType() {
    char buf[128] = {0};
    tai_serialize_option_t option = {
        .human = true,
    };
    int ret = tai_serialize_attr_value_type(buf, 128, TAI_ATTR_VALUE_TYPE_BOOLDATA, &option);
    if ( ret < 0 ) {
        return -1;
    }
    ret = strcmp(buf, "bool");
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testDeepequalAttrValue() {
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_NETWORKIF, TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS);
    tai_attribute_t src = {.id = TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS}, dst = {.id=TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS};
    tai_status_t status;
    status = tai_metadata_alloc_attr_value(meta, &src, NULL);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("failed to alloc attr value: %d\n", status);
        return -1;
    }
    status = tai_metadata_alloc_attr_value(meta, &dst, NULL);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("failed to alloc attr value: %d\n", status);
        return -1;
    }
    src.value.s32list.count = 2;
    src.value.s32list.list[0] = TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_TIMING;
    src.value.s32list.list[1] = TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_OUT;

    bool result;

    status = tai_metadata_deepequal_attr_value(meta, &src, &dst, &result);
    if ( status != TAI_STATUS_SUCCESS ) {
        return -1;
    }
    if ( result != false ) {
        return -1;
    }

    dst.value.s32list.count = 2;
    status = tai_metadata_deepequal_attr_value(meta, &src, &dst, &result);
    if ( status != TAI_STATUS_SUCCESS ) {
        return -1;
    }
    if ( result != false ) {
        return -1;
    }

    dst.value.s32list.list[0] = TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_TIMING;
    status = tai_metadata_deepequal_attr_value(meta, &src, &dst, &result);
    if ( status != TAI_STATUS_SUCCESS ) {
        return -1;
    }
    if ( result != false ) {
        return -1;
    }

    dst.value.s32list.list[1] = TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_TIMING;
    status = tai_metadata_deepequal_attr_value(meta, &src, &dst, &result);
    if ( status != TAI_STATUS_SUCCESS ) {
        return -1;
    }
    if ( result != false ) {
        return -1;
    }

    dst.value.s32list.list[1] = TAI_NETWORK_INTERFACE_TX_ALIGN_STATUS_OUT;
    status = tai_metadata_deepequal_attr_value(meta, &src, &dst, &result);
    if ( status != TAI_STATUS_SUCCESS ) {
        return -1;
    }
    if ( result != true ) {
        return -1;
    }
    status = tai_metadata_free_attr_value(meta, &src, NULL);
    if ( status != 0 ) {
        return -1;
    }
    status = tai_metadata_free_attr_value(meta, &dst, NULL);
    if ( status != 0 ) {
        return -1;
    }
    return 0;
}

int testSerializeJSONEnumList() {
    tai_serialize_option_t option = {true, true, true};
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_HOSTIF, TAI_HOST_INTERFACE_ATTR_TX_PCS_ALARM);
    tai_attribute_t attr = {0};
    tai_status_t status = tai_metadata_alloc_attr_value(meta, &attr, NULL);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("failed to alloc attr value: %d\n", status);
        return -1;
    }
    attr.id = TAI_HOST_INTERFACE_ATTR_TX_PCS_ALARM;
    attr.value.s32list.count = 8;
    for ( int i = 0; i < 8; i++ ) {
        attr.value.s32list.list[i] = i;
    }
    // check how much buf size we need;
    int ret = tai_serialize_attribute(NULL, 0, meta, &attr, &option);
    if ( ret < 0 ) {
        return -1;
    }
    char *p = malloc(ret+1);
    int ret2 = tai_serialize_attribute(p, ret+1, meta, &attr, &option);
    if ( ret != ret2 ) {
        free(p);
        return -1;
    }
    free(p);
    ret = tai_metadata_free_attr_value(meta, &attr, NULL);
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testDeserializeJSONCharlist() {
    char list[10] = {0};
    tai_char_list_t value;
    tai_serialize_option_t option = {
        .json = true,
    };
    value.count = 10;
    value.list = list;
    int ret = tai_deserialize_charlist("hello", &value, &option);
    if ( ret != TAI_SERIALIZE_ERROR ) {
        return -1;
    }
    value.count = 1;
    char *json = "\"hello\"";
    ret = tai_deserialize_charlist(json, &value, &option);
    if ( ret != TAI_STATUS_BUFFER_OVERFLOW ) {
        return -1;
    }
    value.count = 10;
    ret = tai_deserialize_charlist(json, &value, &option);
    if ( ret != strlen(json)) {
        return -1;
    }
    ret = strncmp(value.list, "hello", value.count);
    if ( ret != 0 ) {
        return -1;
    }
    return 0;
}

int testClearAttrValue() {
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_NETWORKIF, TAI_NETWORK_INTERFACE_ATTR_TX_ALIGN_STATUS);
    tai_attribute_t attr = {0};
    tai_status_t status;
    tai_alloc_info_t info = { .list_size = 5 };
    if ( attr.value.s32list.list != NULL || attr.value.s32list.count != 0 ) {
        printf("failed initialization\n");
        return -1;
    }
    status = tai_metadata_alloc_attr_value(meta, &attr, &info);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("failed to alloc attr value: %d\n", status);
        return -1;
    }
    if ( attr.value.s32list.list == NULL || attr.value.s32list.count != 5 ) {
        printf("failed alloc_attr_value() list = %p, count = %d\n", attr.value.s32list.list, attr.value.s32list.count);
        return -1;
    }
    status = tai_metadata_clear_attr_value(meta, &attr);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("failed to clear attr value: %d\n", status);
        return -1;
    }
    if ( attr.value.s32list.list == NULL || attr.value.s32list.count != 0 ) {
        printf("failed clear_attr_value()\n");
        return -1;
    }
    status = tai_metadata_free_attr_value(meta, &attr, NULL);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("failed to free attr value: %d\n", status);
        return -1;
    }
    if ( attr.value.s32list.list != NULL || attr.value.s32list.count != 0 ) {
        printf("failed free_attr_value()\n");
        return -1;
    }
    return 0;
}

typedef int (*testF)();

struct testCase {
    const char* name;
    testF f;
};

#define D(n) { #n , n }

struct testCase tests[] = {
    D(testSerializeModuleOperStatus),
    D(testDeserializeModuleOperStatus),
    D(testSerializeAttributeEnum),
    D(testSerializeAttributeFloat),
    D(testSerializeAttributeEnumList),
    D(testGetAttrMetadataByAttrIdName),
    D(testDeserializeNetworkInterfaceAttr),
    D(testDeepcopyAttrValue),
    D(testSerializeUnsignedRange),
    D(testSerializeSignedRange),
    D(testSerializeValueAttrList),
    D(testDeserializeU8list),
    D(testDeserializeU32list),
    D(testDeserializeObjlist),
    D(testDeserializeU8listLengthCheck),
    D(testDeserializeU8listInvalidValue),
    D(testDeserializeFloatlist),
    D(testSerializeListJSON),
    D(testDeserializeJSONU8List),
    D(testDeserializeJSONFloatList),
    D(testDeserializeJSONEnumList),
    D(testDeserializeJSONAttrList),
    D(testDeserializeJSONAttrList2),
    D(testDeserializeJSONBufferOverflow),
    D(testDeserializeJSONBufferOverflow2),
    D(testSerializeStatus),
    D(testSerializeAttrValueType),
    D(testDeepequalAttrValue),
    D(testSerializeJSONEnumList),
    D(testDeserializeJSONCharlist),
    D(testSerializeObjectMapList),
    D(testDeserializeObjectMapList),
    D(testAllocObjectMapList),
    D(testDeepcopyObjectMapList),
    D(testDeepequalObjectMapList),
    D(testClearAttrValue),
    D(NULL),
};

int main() {
    int i = 0, ret;

    tai_metadata_log_level = TAI_LOG_LEVEL_DEBUG;

    while (true) {
        if( tests[i].f == NULL ) {
            break;
        }
        ret = tests[i].f();
        if ( ret < 0 ) {
            printf("test %s(%d) failed\n", tests[i].name, i);
            return -ret;
        }
        ++i;
    }
    printf("test all passed!\n");
    return 0;
}
