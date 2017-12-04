// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2015 Intel Corporation. All Rights Reserved.

#include "../include/librealsense2/rs.hpp"
#include "../include/librealsense2/rsutil.h"

#include "core/video.h"
#include "align.h"
#include "archive.h"
#include "context.h"
#include "environment.h"

namespace librealsense
{
    template<class MAP_DEPTH> void deproject_depth(float * points, const rs2_intrinsics & intrin, const uint16_t * depth, MAP_DEPTH map_depth)
    {
        for (int y = 0; y<intrin.height; ++y)
        {
            for (int x = 0; x<intrin.width; ++x)
            {
                const float pixel[] = { (float)x, (float)y };
                rs2_deproject_pixel_to_point(points, &intrin, pixel, map_depth(*depth++));
                points += 3;
            }
        }
    }

    const float3 * depth_to_points(uint8_t* image, const rs2_intrinsics &depth_intrinsics, const uint16_t * depth_image, float depth_scale)
    {
        deproject_depth(reinterpret_cast<float *>(image), depth_intrinsics, depth_image, [depth_scale](uint16_t z) { return depth_scale * z; });

        return reinterpret_cast<float3 *>(image);
    }

    float3 transform(const rs2_extrinsics *extrin, const float3 &point) { float3 p = {}; rs2_transform_point_to_point(&p.x, extrin, &point.x); return p; }
    float2 project(const rs2_intrinsics *intrin, const float3 & point) { float2 pixel = {}; rs2_project_point_to_pixel(&pixel.x, intrin, &point.x); return pixel; }
    float2 pixel_to_texcoord(const rs2_intrinsics *intrin, const float2 & pixel) { return{ (pixel.x + 0.5f) / intrin->width, (pixel.y + 0.5f) / intrin->height }; }
    float2 project_to_texcoord(const rs2_intrinsics *intrin, const float3 & point) { return pixel_to_texcoord(intrin, project(intrin, point)); }

