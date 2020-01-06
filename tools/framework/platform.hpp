#ifndef __TAI_FRAMEWORK_PLATFORM_HPP__
#define __TAI_FRAMEWORK_PLATFORM_HPP__

#include "object.hpp"

namespace tai::framework {

    using Location = std::string;

    class Platform {
        public:
            Platform(const tai_service_method_table_t * services) : m_services(services) {};
            virtual tai_status_t create(tai_object_type_t type, tai_object_id_t module_id, uint32_t attr_count, const tai_attribute_t *attr_list, tai_object_id_t *id) { return TAI_STATUS_NOT_SUPPORTED; }
            tai_status_t create(tai_object_type_t type, uint32_t attr_count, const tai_attribute_t *attr_list, tai_object_id_t *id) {
                return create(type, TAI_NULL_OBJECT_ID, attr_count, attr_list, id);
            }
            virtual tai_status_t remove(tai_object_id_t id) = 0;

            S_BaseObject get(tai_object_id_t id, tai_object_type_t filter = TAI_OBJECT_TYPE_NULL) {
                auto it = m_objects.find(id);
                if ( it == m_objects.end() ) {
                    return nullptr;
                }
                if ( filter == TAI_OBJECT_TYPE_NULL || it->second->type() == filter ) {
                    return it->second;
                }
                return nullptr;
            }

            virtual tai_object_type_t get_object_type(tai_object_id_t id) = 0;
            virtual tai_object_id_t get_module_id(tai_object_id_t id) = 0;

            virtual tai_status_t set_log(tai_api_t tai_api_id, tai_log_level_t log_level, tai_log_fn log_fn) {
                return TAI_STATUS_SUCCESS;
            }

        protected:
            const tai_service_method_table_t * m_services;
            // we don't need a lock to access m_objects/m_fsms since TAI API is not thread-safe
            std::map<tai_object_id_t, S_BaseObject> m_objects;
            std::map<Location, S_FSM> m_fsms;
    };

};

#endif // __PLATFORM_HPP__
