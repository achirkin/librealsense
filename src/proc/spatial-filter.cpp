// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.
#include "../include/librealsense2/rs.hpp"

#include<algorithm>

#include "option.h"
#include "environment.h"
#include "context.h"
#include "proc/synthetic-stream.h"
#include "proc/spatial-filter.h"

// TODO refactor
# define DO_HORIZONTAL
# define DO_LEFT_TO_RIGHT
# define DO_RIGHT_TO_LEFT
# define DO_VERTICAL
# define DO_TOP_TO_BOTTOM
# define DO_BOTTOM_TO_TOP

namespace librealsense
{
    const uint8_t spatial_patch_min_val = 3;
    const uint8_t spatial_patch_max_val = 11;
    const uint8_t spatial_patch_default_val = 5;
    const uint8_t spatial_patch_step = 2;    // The filter suppors non-even kernels in the range of [3..11]


    spatial_filter::spatial_filter() :
        _spatial_param(spatial_patch_default_val),
        _window_size(_spatial_param*_spatial_param),
        _patch_size(spatial_patch_default_val),
        _spatial_alpha_param(0.85f),
        _spatial_delta_param(50.f),
        _width(0), _height(0),
        _range_from(1), _range_to(0xFFFF),
        _enable_filter(true)
    {
        //"Spatial Alpha [0.1,.0.2,.. ..., 0.85,   ...,2] size"
        auto spatial_filter_alpha = std::make_shared<ptr_option<float>>(0.1f, 2.f, 0.05f, 0.85f, &_spatial_alpha_param, "Spatial alpha");
        auto spatial_filter_delta = std::make_shared<ptr_option<float>>(25.f, 100.f, 5.f, 50.f, &_spatial_delta_param, "Spatial delta");
        auto spatial_filter_control = std::make_shared<ptr_option<uint8_t>>(
            spatial_patch_min_val, spatial_patch_max_val,
            spatial_patch_step, spatial_patch_default_val,
            &_spatial_param, "Spatial kernel size");

        //spatial_filter_control->on_set([this, spatial_filter_control](float val)
        //{
        //    if (!spatial_filter_control->is_valid(val))
        //        throw invalid_value_exception(to_string()
        //            << "Unsupported spatial patch size " << val << " is out of range.");

        //    _patch_size = uint8_t(val);
        //    _window_size = _patch_size*_patch_size;
        //});

        register_option(RS2_OPTION_FILTER_MAGNITUDE, spatial_filter_control);
        register_option(RS2_OPTION_FILTER_OPT1, spatial_filter_alpha);
        register_option(RS2_OPTION_FILTER_OPT2, spatial_filter_delta);
        unregister_option(RS2_OPTION_FRAMES_QUEUE_SIZE);

        auto enable_control = std::make_shared<ptr_option<bool>>(false, true, true, true, &_enable_filter, "Apply spatial dxf");
        register_option(RS2_OPTION_FILTER_ENABLED, enable_control);
        _enable_filter = true;

        auto on_frame = [this](rs2::frame f, const rs2::frame_source& source)
        {
            rs2::frame out = f, tgt, depth;

            if (this->_enable_filter)
            {
                bool composite = f.is<rs2::frameset>();

                depth = (composite) ? f.as<rs2::frameset>().first_or_default(RS2_STREAM_DEPTH) : f;
                if (depth) // Processing required
                {
                    update_configuration(f);
                    tgt = prepare_target_frame(depth, source);

                    if (false)
                    {
                        median_smooth(static_cast<uint16_t*>(const_cast<void*>(tgt.get_data())),
                            _sandbox[_current_frm_size_pixels].data(), 1);     // Flag that control the filter properties
                    }
                    else
                    {
                        // Spatial smooth with domain trandform filter
                        dxf_smooth(static_cast<uint16_t*>(const_cast<void*>(tgt.get_data())),
                            _sandbox[_current_frm_size_pixels].data());
                    }
                }

                out = composite ? source.allocate_composite_frame({ tgt }) : tgt;
            }
            source.frame_ready(out);
        };

        auto callback = new rs2::frame_processor_callback<decltype(on_frame)>(on_frame);
        processing_block::set_processing_callback(std::shared_ptr<rs2_frame_processor_callback>(callback));
    }

