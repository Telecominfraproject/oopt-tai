#ifndef __CUSTOM_MODULE__
#define __CUSTOM_MODULE__

#include <tai.h>

/**
 * @brief Custom enum
 */
typedef enum _tai_module_custom_status_t
{
    TAI_MODULE_CUSTOM_STATUS_UNKNOWN,
    TAI_MODULE_CUSTOM_STATUS_A,
    TAI_MODULE_CUSTOM_STATUS_B,
    TAI_MODULE_CUSTOM_STATUS_MAX,
} tai_module_custom_status_t;

/**
 * @brief definition of custom module attributes
 */
typedef enum _custom_module_attr_t
{
    TAI_MODULE_ATTR_CUSTOM_START = TAI_MODULE_ATTR_CUSTOM_RANGE_START,

    /**
     * @brief Custom attribute
     *
     * @type #tai_module_custom_status_t
     * @flags READ_ONLY
     */
    TAI_MODULE_ATTR_CUSTOM_STATUS,

    TAI_MODULE_ATTR_CUSTOM_END = TAI_MODULE_ATTR_CUSTOM_RANGE_END,

} custom_module_attr_t;

#endif // __CUSTOM_MODULE__
