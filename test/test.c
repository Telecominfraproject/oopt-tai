#include <tai.h>
#include <stdio.h>

tai_device_api_t *device_api;
tai_optical_module_api_t *optical_module_api;
tai_optical_channel_api_t *optical_channel_api;

sai_object_id_t g_device_id;
sai_object_id_t g_port_ids[8];
sai_object_id_t g_channel_ids[8][2];

sai_status_t create_device() {
    sai_status_t status;
    tai_attribute_t attr;
    status = device_api->create_device(&g_device_id, 0, NULL);
    if ( status != SAI_STATUS_SUCCESS ) {
        return status;
    }
    attr.id = TAI_DEVICE_ATTR_OPTICAL_MODULE_NUMBER;
    attr.value.u32 = 0;
    status = device_api->get_device_attribute(g_device_id, 1, &attr);
    if ( status != SAI_STATUS_SUCCESS ) {
        return status;
    }
    printf("number of optical module: %d\n", attr.value.u32);

    return status;
}

sai_status_t create_port(int i) {
    sai_status_t status;
    tai_attribute_t attr_list[6];
    int j;

    attr_list[0].id = TAI_OPTICAL_MODULE_ATTR_MODULE_INDEX;
    attr_list[0].value.u32 = (sai_uint32_t)i;

    attr_list[1].id = TAI_OPTICAL_MODULE_ATTR_MODULATION_FORMAT;
    attr_list[1].value.s32 = TAI_OPTICAL_MODULE_MODULATION_FORMAT_DP_16QAM;

    attr_list[2].id = TAI_OPTICAL_MODULE_ATTR_TX_FREQUENCY_GRID;
    attr_list[2].value.s32 = TAI_OPTICAL_MODULE_CHANNEL_GRID_50GHZ;

    attr_list[3].id = TAI_OPTICAL_MODULE_ATTR_TX_FREQUENCY_CHANNEL;
    attr_list[3].value.u8 = 1;

    attr_list[4].id = TAI_OPTICAL_MODULE_ATTR_RX_FREQUENCY_GRID;
    attr_list[4].value.s32 = TAI_OPTICAL_MODULE_CHANNEL_GRID_50GHZ;

    attr_list[5].id = TAI_OPTICAL_MODULE_ATTR_RX_FREQUENCY_CHANNEL;
    attr_list[5].value.u8 = 1;

    status = optical_module_api->create_optical_module(&g_port_ids[i], g_device_id, 6, attr_list);
    if ( status != SAI_STATUS_SUCCESS ) {
        printf("failed to create optical module %d\n", i);
        return status;
    }

    attr_list[0].id = TAI_OPTICAL_MODULE_ATTR_OPTICAL_CHANNEL_NUMBER;

    status = optical_module_api->get_optical_module_attribute(g_port_ids[i], 1, attr_list);
    if ( status != SAI_STATUS_SUCCESS ) {
        printf("failed to get optical channel number\n");
        return status;
    }

    for ( j = 0; j < attr_list[0].value.u32; j++ ) {
        status = optical_channel_api->create_optical_channel(&g_channel_ids[i][j], g_port_ids[i], 0, NULL);
        if ( status != SAI_STATUS_SUCCESS ) {
            printf("failed to create optical channel: i %d, j %d\n", i, j);
            return status;
        }
    }

    return status;
}

int main() {
    sai_status_t status;
    int i;
    status = tai_api_initialize(0, NULL);
    if ( status != SAI_STATUS_SUCCESS ) {
        printf("failed to initialize TAI\n");
        return 1;
    }
    tai_log_set(TAI_API_DEVICE, TAI_LOG_LEVEL_INFO);
    tai_log_set(TAI_API_OPTICAL_MODULE, TAI_LOG_LEVEL_INFO);

    status = tai_api_query(TAI_API_DEVICE, (void**)&device_api);
    if ( status != SAI_STATUS_SUCCESS ) {
        printf("no api for TAI_API_DEVICE\n");
        return 1;
    }

    printf("device_api: %p\n", device_api);

    status = tai_api_query(TAI_API_OPTICAL_MODULE, (void**)&optical_module_api);
    if ( status != SAI_STATUS_SUCCESS ) {
        printf("no api for TAI_API_DEVICE\n");
        return 1;
    }

    printf("optical_module_api: %p\n", optical_module_api);

    status = tai_api_query(TAI_API_OPTICAL_CHANNEL, (void**)&optical_channel_api);
    if ( status != SAI_STATUS_SUCCESS ) {
        printf("no api for TAI_API_CHANNEL\n");
        return 1;
    }

    printf("optical_channel_api: %p\n", optical_channel_api);

    create_device();
    for ( i = 0; i < 8; i++ ) {
        create_port(i);
    }

    status = tai_api_uninitialize();
    if ( status != SAI_STATUS_SUCCESS ) {
        printf("failed to uninitialize TAI\n");
        return 1;
    }
}
