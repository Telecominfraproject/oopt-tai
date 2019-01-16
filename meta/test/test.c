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

typedef int (*testF)();

testF tests[] = {
    testSerializeModuleOperStatus,
    testDeserializeModuleOperStatus,
    testSerializeAttributeEnum,
    testSerializeAttributeFloat,
    testSerializeAttributeEnumList,
    testGetAttrMetadataByAttrIdName,
    testDeserializeNetworkInterfaceAttr,
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