    void  spatial_filter::update_configuration(const rs2::frame& f)
    {
        if (f.get_profile().get() != _target_stream_profile.get())
        {
            _target_stream_profile = f.get_profile().clone(RS2_STREAM_DEPTH, 0, RS2_FORMAT_Z16);
            environment::get_instance().get_extrinsics_graph().register_same_extrinsics(
                *(stream_interface*)(f.get_profile().get()->profile),
                *(stream_interface*)(_target_stream_profile.get()->profile));
            auto vp = _target_stream_profile.as<rs2::video_stream_profile>();
            _width = vp.width();
            _height = vp.height();
            _current_frm_size_pixels = _width * _height;

            // Allocate intermediate results buffer, if needed
            auto it = _sandbox.find(_current_frm_size_pixels);
            if (it == _sandbox.end())
                _sandbox.emplace(_current_frm_size_pixels, std::vector<uint16_t>(_current_frm_size_pixels));
        }
    }

    rs2::frame spatial_filter::prepare_target_frame(const rs2::frame& f, const rs2::frame_source& source)
    {
        // Allocate and copy the content of the original frame to the target
        auto vf = f.as<rs2::video_frame>();
        rs2::frame tgt = source.allocate_video_frame(_target_stream_profile, f,
            vf.get_bytes_per_pixel(),
            vf.get_width(),
            vf.get_height(),
            vf.get_stride_in_bytes(),
            RS2_EXTENSION_DEPTH_FRAME);

        int a = vf.get_width();
        int b = vf.get_height();
        std::cout << a - b << std::endl;
        // TODO - optimize
        memmove(const_cast<void*>(tgt.get_data()), f.get_data(), _current_frm_size_pixels * 2); // Z16-bit specialized
        return tgt;
    }

    bool spatial_filter::median_smooth(uint16_t * frame_data, uint16_t * intermediate_data, int flags)
    {
        if (_patch_size < spatial_patch_min_val || _patch_size > spatial_patch_max_val || _range_from < 0 || _range_to < _range_from)
            return false;

        uint16_t* axl_buf = intermediate_data;
        uint8_t half_size = _patch_size >> 1;
        size_t h = _height - half_size;

        // TODO refactor to stl container for boundary checking
        //std::array<uint16_t, spatial_patch_max_val*spatial_patch_max_val> data;
        uint16_t data[spatial_patch_max_val*spatial_patch_max_val];

        memmove(axl_buf, frame_data, _width * half_size * sizeof(uint16_t));

        uint16_t * out = axl_buf + _width * half_size;
        uint16_t * in = frame_data;

        size_t offset = half_size * _width;
        size_t offset_1 = offset + half_size;
        size_t counter = 0;

        uint16_t * p = nullptr;
        uint16_t * pdata = nullptr;

        if (flags == 1)
        {
            for (int j = half_size; j < h; ++j)
            {
                p = in + offset;

                int i;
                for (i = 0; i < half_size; i++)
                    *out++ = *p++;

                for (; i < _width - half_size; ++i)
                {
                    p = offset_1 + in;
                    if (*p < _range_from || *p > _range_to)
                    {
                        *out++ = *p;
                    }
                    else
                    {
                        pdata = data;
                        p = in;

                        counter = 0;

                        for (uint8_t n = 0; n < _patch_size; ++n)
                        {
                            for (uint8_t m = 0; m < _patch_size; ++m)
                            {
                                if (p[m])
                                {
                                    *pdata++ = p[m];
                                    ++counter;
                                }
                            }

                            p += _width;
                            //pdata += _patch_size; - Ev - memory override
                        }

                        if (counter)
                        {
                            std::nth_element(data, data + (counter / 2), data + counter);
                            //std::sort(data, data + counter);
                            *out++ = data[counter / 2];
                        }
                        else
                            *out++ = 0;
                    }

                    ++in;
                }

                in += half_size;
                for (i = 0; i < half_size; i++)
                    *out++ = in[i];
                in += half_size;
            }
        }
        else if (flags == 2)
        {
            int sum = 0;

            for (int j = half_size; j < h; ++j)
            {
                p = in + offset;

                int i;
                for (i = 0; i < half_size; i++)
                    *out++ = *p++;

                for (; i < _width - half_size; ++i)
                {
                    p = offset_1 + in;
                    if (*p < _range_from || *p > _range_to)
                    {
                        *out++ = *p;
                    }
                    else
                    {
                        pdata = data;
                        p = in;

                        sum = 0;
                        counter = 0;

                        for (int n = 0; n < _patch_size; ++n)
                        {
                            for (int m = 0; m < _patch_size; ++m)
                            {
                                if (p[m])
                                {
                                    sum += p[m];
                                    ++counter;
                                }
                            }

                            p += _width;
                            pdata += _patch_size;
                        }

                        *out++ = (counter ? (uint16_t)(sum / counter) : 0);
                    }

                    ++in;
                }

                in += half_size;
                for (i = 0; i < half_size; i++)
                    *out++ = in[i];
                in += half_size;
            }
        }
        else
        {
            size_t size = _patch_size * _patch_size;

            for (size_t j = half_size; j < h; ++j)
            {
                int i;
                p = in + offset;
                for (i = 0; i < half_size; i++)
                    *out++ = *p++;

                for (; i < _width - half_size; ++i)
                {
                    p = offset_1 + in;
                    if (*p < _range_from || *p > _range_to)
                    {
                        *out++ = *p;
                    }
                    else
                    {
                        pdata = data;
                        p = in;

                        for (uint8_t n = 0; n < _patch_size; ++n)
                        {
                            memmove(pdata, p, _patch_size * sizeof(uint16_t));

                            p += _width;
                            pdata += _patch_size;
                        }

                        std::sort(data, data + size);
                        *out++ = data[size / 2];
                    }

                    ++in;
                }

                in += half_size;
                for (i = 0; i < half_size; i++)
                    *out++ = in[i];
                in += half_size;
            }
        }

        memmove(out, in, _width * half_size * sizeof(uint16_t));

        memmove(frame_data, axl_buf, _width * _height * sizeof(uint16_t));

        return true;
    }

