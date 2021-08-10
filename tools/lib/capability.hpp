#ifndef __CAPABILITY_HPP__
#define __CAPABILITY_HPP__

#include "attribute.hpp"
#include <memory>

namespace tai {

    class Capability;

    using S_Capability = std::shared_ptr<Capability>;

    using cap_getter = std::function<tai_status_t(tai_attribute_capability_t*)>;

    class Capability {
        public:
            Capability(const tai_attr_metadata_t* const meta, cap_getter getter) :  m_meta(meta), m_cap() {
                if ( meta == nullptr ) {
                    throw Exception(TAI_STATUS_INVALID_PARAMETER);
                }
                if ( meta->isenum && meta->enummetadata != nullptr ) {
                    tai_attr_metadata_t m = {
                        .attrvaluetype = TAI_ATTR_VALUE_TYPE_ATTRLIST,
                        .attrlistvaluetype = TAI_ATTR_VALUE_TYPE_S32
                    };
                    tai_alloc_info_t alloc_info = {
                        .list_size = static_cast<uint32_t>(meta->enummetadata->valuescount)
                    };
                    tai_attribute_t a = {};
                    auto ret = tai_metadata_alloc_attr_value(&m, &a, &alloc_info);
                    if ( ret != TAI_STATUS_SUCCESS ) {
                        throw Exception(ret);
                    }
                    m_cap.supportedvalues = a.value.attrlist;
                }
                m_cap.id = meta->attrid;
                auto ret = getter(&m_cap);
                if ( ret != TAI_STATUS_SUCCESS ) {
                    _free();
                    throw Exception(ret);
                }
            }

            const tai_attribute_capability_t* const raw(){
                return &m_cap;
            }

            ~Capability(){
                _free();
            }

        private:
            const tai_attr_metadata_t* const m_meta;
            tai_attribute_capability_t m_cap;

            tai_status_t _free() {
                if ( !m_meta->isenum || m_meta->enummetadata == nullptr ) {
                    return TAI_STATUS_SUCCESS;
                }
                tai_attr_metadata_t m{
                    .attrvaluetype = TAI_ATTR_VALUE_TYPE_ATTRLIST,
                    .attrlistvaluetype = TAI_ATTR_VALUE_TYPE_S32
                };
                tai_attribute_t a{.value = {.attrlist = m_cap.supportedvalues}};
                return tai_metadata_free_attr_value(&m, &a, nullptr);
            }
    };
}

#endif // __CAPABILITY_HPP__
