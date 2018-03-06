#include "tai.h"

sai_status_t stub_tai_create_optical_channel(sai_object_id_t *id,
                                             sai_object_id_t module_id,
                                             uint32_t attr_count,
                                             const tai_attribute_t *attr_list) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_tai_get_optical_channel_stats(sai_object_id_t id,
                                            uint32_t number_of_counters,
                                            const tai_optical_channel_stat_t *counter_ids,
                                            tai_attribute_value_t *counters) {
    return SAI_STATUS_SUCCESS;
}

tai_optical_channel_api_t stub_tai_optical_channel_api = {
    .create_optical_channel = stub_tai_create_optical_channel,
    .get_optical_channel_stats = stub_tai_get_optical_channel_stats,
};

sai_status_t stub_tai_create_optical_module(sai_object_id_t *id,
                                       sai_object_id_t device_id,
                                       uint32_t attr_count,
                                       const tai_attribute_t *attr_list) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_tai_remove_optical_module(sai_object_id_t optical_module_id){
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_tai_set_optical_module_attribute(sai_object_id_t optical_module_id, const tai_attribute_t *attr){
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_tai_get_optical_module_attribute(sai_object_id_t id, uint32_t attr_count, tai_attribute_t *attr_list){
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_tai_get_optical_module_stats(sai_object_id_t optical_module_id, uint32_t number_of_counters, const tai_optical_module_stat_t *counter_ids, tai_attribute_value_t *counters){
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_tai_clear_optical_module_stats(sai_object_id_t optical_module_id, uint32_t number_of_counters, const tai_optical_module_stat_t *counter_ids) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_tai_clear_optical_module_all_stats(sai_object_id_t optical_module_id) {
    return SAI_STATUS_SUCCESS;
}

tai_optical_module_api_t stub_tai_optical_module_api = {
    .create_optical_module = stub_tai_create_optical_module,
    .remove_optical_module = stub_tai_remove_optical_module,
    .set_optical_module_attribute = stub_tai_set_optical_module_attribute,
    .get_optical_module_attribute = stub_tai_get_optical_module_attribute,
    .get_optical_module_stats = stub_tai_get_optical_module_stats,
    .clear_optical_module_stats = stub_tai_clear_optical_module_stats,
    .clear_optical_module_all_stats = stub_tai_clear_optical_module_all_stats,
};

sai_status_t stub_tai_create_device(sai_object_id_t *device_id, uint32_t attr_count, const tai_attribute_t *attr_list) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_tai_remove_device(sai_object_id_t device_id) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_tai_set_device_attribute(sai_object_id_t device_id, const tai_attribute_t *attr_list) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_tai_get_device_attribute(sai_object_id_t device_id, uint32_t attr_count, tai_attribute_t *attr_list) {
    return SAI_STATUS_SUCCESS;
}

tai_device_api_t stub_tai_device_api = {
    .create_device = stub_tai_create_device,
    .remove_device = stub_tai_remove_device,
    .set_device_attribute = stub_tai_set_device_attribute,
    .get_device_attribute = stub_tai_get_device_attribute,
};

sai_status_t tai_api_initialize(uint64_t flags, const service_method_table_t* services) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t tai_api_uninitialize(void) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t tai_log_set(tai_api_t stub_tai_api_id, tai_log_level_t log_level) {
    return SAI_STATUS_SUCCESS;
}

sai_status_t tai_api_query(tai_api_t tai_api_id, void** api_method_table) {
    switch (tai_api_id) {
    case TAI_API_DEVICE:
        *api_method_table = &stub_tai_device_api;
        break;
    case TAI_API_OPTICAL_MODULE:
        *api_method_table = &stub_tai_optical_module_api;
        break;
    case TAI_API_OPTICAL_CHANNEL:
        *api_method_table = &stub_tai_optical_channel_api;
        break;
    default:
        return SAI_STATUS_NOT_SUPPORTED;
    }
    return SAI_STATUS_SUCCESS;
}
