#include "tai.h"
#include "taimetadata.h"
#include "stdio.h"

int test(char *buffer) {
    tai_attribute_t attr = {0};
    tai_serialize_option_t option = {
        .valueonly = true,
    };
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_MODULE, TAI_MODULE_ATTR_OPER_STATUS);
    attr.id = TAI_MODULE_ATTR_OPER_STATUS;
    attr.value.s32 = TAI_MODULE_OPER_STATUS_READY;
    return tai_serialize_attribute(buffer, meta, &attr, &option);
}

int test2(char *buffer) {
    tai_attribute_t attr = {0};
    tai_serialize_option_t option = {
        .valueonly = true,
    };
    const tai_attr_metadata_t* meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_MODULE, TAI_MODULE_ATTR_TEMP);
    attr.id = TAI_MODULE_ATTR_TEMP;
    attr.value.flt = 1.10;
    return tai_serialize_attribute(buffer, meta, &attr, &option);
}

int test3(char *buffer) {
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

    return tai_serialize_attribute(buffer, meta, &attr, &option);
}

int test4() {
    const tai_attr_metadata_t* meta;
    meta = tai_metadata_get_attr_metadata_by_attr_id_name("TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ");
    printf("%p\n", meta);
    return 0;
}

int test5() {
    int32_t value;
    int ret;
    tai_serialize_option_t option = {
        .human = true,
    };
    ret = tai_deserialize_network_interface_attr("tx-channel", &value, &option);
    printf("%d, %d\n", value, ret);
    return 0;
}

int main() {
    int ret, value;
    char buf[128] = {0};

    ret = tai_serialize_module_oper_status(buf, TAI_MODULE_OPER_STATUS_READY, NULL);

    printf("%s, %d\n", buf, ret);
    ret = tai_deserialize_module_oper_status("TAI_MODULE_OPER_STATUS_INITIALIZE", &value, NULL);
    printf("%d, %d\n", value, ret);

    ret = test(buf);
    printf("%s, %d\n", buf, ret);

    ret = test2(buf);
    printf("%s, %d\n", buf, ret);

    ret = test3(buf);
    printf("%s, %d\n", buf, ret);

    test4();

    test5();
}
