#include <tai.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

tai_module_api_t *module_api;
tai_network_interface_api_t *network_interface_api;
tai_host_interface_api_t *host_interface_api;

#define TAI_MAX_HOST_IFS    8
#define TAI_MAX_NET_IFS     8

int g_module_location_head = 0;
int g_module_location_tail = 0;
char g_module_locations[TAI_MAX_MODULES+1][TAI_MAX_HARDWARE_ID_LEN];

tai_object_id_t g_module_ids[TAI_MAX_MODULES];
tai_object_id_t g_netif_ids[TAI_MAX_MODULES][TAI_MAX_NET_IFS];
tai_object_id_t g_hostif_ids[TAI_MAX_MODULES][TAI_MAX_HOST_IFS];


void module_shutdown(tai_object_id_t module_id)
{
    printf("Module shutdown request received for module_id %lx", module_id);
}

void module_state_change(tai_object_id_t module_id, tai_module_oper_status_t status)
{
    printf("Module state change received for module_id %lx. New state: %d",
           module_id, status);
}

tai_status_t create_modules() {
    tai_status_t status;
    tai_attribute_t attr[4];
    int network_ifs;
    int host_ifs;
    int i;

    while (g_module_location_head != g_module_location_tail) {
        attr[0].id = TAI_MODULE_ATTR_LOCATION;
        attr[0].value.charlist.count = strlen(g_module_locations[g_module_location_tail]);
        attr[0].value.charlist.list = g_module_locations[g_module_location_tail];
        attr[1].id = TAI_MODULE_ATTR_MODULE_SHUTDOWN_REQUEST_NOTIFY;
        attr[1].value.ptr = module_shutdown;
        attr[2].id = TAI_MODULE_ATTR_MODULE_STATE_CHANGE_NOTIFY;
        attr[2].value.ptr = module_state_change;
        status = module_api->create_module(&g_module_ids[g_module_location_tail], 3, attr);
        if ( status != TAI_STATUS_SUCCESS ) {
            return status;
        }

        attr[0].id = TAI_MODULE_ATTR_NUM_NETWORK_INTERFACES;
        attr[0].value.u32 = 0;
        status = module_api->get_module_attribute(g_module_ids[g_module_location_tail], &attr[0]);
        if ( status != TAI_STATUS_SUCCESS ) {
            return status;
        }
        printf("number of network interfaces on module %s: %d\n",
               g_module_locations[g_module_location_tail], attr[0].value.u32);
        network_ifs = attr[0].value.u32;

        for (i = 0; i < network_ifs; i++) {
            attr[0].id = TAI_NETWORK_INTERFACE_ATTR_INDEX;
            attr[0].value.u32 = i;

            attr[1].id = TAI_NETWORK_INTERFACE_ATTR_TX_DIS;
            attr[1].value.booldata = false;

            attr[2].id = TAI_NETWORK_INTERFACE_ATTR_TX_LASER_FREQ;
            attr[2].value.u64 = 191300000000000;

            attr[3].id = TAI_NETWORK_INTERFACE_ATTR_OUTPUT_POWER;
            attr[3].value.flt = 1.0;

            status = network_interface_api->create_network_interface(
               &g_netif_ids[g_module_location_tail][i],
               g_module_ids[g_module_location_tail], 4, &attr[0]); 
            if ( status != TAI_STATUS_SUCCESS ) {
                return status;
            }
        }

        attr[0].id = TAI_MODULE_ATTR_NUM_HOST_INTERFACES;
        attr[0].value.u32 = 0;
        status = module_api->get_module_attribute(g_module_ids[g_module_location_tail], &attr[0]);
        if ( status != TAI_STATUS_SUCCESS ) {
            return status;
        }
        printf("number of host interfaces on module %s: %d\n",
               g_module_locations[g_module_location_tail], attr[0].value.u32);
        host_ifs = attr[0].value.u32;

        for (i = 0; i < host_ifs; i++) {
            attr[0].id = TAI_HOST_INTERFACE_ATTR_INDEX;
            attr[0].value.u32 = i;

            status = host_interface_api->create_host_interface(
               &g_hostif_ids[g_module_location_tail][i],
               g_module_ids[g_module_location_tail], 1, &attr[0]); 
            if ( status != TAI_STATUS_SUCCESS ) {
                return status;
            }
        }

        attr[0].id = TAI_MODULE_ATTR_ADMIN_STATUS;
        attr[0].value.u32 = TAI_MODULE_ADMIN_STATUS_UP;

        status = module_api->set_module_attribute(
           g_module_ids[g_module_location_tail], &attr[0]); 
        if ( status != TAI_STATUS_SUCCESS ) {
            return status;
        }

        g_module_location_tail = (g_module_location_tail + 1) % (TAI_MAX_MODULES+1);
    }
    return status;
}

void module_event(bool present, char * module_location)
{
    int next_head = (g_module_location_head + 1) % (TAI_MAX_MODULES+1);

    printf("module_event: module %s is %s\n", 
           module_location, present ? "present" : "absent");
    if (present && (next_head != g_module_location_tail)) {
        strcpy(g_module_locations[g_module_location_head], module_location);
        g_module_location_head = next_head;
    }
    return;
}

tai_service_method_table_t g_service_table = {
    .module_presence = module_event
};

int main() {
    tai_status_t status;
    int i;
    status = tai_api_initialize(0, &g_service_table);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("failed to initialize TAI\n");
        return 1;
    }
    tai_log_set(TAI_API_MODULE, TAI_LOG_LEVEL_INFO, NULL);
    tai_log_set(TAI_API_HOSTIF, TAI_LOG_LEVEL_INFO, NULL);
    tai_log_set(TAI_API_NETWORKIF, TAI_LOG_LEVEL_INFO, NULL);

    status = tai_api_query(TAI_API_MODULE, (void**)&module_api);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("no api for TAI_API_MODULE\n");
        return 1;
    }

    printf("module_api: %p\n", module_api);

    status = tai_api_query(TAI_API_NETWORKIF, (void**)&network_interface_api);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("no api for TAI_API_NETWORKIF\n");
        return 1;
    }

    printf("network_interface_api: %p\n", network_interface_api);

    status = tai_api_query(TAI_API_HOSTIF, (void**)&host_interface_api);
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("no api for TAI_API_HOSTIF\n");
        return 1;
    }

    printf("host_interface_api: %p\n", host_interface_api);

    sleep(1);
    create_modules();

    status = tai_api_uninitialize();
    if ( status != TAI_STATUS_SUCCESS ) {
        printf("failed to uninitialize TAI\n");
        return 1;
    }
}
