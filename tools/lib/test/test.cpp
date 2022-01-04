#include <iostream>
#include "attribute.hpp"

#include "taimetadata.h"

int main() {
    const auto meta = tai_metadata_get_attr_metadata(TAI_OBJECT_TYPE_MODULE, TAI_MODULE_ATTR_TRIBUTARY_MAPPING);
    tai_serialize_option_t option = {.human = true, .valueonly = true};
    {
        auto attr = tai::Attribute(meta, "[]", &option);
        std::cout << attr.to_string(&option) << std::endl;
        if ( attr.to_string(&option) != "[]" ) {
            return -1;
        }
        std::cout << "." << std::endl;
    }
    {
        auto attr = tai::Attribute(meta, "[{\"oid:0x1\": [\"oid:0x2\"]}]", &option);
        if ( attr.to_string(&option) != "[{\"oid:0x1\": [\"oid:0x2\"]}]" ) {
            return -1;
        }
        std::cout << "." << std::endl;
    }
    {
        auto src = tai::Attribute(meta, "[{\"oid:0x1\": [\"oid:0x2\"]}]", &option);
        auto dst = src;
        if ( dst.to_string(&option) != "[{\"oid:0x1\": [\"oid:0x2\"]}]" ) {
            return -1;
        }
        std::cout << "." << std::endl;
    }
    return 0;
}
