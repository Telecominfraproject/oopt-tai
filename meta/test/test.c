#include "tai.h"
#include "taimetadata.h"
#include <stdio.h>
#include <string.h>

int testSerializeModuleOperStatus() {
    int ret;
    char buf[128] = {0};
    ret = tai_serialize_module_oper_status(buf, TAI_MODULE_OPER_STATUS_READY, NULL);
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
    ret = tai_serialize_attribute(buf, meta, &attr, &option);
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
    ret = tai_serialize_attribute(buf, meta, &attr, &option);
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
    ret = tai_serialize_attribute(buf, meta, &attr, &option);
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
    tai_attribute_t src, dst = {0};
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

typedef int (*testF)();

testF tests[] = {
    testSerializeModuleOperStatus,
    testDeserializeModuleOperStatus,
    testSerializeAttributeEnum,
    testSerializeAttributeFloat,
    testSerializeAttributeEnumList,
    testGetAttrMetadataByAttrIdName,
    testDeserializeNetworkInterfaceAttr,
    testDeepcopyAttrValue,
    NULL,
};

int main() {
    int i = 0, ret;

    tai_metadata_log_level = TAI_LOG_LEVEL_DEBUG;

    while (true) {
        if( tests[i] == NULL ) {
            break;
        }
        ret = tests[i++]();
        if ( ret < 0 ) {
            return ret;
        }
    }
    return 0;
}
