#pragma once
#include "librealsense2/rs.hpp"
#include <type_traits>

// extra checks to help specialization priority resoultion
template <typename T> using extra_checks = std::bool_constant<std::is_base_of<rs2::frame, T>::value || std::is_base_of<rs2::options, T>::value>;

// C API

// rs_types.hpp

// rs_frame.hpp
template<> struct MatlabParamParser::type_traits<rs2::stream_profile> { using rs2_internal_t = const rs2_stream_profile*; };
template<typename T> struct MatlabParamParser::type_traits<T, typename std::enable_if<std::is_base_of<rs2::frame, T>::value>::type> { using rs2_internal_t = rs2_frame * ; };

// rs_sensor.hpp
template<> struct MatlabParamParser::type_traits<rs2::options> {
    struct carrier {
        void * ptr;
        enum class types { rs2_sensor } type;
        carrier(void *ptr_, types t) : ptr(ptr_), type(t) {}
        ~carrier(); // implemented at the bottom with an explanation as to why
    };
};
template<typename T> struct MatlabParamParser::type_traits<T, typename std::enable_if<std::is_base_of<rs2::sensor, T>::value>::type> {
    using rs2_internal_t = std::shared_ptr<rs2_sensor>;
    using carrier_t = type_traits<rs2::options>::carrier;
    static const carrier_t::types carrier_enum = carrier_t::types::rs2_sensor;
    // wrapper creates new object on the heap, so don't allow the wrapper to unload before the object is destroyed.
    // librealsense types are sent to matlab using a pointer to the internal type.
    static carrier_t* make_carrier(T&& var) { mexLock(); return new carrier_t(new rs2_internal_t(var), carrier_t::types::rs2_sensor); }
    static T from_carrier(carrier_t *carrier) { return T(rs2::sensor(*reinterpret_cast<rs2_internal_t*>(carrier->ptr))); }
};

// rs_device.hpp
template<> struct MatlabParamParser::type_traits<rs2::device> { using rs2_internal_t = std::shared_ptr<rs2_device>; };
template<> struct MatlabParamParser::type_traits<rs2::device_list> { using rs2_internal_t = std::shared_ptr<rs2_device_list>; };

// rs_record_playback.hpp
template<> struct MatlabParamParser::type_traits<rs2::playback> : MatlabParamParser::type_traits<rs2::device> {
    using special_t = traits_trampoline::special_;
    static rs2::playback from_internal(rs2_internal_t * ptr) { return traits_trampoline::from_internal<rs2::device>(ptr, special_t()); }
};

// rs_processing.hpp

// rs_context.hpp
// rs2::event_information                       [?]
template<> struct MatlabParamParser::type_traits<rs2::context> { using rs2_internal_t = std::shared_ptr<rs2_context>; };
template<> struct MatlabParamParser::type_traits<rs2::device_hub> { using rs2_internal_t = std::shared_ptr<rs2_device_hub>; };

// rs_pipeline.hpp
template<> struct MatlabParamParser::type_traits<rs2::pipeline> { using rs2_internal_t = std::shared_ptr<rs2_pipeline>; };
template<> struct MatlabParamParser::type_traits<rs2::config> { using rs2_internal_t = std::shared_ptr<rs2_config>; };
template<> struct MatlabParamParser::type_traits<rs2::pipeline_profile> { using rs2_internal_t = std::shared_ptr<rs2_config>; };

// This needs to go at the bottom so that all the relevant type_traits specializations will have already occured.
MatlabParamParser::type_traits<rs2::options>::carrier::~carrier() {
    switch (type) {
    case types::rs2_sensor: delete reinterpret_cast<type_traits<rs2::sensor>::rs2_internal_t*>(ptr);
    }
}