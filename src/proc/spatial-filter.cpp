// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.
#include "../include/librealsense2/rs.hpp"

#include "option.h"
#include "context.h"
#include "proc/synthetic-stream.h"
#include "proc/spatial-filter.h"

void downsample_depth(const uint16_t * frame_data_in, uint16_t * frame_data_out, size_t width_in, size_t height_in, size_t scale);

namespace librealsense
{
    spatial_filter::spatial_filter():
        _filter_type(rs2_median),
        _spatial_param(4),
        _kernel_size(_spatial_param*_spatial_param),
        _width(0), _height(0)
    {
        auto spatial_filter_control = std::make_shared<ptr_option<uint8_t>>(1, 5, 1, 4, &_spatial_param, "Spatial magnitude");
        spatial_filter_control->on_set([this](float val)
        {
            _kernel_size = 0x1 << uint8_t(val-1);
            _kernel_size *= _kernel_size;
        });
        register_option(RS2_OPTION_FILTER_MAGNITUDE, spatial_filter_control);
        unregister_option(RS2_OPTION_FRAMES_QUEUE_SIZE);

        // TODO - a candidate to base class refactoring
        auto enable_control = std::make_shared<ptr_option<bool>>(false,true,true,true, &_enable_filter, "Apply spatial filter");
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
                    auto fi = (frame_interface*)f.get();
                    auto vf = f.as<rs2::video_frame>();
                    auto tgt =  source.allocate_video_frame(_target_stream_profile, f,
                        vf.get_bytes_per_pixel(), vf.get_width(), vf.get_height(), vf.get_stride_in_bytes(), RS2_EXTENSION_DEPTH_FRAME);

                    auto src = depth.as<rs2::video_frame>();
                        downsample_depth(static_cast<const uint16_t*>(src.get_data()),
                            static_cast<uint16_t*>(const_cast<void*>(tgt.get_data())),
                            src.get_width(), src.get_height(), this->_spatial_param);
                }

                out = composite ? source.allocate_composite_frame({ tgt }) : tgt;
            }

            source.frame_ready(out);
        };

        auto callback = new rs2::frame_processor_callback<decltype(on_frame)>(on_frame);
        processing_block::set_processing_callback(std::shared_ptr<rs2_frame_processor_callback>(callback));
    }

}

void downsample_depth(const uint16_t * frame_data_in, uint16_t * frame_data_out,
    size_t width_in, size_t height_in, size_t scale)
{
    // The frame size must be a multiple of the filter's kernel unit
    assert(0 == (width_in%scale));
    assert(0 == (height_in%scale));

    auto width_out = width_in / scale;
    auto height_out = height_in / scale;
    auto kernel_size = scale * scale;

    // Use median filtering
    std::vector<uint16_t> working_kernel(kernel_size);
    auto wk_begin = working_kernel.data();
    std::vector<uint16_t*> pixel_raws(scale);
    uint16_t* block_start = const_cast<uint16_t*>(frame_data_in);

    //#pragma omp parallel for schedule(dynamic) //Using OpenMP to try to parallelise the loop
    //TODO Evgeni
    for (int j = 0; j < height_out; j++)
    {
        // Mark the beginning of each of the N lines that the filter will run upon
        for (size_t i = 0; i < pixel_raws.size(); i++)
            pixel_raws[i] = block_start + (width_in*i);

        for (size_t i = 0, chunk_offset=0; i < width_out; i++)
        {
            // extract data the kernel to prrocess
            for (size_t n = 0; n < scale; ++n)
            {
                for (size_t m = 0; m < scale; ++m)
                    working_kernel[n*scale+m] = *(pixel_raws[n]+ chunk_offset +m);
            }

            std::nth_element(wk_begin, wk_begin + (kernel_size / 2), wk_begin + kernel_size);
            //std::sort(working_kernel.begin(),working_kernel.end());
            *frame_data_out++ = working_kernel[kernel_size / 2];

            chunk_offset += scale;
        }

        // Skip N lines to the beginnig of the next processing segment
        block_start += width_in*scale;
    }
}