    void processing_block::set_processing_callback(frame_processor_callback_ptr callback)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _callback = callback;
    }

    void processing_block::set_output_callback(frame_callback_ptr callback)
    {
        _source.set_callback(callback);
    }

    processing_block::processing_block()
        : _source_wrapper(_source)
    {
        register_option(RS2_OPTION_FRAMES_QUEUE_SIZE, _source.get_published_size_option());
        _source.init(std::make_shared<metadata_parser_map>());
    }

    void processing_block::invoke(frame_holder f)
    {
        auto callback = _source.begin_callback();
        try
        {
            if (_callback)
            {
                frame_interface* ptr = nullptr;
                std::swap(f.frame, ptr);

                _callback->on_frame((rs2_frame*)ptr, _source_wrapper.get_c_wrapper());
            }
        }
        catch(...)
        {
            LOG_ERROR("Exception was thrown during user processing callback!");
        }
    }

    void synthetic_source::frame_ready(frame_holder result)
    {
        _actual_source.invoke_callback(std::move(result));
    }

    frame_interface* synthetic_source::allocate_points(std::shared_ptr<stream_profile_interface> stream, frame_interface* original)
    {
        auto vid_stream = dynamic_cast<video_stream_profile_interface*>(stream.get());
        if (vid_stream)
        {
            frame_additional_data data{};
            data.frame_number = original->get_frame_number();
            data.timestamp = original->get_frame_timestamp();
            data.timestamp_domain = original->get_frame_timestamp_domain();
            data.metadata_size = 0;
            data.system_time = _actual_source.get_time();

            auto res = _actual_source.alloc_frame(RS2_EXTENSION_POINTS, vid_stream->get_width() * vid_stream->get_height() * sizeof(float) * 5, data, true);
            if (!res) throw wrong_api_call_sequence_exception("Out of frame resources!");
            res->set_sensor(original->get_sensor());
            res->set_stream(stream);
            return res;
        }
        return nullptr;
    }

    frame_interface* synthetic_source::allocate_video_frame(std::shared_ptr<stream_profile_interface> stream,
                                                            frame_interface* original,
                                                            int new_bpp,
                                                            int new_width,
                                                            int new_height,
                                                            int new_stride,
                                                            rs2_extension frame_type)
    {
        video_frame* vf = nullptr;

        if (new_bpp == 0 || (new_width == 0 && new_stride == 0) || new_height == 0)
        {
            // If the user wants to delegate width, height and etc to original frame, it must be a video frame
            if (!rs2_is_frame_extendable_to((rs2_frame*)original, RS2_EXTENSION_VIDEO_FRAME, nullptr))
            {
                throw std::runtime_error("If original frame is not video frame, you must specify new bpp, width/stide and height!");
            }
            vf = static_cast<video_frame*>(original);
        }

        frame_additional_data data{};
        data.frame_number = original->get_frame_number();
        data.timestamp = original->get_frame_timestamp();
        data.timestamp_domain = original->get_frame_timestamp_domain();
        data.metadata_size = 0;
        data.system_time = _actual_source.get_time();

        auto width = new_width;
        auto height = new_height;
        auto bpp = new_bpp * 8;
        auto stride = new_stride;

        if (bpp == 0)
        {
            bpp = vf->get_bpp();
        }

        if (width == 0 && stride == 0)
        {
            width = vf->get_width();
            stride = width * bpp / 8;
        }
        else if (width == 0)
        {
            width = stride * 8 / bpp;
        }
        else if (stride == 0)
        {
            stride = width * bpp / 8;
        }

        if (height == 0)
        {
            height = vf->get_height();
        }

        auto res = _actual_source.alloc_frame(frame_type, stride * height, data, true);
        if (!res) throw wrong_api_call_sequence_exception("Out of frame resources!");
        vf = static_cast<video_frame*>(res);
        vf->assign(width, height, stride, bpp);
        vf->set_sensor(original->get_sensor());
        res->set_stream(stream);

        if (frame_type == RS2_EXTENSION_DEPTH_FRAME)
        {
            original->acquire();
            (dynamic_cast<depth_frame*>(res))->set_original(original);
        }

        return res;
    }

    int get_embeded_frames_size(frame_interface* f)
    {
        if (f == nullptr) return 0;
        if (auto c = dynamic_cast<composite_frame*>(f))
            return static_cast<int>(c->get_embedded_frames_count());
        return 1;
    }

    void copy_frames(frame_holder from, frame_interface**& target)
    {
        if (auto comp = dynamic_cast<composite_frame*>(from.frame))
        {
            auto frame_buff = comp->get_frames();
            for (size_t i = 0; i < comp->get_embedded_frames_count(); i++)
            {
                std::swap(*target, frame_buff[i]);
                target++;
            }
            from.frame->disable_continuation();
        }
        else
        {
            *target = nullptr; // "move" the frame ref into target
            std::swap(*target, from.frame);
            target++;
        }
    }

    frame_interface* synthetic_source::allocate_composite_frame(std::vector<frame_holder> holders)
    {
        frame_additional_data d {};

        auto req_size = 0;
        for (auto&& f : holders)
            req_size += get_embeded_frames_size(f.frame);

        auto res = _actual_source.alloc_frame(RS2_EXTENSION_COMPOSITE_FRAME, req_size * sizeof(rs2_frame*), d, true);
        if (!res) return nullptr;

        auto cf = static_cast<composite_frame*>(res);

        auto frames = cf->get_frames();
        for (auto&& f : holders)
            copy_frames(std::move(f), frames);
        frames -= req_size;

        auto releaser = [frames, req_size]()
        {
            for (auto i = 0; i < req_size; i++)
            {
                frames[i]->release();
                frames[i] = nullptr;
            }
        };
        frame_continuation release_frames(releaser, nullptr);
        cf->attach_continuation(std::move(release_frames));
        cf->set_stream(cf->first()->get_stream());

        return res;
    }

    pointcloud::pointcloud()
          :_depth_intrinsics_ptr(nullptr),
          _depth_units_ptr(nullptr),
          _mapped_intrinsics_ptr(nullptr),
          _extrinsics_ptr(nullptr),
          _mapped(nullptr)
    {
        auto on_frame = [this](rs2::frame f, const rs2::frame_source& source)
        {
            auto inspect_depth_frame = [this](const rs2::frame& depth)
            {
                auto depth_frame = (frame_interface*)depth.get();
                std::lock_guard<std::mutex> lock(_mutex);

                if (!_stream.get() || _depth_stream != depth_frame->get_stream().get())
                {
                    _stream = depth_frame->get_stream()->clone();
                    _depth_stream = depth_frame->get_stream().get();
                    environment::get_instance().get_extrinsics_graph().register_same_extrinsics(*_stream, *depth_frame->get_stream());
                    _depth_intrinsics_ptr = nullptr;
                    _depth_units_ptr = nullptr;
                }

                bool found_depth_intrinsics = false;
                bool found_depth_units = false;

                if (!_depth_intrinsics_ptr)
                {
                    auto stream_profile = depth_frame->get_stream();
                    if (auto video = dynamic_cast<video_stream_profile_interface*>(stream_profile.get()))
                    {
                        _depth_intrinsics = video->get_intrinsics();
                        _depth_intrinsics_ptr = &_depth_intrinsics;
                        found_depth_intrinsics = true;
                    }
                }

                if (!_depth_units_ptr)
                {
                    auto sensor = depth_frame->get_sensor();
                    _depth_units = sensor->get_option(RS2_OPTION_DEPTH_UNITS).query();
                    _depth_units_ptr = &_depth_units;
                    found_depth_units = true;
                }

                if (found_depth_units != found_depth_intrinsics)
                {
                    throw wrong_api_call_sequence_exception("Received depth frame that doesn't provide either intrinsics or depth units!");
                }
            };

            auto inspect_other_frame = [this](const rs2::frame& other)
            {
                auto other_frame = (frame_interface*)other.get();
                std::lock_guard<std::mutex> lock(_mutex);

                if (_mapped.get() != other_frame->get_stream().get())
                {
                    _mapped = other_frame->get_stream();
                    _mapped_intrinsics_ptr = nullptr;
                    _extrinsics_ptr = nullptr;
                }

                if (!_mapped_intrinsics_ptr)
                {
                    if (auto video = dynamic_cast<video_stream_profile_interface*>(_mapped.get()))
                    {
                        _mapped_intrinsics = video->get_intrinsics();
                        _mapped_intrinsics_ptr = &_mapped_intrinsics;
                    }
                }

                if (_stream && !_extrinsics_ptr)
                {
                    if ( environment::get_instance().get_extrinsics_graph().try_fetch_extrinsics(
                        *_stream, *other_frame->get_stream(), &_extrinsics
                    ))
                    {
                        _extrinsics_ptr = &_extrinsics;
                    }
                }
            };

            auto process_depth_frame = [this](const rs2::depth_frame& depth)
            {
                frame_holder res = get_source().allocate_points(_stream, (frame_interface*)depth.get());

                auto pframe = (points*)(res.frame);

                auto depth_data = (const uint16_t*)depth.get_data();
                //auto original_depth = ((depth_frame*)depth.get())->get_original_depth();
                //if (original_depth) depth_data = (const uint16_t*)original_depth->get_frame_data();

                auto points = depth_to_points((uint8_t*)pframe->get_vertices(), *_depth_intrinsics_ptr, depth_data, *_depth_units_ptr);

                auto vid_frame = depth.as<rs2::video_frame>();
                float2* tex_ptr = pframe->get_texture_coordinates();

                rs2_intrinsics mapped_intr;
                rs2_extrinsics extr;
                bool map_texture = false;
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    if (_extrinsics_ptr && _mapped_intrinsics_ptr)
                    {
                        mapped_intr = *_mapped_intrinsics_ptr;
                        extr = *_extrinsics_ptr;
                        map_texture = true;
                    }
                }

                if (map_texture)
                {
                    auto height = vid_frame.get_height();
                    auto width = vid_frame.get_width();

                    for (int y = 0; y < height; ++y)
                    {
                        for (int x = 0; x < width; ++x)
                        {
                            if (points->z)
                            {
                                auto trans = transform(&extr, *points);
                                auto tex_xy = project_to_texcoord(&mapped_intr, trans);

                                *tex_ptr = tex_xy;
                            }
                            else
                            {
                                *tex_ptr = { 0.f, 0.f };
                            }
                            ++points;
                            ++tex_ptr;
                        }
                    }
                }

                get_source().frame_ready(std::move(res));
            };

            if (auto composite = f.as<rs2::frameset>())
            {
                auto depth = composite.first_or_default(RS2_STREAM_DEPTH);
                if (depth)
                {
                    auto id = depth.get_profile().unique_id();
                    inspect_depth_frame(depth);
                    process_depth_frame(depth);
                }
                else
                {
                    composite.foreach(inspect_other_frame);
                }
            }
            else
            {
                if (f.get_profile().stream_type() == RS2_STREAM_DEPTH)
                {
                    auto id = f.get_profile().unique_id();
                    inspect_depth_frame(f);
                    process_depth_frame(f);
                }
                else
                {
                    inspect_other_frame(f);
                }
            }
        };
        auto callback = new rs2::frame_processor_callback<decltype(on_frame)>(on_frame);
        processing_block::set_processing_callback(std::shared_ptr<rs2_frame_processor_callback>(callback));
    }

    //void colorize::set_color_map(rs2_color_map cm)
    //{
    //    std::lock_guard<std::mutex> lock(_mutex);
    //    switch(cm)
    //    {
    //    case RS2_COLOR_MAP_CLASSIC:
    //        _cm = &classic;
    //        break;
    //    case RS2_COLOR_MAP_JET:
    //        _cm = &jet;
    //        break;
    //    case RS2_COLOR_MAP_HSV:
    //        _cm = &hsv;
    //        break;
    //    default:
    //        _cm = &classic;
    //    }
    //}

    //void colorize::histogram_equalization(bool enable)
    //{
    //    std::lock_guard<std::mutex> lock(_mutex);
    //    _equalize = enable;
    //}

    //colorize::colorize(std::shared_ptr<uvc::time_service> ts)
    //    : processing_block(RS2_EXTENSION_VIDEO_FRAME, ts), _cm(&classic), _equalize(true)
    //{
    //    auto on_frame = [this](std::vector<rs2::frame> frames, const rs2::frame_source& source)
    //    {
    //        std::lock_guard<std::mutex> lock(_mutex);

    //        for (auto&& f : frames)
    //        {
    //            if (f.get_stream_type() == RS2_STREAM_DEPTH)
    //            {
    //                const auto max_depth = 0x10000;

    //                static uint32_t histogram[max_depth];
    //                memset(histogram, 0, sizeof(histogram));

    //                auto vf = f.as<video_frame>();
    //                auto width = vf.get_width();
    //                auto height = vf.get_height();

    //                auto depth_image = vf.get_frame_data();

    //                for (auto i = 0; i < width*height; ++i) ++histogram[depth_image[i]];
    //                for (auto i = 2; i < max_depth; ++i) histogram[i] += histogram[i - 1]; // Build a cumulative histogram for the indices in [1,0xFFFF]
    //                for (auto i = 0; i < width*height; ++i)
    //                {
    //                    auto d = depth_image[i];

    //                    //if (d)
    //                    //{
    //                    //    auto f = histogram[d] / (float)histogram[0xFFFF]; // 0-255 based on histogram location

    //                    //    auto c = map.get(f);
    //                    //    rgb_image[i * 3 + 0] = c.x;
    //                    //    rgb_image[i * 3 + 1] = c.y;
    //                    //    rgb_image[i * 3 + 2] = c.z;
    //                    //}
    //                    //else
    //                    //{
    //                    //    rgb_image[i * 3 + 0] = 0;
    //                    //    rgb_image[i * 3 + 1] = 0;
    //                    //    rgb_image[i * 3 + 2] = 0;
    //                    //}
    //                }
    //            }
    //        }
    //    };
    //    auto callback = new rs2::frame_processor_callback<decltype(on_frame)>(on_frame);
    //    set_processing_callback(std::shared_ptr<rs2_frame_processor_callback>(callback));
    //}

    template<class GET_DEPTH, class TRANSFER_PIXEL>
    void align_images(const rs2_intrinsics & from_intrin, const rs2_extrinsics & extrinsic,
        const rs2_intrinsics & to_intrin, GET_DEPTH get_depth, TRANSFER_PIXEL transfer_pixel)
    {
        // Iterate over the pixels of the depth image
        #pragma omp parallel for schedule(dynamic)
        for (int y = 0; y < from_intrin.height; ++y)
        {
            int pixel_index = y * from_intrin.width;
            for (int x = 0; x < from_intrin.width; ++x, ++pixel_index)
            {
                // Skip over depth pixels with the value of zero, we have no depth data so we will not write anything into our aligned images
                if (float depth = get_depth(pixel_index))
                {
                    // Map the top-left corner of the depth pixel onto the other image
                    float from_pixel[2] = { x - 0.5f, y - 0.5f };
                    float from_point[3];
                    float to_point[3];
                    float to_pixel[2];
                    rs2_deproject_pixel_to_point(from_point, &from_intrin, from_pixel, depth);
                    rs2_transform_point_to_point(to_point, &extrinsic, from_point);
                    rs2_project_point_to_pixel(to_pixel, &to_intrin, to_point);
                    const int to_x0 = static_cast<int>(to_pixel[0] + 0.5f);
                    const int to_y0 = static_cast<int>(to_pixel[1] + 0.5f);

                    // Map the bottom-right corner of the pixel onto the other image
                    from_pixel[0] = x + 0.5f; from_pixel[1] = y + 0.5f;
                    rs2_deproject_pixel_to_point(from_point, &from_intrin, from_pixel, depth);
                    rs2_transform_point_to_point(to_point, &extrinsic, from_point);
                    rs2_project_point_to_pixel(to_pixel, &to_intrin, to_point);
                    const int to_x1 = static_cast<int>(to_pixel[0] + 0.5f);
                    const int to_y1 = static_cast<int>(to_pixel[1] + 0.5f);

                    if (to_x0 < 0 || to_y0 < 0 || to_x1 >= to_intrin.width || to_y1 >= to_intrin.height)
                        continue;

                    // Transfer between the pixels and the pixels inside the rectangle on the other image
                    for (int y = to_y0; y <= to_y1; ++y)
                    {
                        for (int x = to_x0; x <= to_x1; ++x)
                        {
                            transfer_pixel(pixel_index, y * to_intrin.width + x);
                        }
                    }
                }
            }
        }
    }

    void align::update_frame_info(const frame_interface* frame, optional_value<rs2_intrinsics>& intrin,
        std::shared_ptr<stream_profile_interface>& profile, bool register_extrin)
    {
        if (!frame)
            return;

        // Get profile
        if (!profile)
        {
            profile = frame->get_stream();
            if(register_extrin)
                environment::get_instance().get_extrinsics_graph().register_same_extrinsics(*profile, *profile);
        }

        // Get intrinsics
        if (!intrin)
        {
            if (auto video = As<video_stream_profile_interface>(profile))
            {
                intrin = video->get_intrinsics();
            }
        }
    }
    void align::update_align_info(const frame_interface* depth_frame)
    {
        // Get depth units
        if (!_depth_units)
        {
            auto sensor = depth_frame->get_sensor();
            _depth_units = sensor->get_option(RS2_OPTION_DEPTH_UNITS).query();
        }

        // Get extrinsics
        if (!_extrinsics && _from_stream_profile && _to_stream_profile)
        {
            rs2_extrinsics ex;
            if (environment::get_instance().get_extrinsics_graph().try_fetch_extrinsics(*_from_stream_profile, *_to_stream_profile, &ex))
            {
                _extrinsics = ex;
            }
        }
    }
    

    align::align(rs2_stream to_stream)
    {
        auto on_frame = [this, to_stream](rs2::frame f, const rs2::frame_source& source)
        {
            auto composite = f.as<rs2::frameset>();
            if (!composite)
                return;

            std::unique_lock<std::mutex> lock(_mutex);

            assert(composite.size() >= 2);
            if (!_from_stream_type)
            {
                _from_stream_type = RS2_STREAM_DEPTH;
                _to_stream_type = to_stream;
                if (to_stream == RS2_STREAM_DEPTH) //Align other to depth
                {
                    for (auto&& f : composite)
                    {
                        rs2_stream other_stream_type = f.get_profile().stream_type();
                        if (other_stream_type != RS2_STREAM_DEPTH)
                        {
                            _from_stream_type = other_stream_type;
                            break; //Take first matching stream type that is not depth
                        }
                    }
                    if (!_from_stream_type)
                        _from_stream_type = RS2_STREAM_DEPTH; //If we only have depth frames, align them to one another
                }
            }

            rs2::frame from = composite.first_or_default(*_from_stream_type);
            rs2::depth_frame depth_frame = composite.get_depth_frame();
            rs2::frame to = composite.first_or_default(_to_stream_type);

            // align_frames(source, from, to)
            update_frame_info((frame_interface*)from.get(), _from_intrinsics, _from_stream_profile, false);
            update_frame_info((frame_interface*)to.get(), _to_intrinsics, _to_stream_profile, true);
            update_align_info((frame_interface*)depth_frame.get());

            if (!_to_bytes_per_pixel)
            {
                auto vid_frame = to.as<rs2::video_frame>();
                _to_bytes_per_pixel = vid_frame.get_bytes_per_pixel();
            }

            if (_from_intrinsics && _to_intrinsics && _extrinsics && _depth_units && _from_stream_profile && _to_stream_profile)
            {
                std::vector<frame_holder> frames(2);
                bool from_depth = (*_from_stream_type == RS2_STREAM_DEPTH);

                // Save the target ("to") frame as is
                auto to_frame = (frame_interface*)to.get();
                to_frame->acquire();
                frames[0] = frame_holder{ to_frame };
                
                auto from_frame = (frame_interface*)from.get();

                // Create a new frame which will transform the "from" frame
                frame_holder out_frame = get_source().allocate_video_frame(_from_stream_profile, from_frame,
                    _to_bytes_per_pixel.value(), _to_intrinsics->width, _to_intrinsics->height, 0, from_depth ? RS2_EXTENSION_DEPTH_FRAME : RS2_EXTENSION_VIDEO_FRAME);

                //Clear the new image buffer
                auto p_out_frame = reinterpret_cast<uint16_t*>(((frame*)(out_frame.frame))->data.data());
                int blank_color = (_from_stream_profile->get_format() == RS2_FORMAT_DISPARITY16) ? 0xFF : 0x00;
                memset(p_out_frame, blank_color, _to_intrinsics->height * _to_intrinsics->width * _to_bytes_per_pixel.value());

                auto p_depth_frame = reinterpret_cast<const uint16_t*>(depth_frame.get_data());
                auto p_from_frame = reinterpret_cast<const uint16_t*>(from.get_data());
                
                lock.unlock();
                float depth_units = _depth_units.value();
                align_images(*_from_intrinsics, *_extrinsics, *_to_intrinsics, 
                    [p_depth_frame, depth_units, from_depth](int z_pixel_index) -> float
                {
                    if (from_depth)
                    {
                        return depth_units * p_depth_frame[z_pixel_index];
                    }
                    return 1;
                },
                    [p_out_frame, p_from_frame](int from_pixel_index, int out_pixel_index)
                {
                    p_out_frame[out_pixel_index] = p_out_frame[out_pixel_index] ?
                        std::min((int)(p_out_frame[out_pixel_index]), (int)(p_from_frame[from_pixel_index]))
                        : p_from_frame[from_pixel_index];
                });
                frames[1] = std::move(out_frame);
                auto composite = get_source().allocate_composite_frame(std::move(frames));
                get_source().frame_ready(std::move(composite));
            }
        };
        auto callback = new rs2::frame_processor_callback<decltype(on_frame)>(on_frame);
        processing_block::set_processing_callback(std::shared_ptr<rs2_frame_processor_callback>(callback));
    }
}