    bool spatial_filter::dxf_smooth(uint16_t * frame_data, uint16_t * intermediate_data, float alpha, float delta, int iterations)
    {
        //m_dxf->zImageToFloat(frame_data, m_buf_float, m_width, m_height);
        //m_dxf->filterRSsimple(m_buf_float, m_buf_float, m_width, m_height, alpha, delta, iterations);
        for (int i = 0; i < iterations; i++)
        {
            recursive_filter_horizontal_v2(frame_data, intermediate_data, alpha, delta);
            recursive_filter_vertical_v2(frame_data, intermediate_data, alpha, delta);
        }
        //m_dxf->floatToZimage(m_buf_float, frame_data, m_width, m_height);
        return true;
    }

    void spatial_filter::recursive_filter_horizontal(uint16_t *frame_data, uint16_t * intermediate_data, float alpha, float deltaZ)
    {
        int32_t v{}, u{};
        static const float z_to_meter = 0.001f;      // TODO Evgeni - retrieve from stream profile
        static const float meter_to_z = 1.f / z_to_meter;      // TODO Evgeni - retrieve from stream profile

        for (v = 0; v < _height;) {
            // left to right
            uint16_t *im = frame_data + v * _width;
            float state = (*im)*z_to_meter;
            float previousInnovation = state;
# ifdef DO_LEFT_TO_RIGHT
            im++;
            float innovation = (*im)*z_to_meter;
            u = _width - 1;
            if (!(*(int*)&previousInnovation > 0))
                goto CurrentlyInvalidLR;
            // else fall through

        CurrentlyValidLR:
            for (;;) {
                if (*(int*)&innovation > 0) {
                    float delta = previousInnovation - innovation;
                    bool smallDifference = delta < deltaZ && delta > -deltaZ;

                    if (smallDifference) {
                        float filtered = innovation * alpha + state * (1.0f - alpha);
                        *im = static_cast<uint16_t>((state = filtered) * meter_to_z);
                    }
                    else {
                        state = innovation;
                    }
                    u--;
                    if (u <= 0)
                        goto DoneLR;
                    previousInnovation = innovation;
                    im += 1;
                    innovation = (*im)*z_to_meter;
                }
                else {  // switch to CurrentlyInvalid state
                    u--;
                    if (u <= 0)
                        goto DoneLR;
                    previousInnovation = innovation;
                    im += 1;
                    innovation = (*im)*z_to_meter;
                    goto CurrentlyInvalidLR;
                }
            }

        CurrentlyInvalidLR:
            for (;;) {
                u--;
                if (u <= 0)
                    goto DoneLR;
                if (*(int*)&innovation > 0) { // switch to CurrentlyValid state
                    previousInnovation = state = innovation;
                    im += 1;
                    innovation = (*im)*z_to_meter;
                    goto CurrentlyValidLR;
                }
                else {
                    im += 1;
                    innovation = (*im)*z_to_meter;
                }
            }
        DoneLR:
# endif
# ifdef DO_RIGHT_TO_LEFT
            // right to left
            im = frame_data + (v + 1) * _width - 2;  // end of row - two pixels
            previousInnovation = state = (im[1]) * z_to_meter;
            u = _width - 1;
            innovation = (*im)*z_to_meter;
            if (!(*(int*)&previousInnovation > 0))
                goto CurrentlyInvalidRL;
            // else fall through
        CurrentlyValidRL:
            for (;;) {
                if (*(int*)&innovation > 0) {
                    float delta = previousInnovation - innovation;
                    bool smallDifference = delta < deltaZ && delta > -deltaZ;

                    if (smallDifference) {
                        float filtered = innovation * alpha + state * (1.0f - alpha);
                        *im = static_cast<uint16_t>((state = filtered) * meter_to_z);
                    }
                    else {
                        state = innovation;
                    }
                    u--;
                    if (u <= 0)
                        goto DoneRL;
                    previousInnovation = innovation;
                    im -= 1;
                    innovation = (*im)*z_to_meter;
                }
                else {  // switch to CurrentlyInvalid state
                    u--;
                    if (u <= 0)
                        goto DoneRL;
                    previousInnovation = innovation;
                    im -= 1;
                    innovation = (*im)*z_to_meter;
                    goto CurrentlyInvalidRL;
                }
            }

        CurrentlyInvalidRL:
            for (;;) {
                u--;
                if (u <= 0)
                    goto DoneRL;
                if (*(int*)&innovation > 0) { // switch to CurrentlyValid state
                    previousInnovation = state = innovation;
                    im -= 1;
                    innovation = (*im)*z_to_meter;
                    goto CurrentlyValidRL;
                }
                else {
                    im -= 1;
                    innovation = (*im)*z_to_meter;
                }
            }
        DoneRL:
            v++;
        }
    }
    void spatial_filter::recursive_filter_vertical(uint16_t *frame_data, uint16_t * intermediate_data, float alpha, float deltaZ)
    {
        int32_t v{}, u{};
        static const float z_to_meter = 0.001f;      // TODO Evgeni - retrieve from stream profile
        static const float meter_to_z = 1.f / z_to_meter;      // TODO Evgeni - retrieve from stream profile

        // we'll do one column at a time, top to bottom, bottom to top, left to right, 

        for (u = 0; u < _width;) {

            uint16_t *im = frame_data + u;
            float state = im[0] * z_to_meter;
            float previousInnovation = state;
# ifdef DO_TOP_TO_BOTTOM
            v = _height - 1;
            im += _width;
            float innovation = (*im)*z_to_meter;

            if (!(*(int*)&previousInnovation > 0))
                goto CurrentlyInvalidTB;
            // else fall through

        CurrentlyValidTB:
            for (;;) {
                if (*(int*)&innovation > 0) {
                    float delta = previousInnovation - innovation;
                    bool smallDifference = delta < deltaZ && delta > -deltaZ;

                    if (smallDifference) {
                        float filtered = innovation * alpha + state * (1.0f - alpha);
                        *im = static_cast<uint16_t>((state = filtered) * meter_to_z);
                    }
                    else {
                        state = innovation;
                    }
                    v--;
                    if (v <= 0)
                        goto DoneTB;
                    previousInnovation = innovation;
                    im += _width;
                    innovation = (*im)*z_to_meter;
                }
                else {  // switch to CurrentlyInvalid state
                    v--;
                    if (v <= 0)
                        goto DoneTB;
                    previousInnovation = innovation;
                    im += _width;
                    innovation = (*im)*z_to_meter;
                    goto CurrentlyInvalidTB;
                }
            }

        CurrentlyInvalidTB:
            for (;;) {
                v--;
                if (v <= 0)
                    goto DoneTB;
                if (*(int*)&innovation > 0) { // switch to CurrentlyValid state
                    previousInnovation = state = innovation;
                    im += _width;
                    innovation = (*im)*z_to_meter;
                    goto CurrentlyValidTB;
                }
                else {
                    im += _width;
                    innovation = (*im)*z_to_meter;
                }
            }
        DoneTB:
# endif
# ifdef DO_BOTTOM_TO_TOP
            im = frame_data + u + (_height - 2) * _width;
            state = (im[_width]) * z_to_meter;
            previousInnovation = state;
            innovation = (*im)*z_to_meter;
            v = _height - 1;
            if (!(*(int*)&previousInnovation > 0))
                goto CurrentlyInvalidBT;
            // else fall through
        CurrentlyValidBT:
            for (;;) {
                if (*(int*)&innovation > 0) {
                    float delta = previousInnovation - innovation;
                    bool smallDifference = delta < deltaZ && delta > -deltaZ;

                    if (smallDifference) {
                        float filtered = innovation * alpha + state * (1.0f - alpha);
                        *im = static_cast<uint16_t>((state = filtered) * meter_to_z);
                    }
                    else {
                        state = innovation;
                    }
                    v--;
                    if (v <= 0)
                        goto DoneBT;
                    previousInnovation = innovation;
                    im -= _width;
                    innovation = (*im)*z_to_meter;
                }
                else {  // switch to CurrentlyInvalid state
                    v--;
                    if (v <= 0)
                        goto DoneBT;
                    previousInnovation = innovation;
                    im -= _width;
                    innovation = (*im)*z_to_meter;
                    goto CurrentlyInvalidBT;
                }
            }

        CurrentlyInvalidBT:
            for (;;) {
                v--;
                if (v <= 0)
                    goto DoneBT;
                if (*(int*)&innovation > 0) { // switch to CurrentlyValid state
                    previousInnovation = state = innovation;
                    im -= _width;
                    innovation = (*im)*z_to_meter;
                    goto CurrentlyValidBT;
                }
                else {
                    im -= _width;
                    innovation = (*im)*z_to_meter;
                }
            }
        DoneBT:
            u++;
# endif
        }
    }
#endif

