#include <iostream>
#include <memory>
#include "tai.h"
#include "exception.hpp"

static std::unique_ptr<tai::framework::Platform> g_platform;

/**
 * @brief Retrieve a list of attribute values
 *
 * @param [in] host_interface_id The host interface ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in,out] attr_list A list of attributes to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t get_host_interface_attributes(
    _In_ tai_object_id_t     id,
    _In_ uint32_t            attr_count,
    _Inout_ tai_attribute_t *attr_list)
{
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    auto hostif = g_platform->get(id, TAI_OBJECT_TYPE_HOSTIF);
    if ( hostif == nullptr ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }
    return hostif->get_attributes(attr_count, attr_list);
}

/**
 * @brief Retrieve the value of an attribute
 *
 * @param [in] host_interface_id The host interface ID handle
 * @param [in,out] attr A pointer to the attribute to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t get_host_interface_attribute(_In_ tai_object_id_t id, _Inout_ tai_attribute_t *attr)
{
    return get_host_interface_attributes(id, 1, attr);
}

/**
 * @brief Set the values from a list of attributes
 *
 * @param [in] host_interface_id The host interface ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t set_host_interface_attributes(
   _In_ tai_object_id_t        id,
   _In_ uint32_t               attr_count,
   _In_ const tai_attribute_t *attr_list)
{
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    auto hostif = g_platform->get(id, TAI_OBJECT_TYPE_HOSTIF);
    if ( hostif == nullptr ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }
    return hostif->set_attributes(attr_count, attr_list);
}

/**
 * @brief Set the value of an attribute
 *
 * @param [in] host_interface_id The host interface ID handle
 * @param [in] attr A pointer to the attribute to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t set_host_interface_attribute(_In_ tai_object_id_t id, _In_ const tai_attribute_t *attr)
{
    return set_host_interface_attributes(id, 1, attr);
}

/**
 * @brief Host interface initialization. After the call the capability 
 *        attributes should be ready for retrieval via
 *        tai_get_host_interface_attribute().
 *
 * @param [out] host_interface_id Handle which identifies the host interface
 * @param [in] module_id Handle which identifies the module on which the host 
 *        interface exists
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to set during initialization
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t create_host_interface(
    _Out_ tai_object_id_t *host_interface_id,
    _In_ tai_object_id_t module_id,
    _In_ uint32_t attr_count,
    _In_ const tai_attribute_t *attr_list)
{
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    return g_platform->create(TAI_OBJECT_TYPE_HOSTIF, module_id, attr_count, attr_list, host_interface_id);
}

/**
 * @brief Release all resources associated with previously created host 
 *        interface
 *
 * @param [in] host_interface_id The host interface ID handle being removed
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t remove_host_interface(_In_ tai_object_id_t host_interface_id)
{
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    return g_platform->remove(host_interface_id);
}

static tai_status_t clear_host_interface_attributes(
    _In_ tai_object_id_t     id,
    _In_ uint32_t            attr_count,
    _Inout_ tai_attr_id_t   *attr_list)
{
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    auto hostif = g_platform->get(id, TAI_OBJECT_TYPE_HOSTIF);
    if ( hostif == nullptr ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }
    return hostif->clear_attributes(attr_count, attr_list);
}

static tai_status_t clear_host_interface_attribute(_In_ tai_object_id_t id, _In_ tai_attr_id_t attr_id)
{
    return clear_host_interface_attributes(id, 1, &attr_id);
}

/**
 * @brief The host interface functions. This structure is retrieved via the 
 *        #tai_api_query function.
 */
static tai_host_interface_api_t host_interface_api = {
    .create_host_interface           = create_host_interface,
    .remove_host_interface           = remove_host_interface,
    .set_host_interface_attribute    = set_host_interface_attribute,
    .set_host_interface_attributes   = set_host_interface_attributes,
    .get_host_interface_attribute    = get_host_interface_attribute,
    .get_host_interface_attributes   = get_host_interface_attributes,
    .clear_host_interface_attribute  = clear_host_interface_attribute,
    .clear_host_interface_attributes = clear_host_interface_attributes
};

/**
 * @brief Retrieve a list of attribute values
 *
 * @param [in] network_interface_id The network interface ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in,out] attr_list A list of attributes to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t get_network_interface_attributes(
    _In_ tai_object_id_t     id,
    _In_ uint32_t            attr_count,
    _Inout_ tai_attribute_t *attr_list)
{
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    auto netif = g_platform->get(id, TAI_OBJECT_TYPE_NETWORKIF);
    if ( netif == nullptr ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }
    return netif->get_attributes(attr_count, attr_list);
}

/**
 * @brief Retrieve the value of an attribute
 *
 * @param [in] network_interface_id The network interface ID handle
 * @param [in,out] attr A pointer to the attribute to be retrieved
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t get_network_interface_attribute(
    _In_ tai_object_id_t     id,
    _Inout_ tai_attribute_t *attr)
{
    return get_network_interface_attributes(id, 1, attr);
}

/**
 * @brief Set the values from a list of attributes
 *
 * @param [in] network_interface_id The network interface ID handle
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t set_network_interface_attributes(
   _In_ tai_object_id_t        id,
   _In_ uint32_t               attr_count,
   _In_ const tai_attribute_t *attr_list)
{
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    auto netif = g_platform->get(id, TAI_OBJECT_TYPE_NETWORKIF);
    if ( netif == nullptr ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }
    return netif->set_attributes(attr_count, attr_list);
}


/**
 * @brief Set the value of an attribute
 *
 * @param [in] network_interface_id The network interface ID handle
 * @param [in] attr A pointer to the attribute to be set
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t set_network_interface_attribute(
   _In_ tai_object_id_t        id,
   _In_ const tai_attribute_t *attr)
{
    return set_network_interface_attributes(id, 1, attr);
}


/**
 * @brief Network interface initialization. After the call the capability 
 *        attributes should be ready for retrieval via
 *        tai_get_network_interface_attribute().
 *
 * @param [out] network_interface_id Handle which identifies the network 
 *        interface
 * @param [in] module_id Module id on which the network interface exists
 * @param [in] attr_count A count of the number of elements in the attr_list
 * @param [in] attr_list A list of attributes to set during initialization
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t create_network_interface(
    _Out_ tai_object_id_t *network_interface_id,
    _In_ tai_object_id_t module_id,
    _In_ uint32_t attr_count,
    _In_ const tai_attribute_t *attr_list)
{
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    return g_platform->create(TAI_OBJECT_TYPE_NETWORKIF, module_id, attr_count, attr_list, network_interface_id);
}

/**
 * @brief Release all resources associated with previously created network 
 *        interface
 *
 * @param [in] network_interface_id The network interface ID handle being 
 *        removed
 *
 * @return TAI_STATUS_SUCCESS on success, failure status code on error
 */
