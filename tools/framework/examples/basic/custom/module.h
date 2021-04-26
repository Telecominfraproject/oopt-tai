#ifndef __TAI_BASIC_MODULE__
#define __TAI_BASIC_MODULE__

#include <tai.h>

typedef enum _basic_module_attr_t
{
    /**
     * @brief Custom attribute example
     *
     * @type bool
     * @flags CREATE_AND_SET
     */
    TAI_MODULE_ATTR_CUSTOM = TAI_MODULE_ATTR_CUSTOM_RANGE_START,

} basic_module_attr_t;

#endif