    void  spatial_filter::recursive_filter_horizontal_v2(uint16_t *image, uint16_t * intermediate_data, float alpha, float deltaZ)
    {
        int32_t v{}, u{};
        //static const float z_to_meter = 0.001f;      // TODO Evgeni - retrieve from stream profile
        //static const float meter_to_z = 1.f / z_to_meter;      // TODO Evgeni - retrieve from stream profile

        for (v = 0; v < _height; v++) {
            // left to right
            unsigned short *im = image + v * _width;
            unsigned short val0 = im[0];
            for (u = 1; u < _width; u++) {
                unsigned short val1 = im[1];
                int delta = val0 - val1;
                if (delta < deltaZ && delta > -deltaZ) {
                    float filtered = val1 * alpha + val0 * (1.0f - alpha);
                    val0 = (unsigned short)(filtered + 0.5f);
                    im[1] = val0;
                }
                im += 1;
            }

            // right to left
            im = image + (v + 1) * _width - 2;  // end of row - two pixels
            unsigned short val1 = im[1];
            for (u = _width - 1; u > 0; u--) {
                unsigned short val0 = im[0];
                int delta = val0 - val1;
                if (delta < deltaZ && delta > -deltaZ) {
                    float filtered = val0 * alpha + val1 * (1.0f - alpha);
                    val1 = (unsigned short)(filtered + 0.5f);
                    im[0] = val1;
                }
                im -= 1;
            }
        }
    }
    void spatial_filter::recursive_filter_vertical_v2(uint16_t *image, uint16_t * intermediate_data, float alpha, float deltaZ)
    {
        int32_t v{}, u{};

        // we'll do one row at a time, top to bottom, then bottom to top

        // top to bottom

        unsigned short *im = image;
        for (v = 1; v < _height; v++) {
            for (u = 0; u < _width; u++) {
                unsigned short im0 = im[0];
                unsigned short imw = im[_width];

                if (im0 && imw) {
                    int delta = im0 - imw;
                    if (delta < deltaZ && delta > -deltaZ) {
                        float filtered = imw * alpha + im0 * (1.0f - alpha);
                        im[_width] = (unsigned short)(filtered + 0.5f);
                    }
                }
                im += 1;
            }
        }

        // bottom to top
        im = image + (_height - 2) * _width;
        for (v = 1; v < _height; v++, im -= (_width * 2)) {
            for (u = 0; u < _width; u++) {
                unsigned short  im0 = im[0];
                unsigned short  imw = im[_width];

                if (im0 && imw) {
                    int delta = im0 - imw;
                    if (delta < deltaZ && delta > -deltaZ) {
                        float filtered = im0 * alpha + imw * (1.0f - alpha);
                        im[0] = (unsigned short)(filtered + 0.5f);
                    }
                }
                im += 1;
            }
        }
    }

}
