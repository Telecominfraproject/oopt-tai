#include "attribute.hpp"
#include "logger.hpp"
#include <memory>

namespace tai {

    Attribute::Attribute(const tai_attr_metadata_t* const meta, const std::string& value, const tai_serialize_option_t* const option) : m_meta(meta) {
        tai_attribute_t attr = {.id = meta->attrid};
        auto ret = tai_deserialize_attribute_value(value.c_str(), meta, &attr.value, option);
        if ( ret == TAI_STATUS_BUFFER_OVERFLOW ) {
            tai_alloc_info_t alloc_info = { .reference = &attr };
            ret = tai_metadata_alloc_attr_value(meta, &attr, &alloc_info);
            if ( ret != TAI_STATUS_SUCCESS ) {
                throw Exception(ret);
            }
            ret = tai_deserialize_attribute_value(value.c_str(), meta, &attr.value, option);
            if ( ret < 0 ) {
                tai_metadata_free_attr_value(meta, &attr, nullptr);
                throw Exception(ret);
            }
        } else if ( ret < 0 ) {
            throw Exception(ret);
        }
        m_attr = attr;
    }

    std::string Attribute::to_string(tai_serialize_option_t* option) {
        auto size = tai_serialize_attribute(nullptr, 0, m_meta, &m_attr, option);
        if ( size < 0 ) {
            return "null";
        }
        auto p = std::make_unique<char[]>(size+10);
        auto ret = tai_serialize_attribute(p.get(), size+1, m_meta, &m_attr, option);
        if ( ret < 0 || ret > size+1 ) {
            return "null";
        }
        return std::string(p.get());
    }

    bool cmp(const S_Attribute lhs, const S_Attribute rhs) {
        return lhs->cmp(rhs);
    }

    inline bool operator==(const S_Attribute lhs, const S_Attribute rhs) { return cmp(lhs, rhs) == 0; }
    inline bool operator!=(const S_Attribute lhs, const S_Attribute rhs) { return cmp(lhs, rhs) != 0; }
    std::ostream& operator<<(std::ostream& os, const S_Attribute attr) { return attr->str(os); }

}