static tai_status_t remove_network_interface(_In_ tai_object_id_t network_interface_id)
{
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    return g_platform->remove(network_interface_id);
}

/**
 * @brief The network interface functions. This structure is retrieved via the 
 *        #tai_api_query function.
 */
static tai_network_interface_api_t network_interface_api = {
    .create_network_interface         = create_network_interface,
    .remove_network_interface         = remove_network_interface,
    .set_network_interface_attribute  = set_network_interface_attribute,
    .set_network_interface_attributes = set_network_interface_attributes,
    .get_network_interface_attribute  = get_network_interface_attribute,
    .get_network_interface_attributes = get_network_interface_attributes
};

tai_status_t remove_module(tai_object_id_t module_id) {
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    return g_platform->remove(module_id);
}

tai_status_t set_module_attributes(tai_object_id_t module_id, uint32_t attr_count, const tai_attribute_t *attr_list) {
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    auto module = g_platform->get(module_id, TAI_OBJECT_TYPE_MODULE);
    if ( module == nullptr ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }
    return module->set_attributes(attr_count, attr_list);
}

tai_status_t set_module_attribute(tai_object_id_t module_id, const tai_attribute_t *attr_list) {
    return set_module_attributes(module_id, 1, attr_list);
}

tai_status_t get_module_attributes(tai_object_id_t module_id, uint32_t attr_count, tai_attribute_t *attr_list) {
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    auto module = g_platform->get(module_id, TAI_OBJECT_TYPE_MODULE);
    if ( module == nullptr ) {
        return TAI_STATUS_ITEM_NOT_FOUND;
    }
    return module->get_attributes(attr_count, attr_list);
}

tai_status_t get_module_attribute(tai_object_id_t module_id, tai_attribute_t *attr_list) {
    return get_module_attributes(module_id, 1, attr_list);
}

static tai_status_t create_module(tai_object_id_t *module_id, uint32_t attr_count, const tai_attribute_t *attr_list) {
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    return g_platform->create(TAI_OBJECT_TYPE_MODULE, attr_count, attr_list, module_id);
}

static tai_module_api_t module_api = {
    .create_module = create_module,
    .remove_module = remove_module,
    .set_module_attribute =  set_module_attribute,
    .set_module_attributes = set_module_attributes,
    .get_module_attribute =  get_module_attribute,
    .get_module_attributes = get_module_attributes,
};

tai_status_t tai_api_initialize(uint64_t flags, const tai_service_method_table_t* services) {
    if ( g_platform != nullptr ) {
        return TAI_STATUS_FAILURE;
    }
    try {
        g_platform.reset(new ::Platform(services));
    } catch ( tai::Exception& e ) {
        return e.err();
    }
    return TAI_STATUS_SUCCESS;
}

tai_status_t tai_api_uninitialize(void) {
    if ( g_platform == nullptr ) {
        return TAI_STATUS_UNINITIALIZED;
    }
    g_platform.reset();
    return TAI_STATUS_SUCCESS;
}

tai_status_t tai_log_set(tai_api_t tai_api_id, tai_log_level_t log_level, tai_log_fn log_fn) {
    if ( g_platform != nullptr ) {
        auto ret = g_platform->set_log(tai_api_id, log_level, log_fn);
        if ( ret != TAI_STATUS_SUCCESS ) {
            return ret;
        }
    }
    return tai::Logger::get_instance().set_log(tai_api_id, log_level, log_fn);
}

tai_status_t tai_api_query(tai_api_t tai_api_id, void** api_method_table) {
    switch (tai_api_id) {
    case TAI_API_MODULE:
        *api_method_table = &module_api;
        break;
    case TAI_API_NETWORKIF:
        *api_method_table = &network_interface_api;
        break;
    case TAI_API_HOSTIF:
        *api_method_table = &host_interface_api;
        break;
    default:
        return TAI_STATUS_NOT_SUPPORTED;
    }
    return TAI_STATUS_SUCCESS;
}

tai_object_type_t tai_object_type_query(tai_object_id_t id) {
    if ( g_platform == nullptr ) {
        return TAI_OBJECT_TYPE_NULL;
    }
    return g_platform->get_object_type(id);
}

tai_object_id_t tai_module_id_query(tai_object_id_t id) {
    if ( g_platform == nullptr ) {
        return TAI_NULL_OBJECT_ID;
    }
    return g_platform->get_module_id(id);
}

tai_status_t tai_dbg_generate_dump(const char *dump_file_name) {
    return TAI_STATUS_NOT_SUPPORTED;
}
