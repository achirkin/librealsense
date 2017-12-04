// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

#pragma once

#include "core/processing.h"
#include "image.h"
#include "source.h"

namespace librealsense
{
    class synthetic_source : public synthetic_source_interface
    {
    public:
        synthetic_source(frame_source& actual)
            : _actual_source(actual), _c_wrapper(new rs2_source { this })
        {
        }

        frame_interface* allocate_video_frame(std::shared_ptr<stream_profile_interface> stream,
                                              frame_interface* original,
                                              int new_bpp = 0,
                                              int new_width = 0,
                                              int new_height = 0,
                                              int new_stride = 0,
                                              rs2_extension frame_type = RS2_EXTENSION_VIDEO_FRAME) override;

        frame_interface* allocate_composite_frame(std::vector<frame_holder> frames) override;

        frame_interface* allocate_points(std::shared_ptr<stream_profile_interface> stream, frame_interface* original) override;

        void frame_ready(frame_holder result) override;

        rs2_source* get_c_wrapper() override { return _c_wrapper.get(); }

    private:
        frame_source& _actual_source;
        std::shared_ptr<rs2_source> _c_wrapper;
    };

    class processing_block : public processing_block_interface, public options_container
    {
    public:
        processing_block();

        void set_processing_callback(frame_processor_callback_ptr callback) override;
        void set_output_callback(frame_callback_ptr callback) override;
        void invoke(frame_holder frames) override;

        synthetic_source_interface& get_source() override { return _source_wrapper; }

        virtual ~processing_block(){_source.flush();}
    protected:
        frame_source _source;
        std::mutex _mutex;
        frame_processor_callback_ptr _callback;
        synthetic_source _source_wrapper;
        rs2_extension _output_type;
    };

    class pointcloud : public processing_block
    {
    public:
        pointcloud();

    private:
        std::mutex              _mutex;

        const rs2_intrinsics*   _depth_intrinsics_ptr;
        const float*            _depth_units_ptr;
        const rs2_intrinsics*   _mapped_intrinsics_ptr;
        const rs2_extrinsics*   _extrinsics_ptr;

        rs2_intrinsics          _depth_intrinsics;
        rs2_intrinsics          _mapped_intrinsics;
        float                   _depth_units;
        rs2_extrinsics          _extrinsics;

        std::shared_ptr<stream_profile_interface> _stream, _mapped;
        stream_profile_interface* _depth_stream = nullptr;
    };

    template <typename T>
    class optional_value
    {
    public:
        optional_value() : _valid(false) {}
        explicit optional_value(const T& v) : _valid(true), _value(v) {}

        operator bool() const
        {
            return has_value();
        }
        bool has_value() const
        {
            return _valid;
        }
        
        T& operator=(const T& v)
        {
            _valid = true;
            _value = v;
            return _value;
        }
        
        T& value() &
        {
            if (!_valid) throw std::runtime_error("bad optional access");
            return _value;
        }

        T&& value() &&
        {
            if (!_valid) throw std::runtime_error("bad optional access");
            return std::move(_value);
        }

        const T* operator->() const
        {
            return &_value;
        }
        T* operator->()
        {
            return &_value;
        }
        const T& operator*() const&
        {
            return _value;
        }
        T& operator*() &
        {
            return _value;
        }
        T&& operator*() &&
        {
            return std::move(_value);
        }
    private:
        bool _valid;
        T _value;
    };

    class align : public processing_block
    {
    public:
        align(rs2_stream align_to);

    private:
        static void update_frame_info(const frame_interface* frame, optional_value<rs2_intrinsics>& intrin, std::shared_ptr<stream_profile_interface>& profile, bool register_extrin);
        void update_align_info(const frame_interface* depth_frame);

        std::mutex _mutex;
        optional_value<rs2_intrinsics> _from_intrinsics;
        optional_value<rs2_intrinsics> _to_intrinsics;
        optional_value<rs2_extrinsics> _extrinsics;
        optional_value<float> _depth_units;
        optional_value<int> _to_bytes_per_pixel;
        optional_value<rs2_stream> _from_stream_type;
        rs2_stream _to_stream_type;
        std::shared_ptr<stream_profile_interface> _from_stream_profile;
        std::shared_ptr<stream_profile_interface> _to_stream_profile;
        ;
    };
}
