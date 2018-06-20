#include "MatlabParamParser.h"
#include "Factory.h"
#include "librealsense2/rs.hpp"

#pragma comment(lib, "libmx.lib")
#pragma comment(lib, "libmex.lib")
#pragma comment(lib, "libmat.lib")
#pragma comment(lib, "realsense2.lib")

Factory *factory;

void make_factory(){
    factory = new Factory();

    // TODO: missing destructors

    // rs_frame.hpp
    {
        ClassFactory stream_profile_factory("rs2::stream_profile");
        stream_profile_factory.record("new", 1, 0, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            outv[0] = MatlabParamParser::wrap(rs2::stream_profile());
        });
        stream_profile_factory.record("delete", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            MatlabParamParser::destroy<rs2::stream_profile>(inv[0]);
        });
        stream_profile_factory.record("stream_index", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::stream_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.stream_index());
        });
        stream_profile_factory.record("stream_type", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::stream_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.stream_type());
        });
        stream_profile_factory.record("format", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::stream_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.format());
        });
        stream_profile_factory.record("fps", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::stream_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.fps());
        });
        stream_profile_factory.record("unique_id", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::stream_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.unique_id());
        });
        stream_profile_factory.record("clone", 1, 4, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::stream_profile>(inv[0]);
            auto type = MatlabParamParser::parse<rs2_stream>(inv[1]);
            auto index = MatlabParamParser::parse<int>(inv[2]);
            auto format = MatlabParamParser::parse<rs2_format>(inv[3]);
            outv[0] = MatlabParamParser::wrap(thiz.clone(type, index, format));
        });
        stream_profile_factory.record("operator==", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto rhs = MatlabParamParser::parse<rs2::stream_profile>(inv[0]);
            auto lhs = MatlabParamParser::parse<rs2::stream_profile>(inv[1]);
            MatlabParamParser::wrap(rhs == lhs);
        });
        // rs2::stream_profile::is                                      [TODO] [T = {stream_profile, video_stream_profile, motion_stream_profile}]
        // rs2::stream_profile::as                                      [TODO] [T = {stream_profile, video_stream_profile, motion_stream_profile}]
        stream_profile_factory.record("stream_name", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::stream_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.stream_name());
        });
        stream_profile_factory.record("is_default", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::stream_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.is_default());
        });
        // rs2::stream_profile::operator bool                           [?]
        stream_profile_factory.record("get_extrinsics_to", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::stream_profile>(inv[0]);
            auto to = MatlabParamParser::parse<rs2::stream_profile>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.get_extrinsics_to(to));
        });
        // rs2::stream_profile::register_extrinsics_to                  [?]
        stream_profile_factory.record("is_cloned", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::stream_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.is_cloned());
        });
        factory->record(stream_profile_factory);
    }
    {
        ClassFactory video_stream_profile_factory("rs2::video_stream_profile");
        // rs2::video_stream_profile::constructor(rs2::stream_profile)  [?]
        video_stream_profile_factory.record("width", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::video_stream_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.width());
        });
        video_stream_profile_factory.record("height", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::video_stream_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.height());
        });
        video_stream_profile_factory.record("get_intrinsics", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::video_stream_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_intrinsics());
        });
        factory->record(video_stream_profile_factory);
    }
    {
        ClassFactory motion_stream_profile_factory("rs2::motion_stream_profile");
        // rs2::motion_stream_profile::constructor(rs2::stream_profile) [?]
        motion_stream_profile_factory.record("get_motion_intrinsics", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::motion_stream_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_motion_intrinsics());
        });
        factory->record(motion_stream_profile_factory);
    }
    {
        ClassFactory frame_factory("rs2::frame");
        // rs2::frame::constructor()                                    [?]
        frame_factory.record("delete", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            MatlabParamParser::destroy<rs2::frame>(inv[0]);
        });
        // rs2::frame::operator=                                        [?/HOW]
        // rs2::frame::copy constructor                                 [?/HOW]
        // rs2::frame::swap                                             [?/HOW]
        // rs2::frame::keep                                             [TODO/HOW]
        // rs2::frame::operator bool                                    [?]
        frame_factory.record("get_timestamp", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frame>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_timestamp());
        });
        frame_factory.record("get_frame_timestamp_domain", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frame>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_frame_timestamp_domain());
        });
        frame_factory.record("get_frame_metadata", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frame>(inv[0]);
            auto frame_metadata = MatlabParamParser::parse<rs2_frame_metadata_value>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.get_frame_metadata(frame_metadata));
        });
        frame_factory.record("supports_frame_metadata", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frame>(inv[0]);
            auto frame_metadata = MatlabParamParser::parse<rs2_frame_metadata_value>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.supports_frame_metadata(frame_metadata));
        });
        frame_factory.record("get_frame_number", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frame>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_frame_number());
        });
        frame_factory.record("get_data", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            // TODO - What's going on here? / support more formats
            auto thiz = MatlabParamParser::parse<rs2::frame>(inv[0]);
            if (auto vf = thiz.as<rs2::video_frame>()) {
                /*switch (vf.get_profile().format()) {
                case RS2_FORMAT_RGB8: case RS2_FORMAT_BRG2:
                outv[0] = MatlabParamParser::wrap_array<uint8_t>(vf.get_data(), )
                }*/
                // TODO: turn into matrix instead of column
                size_t dims[] = { static_cast<size_t>(vf.get_height() * vf.get_stride_in_bytes()) };
                outv[0] = MatlabParamParser::wrap_array<uint8_t>(reinterpret_cast<const uint8_t*>(vf.get_data()), /*1,*/ dims);
            }
            else {
                uint8_t byte = *reinterpret_cast<const uint8_t*>(thiz.get_data());
                outv[0] = MatlabParamParser::wrap(std::move(byte));
                mexWarnMsgTxt("Can't detect frame dims, sending only first byte");
            }
        });
        frame_factory.record("get_profile", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frame>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_frame_number());
        });
        // rs2::frame::is                                               [TODO] [T = {frame, video_frame, points, depth_frame, disparity_frame, motion_frame, pose_frame, frameset}]
        // rs2::frame::as                                               [TODO] [T = {frame, video_frame, points, depth_frame, disparity_frame, motion_frame, pose_frame, frameset}]
        factory->record(frame_factory);
    }
    {
        ClassFactory video_frame_factory("rs2::video_frame");
        // rs2::video_frame::constructor()                              [?]
        video_frame_factory.record("delete", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            MatlabParamParser::destroy<rs2::video_frame>(inv[0]);
        });
        video_frame_factory.record("get_width", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::video_frame>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_width());
        });
        video_frame_factory.record("get_height", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::video_frame>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_height());
        });
        video_frame_factory.record("get_stride_in_bytes", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::video_frame>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_stride_in_bytes());
        });
        video_frame_factory.record("get_bits_per_pixel", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::video_frame>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_bits_per_pixel());
        });
        video_frame_factory.record("get_bytes_per_pixel", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::video_frame>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_bytes_per_pixel());
        });
        factory->record(video_frame_factory);
    }
    {
        ClassFactory points_factory("rs2::points");
        // rs2::points::constructor()                                   [?]
        // rs2::points::constrcutor(rs2::frame)                         [?]
        points_factory.record("get_vertices", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::points>(inv[0]);
            // TODO: turn into matrix instead of column?
            size_t dims[] = { thiz.size() };
            outv[0] = MatlabParamParser::wrap_array(thiz.get_vertices(), /* 1, */ dims);
        });
        points_factory.record("export_to_ply", 0, 3, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::points>(inv[0]);
            auto fname = MatlabParamParser::parse<std::string>(inv[1]);
            auto texture = MatlabParamParser::parse<rs2::video_frame>(inv[2]);
            thiz.export_to_ply(fname, texture);
        });
        points_factory.record("get_texture_coordinates", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::points>(inv[0]);
            // TODO: turn into matrix instead of column?
            size_t dims[] = { thiz.size() };
            outv[0] = MatlabParamParser::wrap_array(thiz.get_texture_coordinates(), /* 1, */ dims);
        });
        points_factory.record("size", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::points>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.size());
        });
        factory->record(points_factory);
    }
    {
        ClassFactory depth_frame_factory("rs2::depth_frame");
        // rs2::depth_frame::constructor()                              [?]
        depth_frame_factory.record("delete", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            MatlabParamParser::destroy<rs2::depth_frame>(inv[0]);
        });
        depth_frame_factory.record("get_distance", 1, 3, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::depth_frame>(inv[0]);
            auto x = MatlabParamParser::parse<int>(inv[1]);
            auto y = MatlabParamParser::parse<int>(inv[2]);
            outv[0] = MatlabParamParser::wrap(thiz.get_distance(x, y));
        });
        factory->record(depth_frame_factory);
    }
    {
        ClassFactory disparity_frame_factory("rs2::disparity_frame");
        // rs2::disparity_frame::constructor(rs2::frame)                [?]
        disparity_frame_factory.record("get_baseline", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::disparity_frame>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_baseline());
        });
        factory->record(disparity_frame_factory);
    }
    {
        ClassFactory motion_frame_factory("rs2::motion_frame");
        // rs2::motion_frame::constructor(rs2::frame)                   [?]
        motion_frame_factory.record("get_motion_data", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::motion_frame>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_motion_data());
        });
        factory->record(motion_frame_factory);
    }
    {
        ClassFactory pose_frame_factory("rs2::pose_frame");
        // rs2::pose_frame::constructor(rs2::frame)                     [?]
        pose_frame_factory.record("get_pose_data", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::pose_frame>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_pose_data());
        });
        factory->record(pose_frame_factory);
    }
    {
        ClassFactory frameset_factory("rs2::frameset");
        // rs2::frameset::constructor()                                 [?]
        // rs2::frameset::constructor(frame)                            [?]
        frameset_factory.record("delete", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            MatlabParamParser::destroy<rs2::frameset>(inv[0]);
        });
        frameset_factory.record("first_or_default", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frameset>(inv[0]);
            auto s = MatlabParamParser::parse<rs2_stream>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.first_or_default(s));
        });
        frameset_factory.record("first", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frameset>(inv[0]);
            auto s = MatlabParamParser::parse<rs2_stream>(inv[1]);
            // try/catch moved to outer framework
            outv[0] = MatlabParamParser::wrap(thiz.first(s));
        });
        frameset_factory.record("get_depth_frame", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frameset>(inv[0]);
            // try/catch moved to outer framework
            outv[0] = MatlabParamParser::wrap(thiz.get_depth_frame());
        });
        frameset_factory.record("get_color_frame", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frameset>(inv[0]);
            // try/catch moved to outer framework
            outv[0] = MatlabParamParser::wrap(thiz.get_color_frame());
        });
        frameset_factory.record("get_infrared_frame", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frameset>(inv[0]);
            // try/catch moved to outer framework
            outv[0] = MatlabParamParser::wrap(thiz.get_infrared_frame());
        });
        frameset_factory.record("size", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frameset>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.size());
        });
        // rs2::frameset::foreach                                       [?/Callbacks]
        // rs2::frameset::operator[]                                    [?/HOW]
        // rs2::frameset::iterator+begin+end                            [Pure Matlab?]
        factory->record(frameset_factory);
    }

    // rs_sensor.hpp
    // rs2::notification                                                [?]
    {
        ClassFactory options_factory("rs2::options");
        options_factory.record("supports#rs2_option", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::options>(inv[0]);
            auto option = MatlabParamParser::parse<rs2_option>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.supports(option));
        });
        options_factory.record("get_option_description", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::options>(inv[0]);
            auto option = MatlabParamParser::parse<rs2_option>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.get_option_description(option));
        });
        options_factory.record("get_option_description", 1, 3, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::options>(inv[0]);
            auto option = MatlabParamParser::parse<rs2_option>(inv[1]);
            auto val = MatlabParamParser::parse<float>(inv[2]);
            outv[0] = MatlabParamParser::wrap(thiz.get_option_value_description(option, val));
        });
        options_factory.record("get_option", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::options>(inv[0]);
            auto option = MatlabParamParser::parse<rs2_option>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.get_option(option));
        });
        options_factory.record("get_option_range", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::options>(inv[0]);
            auto option = MatlabParamParser::parse<rs2_option>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.get_option_range(option));
        });
        options_factory.record("set_option", 0, 3, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::options>(inv[0]);
            auto option = MatlabParamParser::parse<rs2_option>(inv[1]);
            auto val = MatlabParamParser::parse<float>(inv[2]);
            thiz.set_option(option, val);
        });
        options_factory.record("is_option_read_only", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::options>(inv[0]);
            auto option = MatlabParamParser::parse<rs2_option>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.is_option_read_only(option));
        });
        // rs2::options::operator=                                      [?/HOW]
        // rs2::options::destructor                                     [?]
        factory->record(options_factory);
    }
    {
        ClassFactory sensor_factory("rs2::sensor");
        // rs2::sensor::constructor()                                   [?]
        sensor_factory.record("delete", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            MatlabParamParser::destroy<rs2::sensor>(inv[0]);
        });
        sensor_factory.record("open#stream_profile", 0, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::sensor>(inv[0]);
            auto profile = MatlabParamParser::parse<rs2::stream_profile>(inv[1]);
            thiz.open(profile);
        });
        sensor_factory.record("supports#rs2_camera_info", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::sensor>(inv[0]);
            auto info = MatlabParamParser::parse<rs2_camera_info>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.supports(info));
        });
        sensor_factory.record("get_info", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::sensor>(inv[0]);
            auto info = MatlabParamParser::parse<rs2_camera_info>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.get_info(info));
        });
        sensor_factory.record("open#vec_stream_profile", 0, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::sensor>(inv[0]);
            auto profiles = MatlabParamParser::parse<std::vector<rs2::stream_profile>>(inv[1]);
            thiz.open(profiles);
        });
        sensor_factory.record("close", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::sensor>(inv[0]);
            thiz.close();
        });
        // rs2::sensor::start(callback)                                 [?/Callbacks]
        // rs2::sensor::start(frame_queue)                              [TODO/Others?]
        sensor_factory.record("stop", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::sensor>(inv[0]);
            thiz.stop();
        });
        sensor_factory.record("get_stream_profiles", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::sensor>(inv[0]);
            MatlabParamParser::wrap(thiz.get_stream_profiles()); // TODO - convert to wrap_array
        });
        // rs2::sensor::operator=                                       [?]
        // rs2::sensor::operator bool                                   [?]
        // rs2::sensor::is                                              [TODO] [T = {sensor, roi_sensor, depth_sensor, depth_stereo_sensor}]
        // rs2::sensor::as                                              [TODO] [T = {sensor, roi_sensor, depth_sensor, depth_stereo_sensor}]
        sensor_factory.record("operator==", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto rhs = MatlabParamParser::parse<rs2::sensor>(inv[0]);
            auto lhs = MatlabParamParser::parse<rs2::sensor>(inv[1]);
            MatlabParamParser::wrap(rhs == lhs);
        });
        factory->record(sensor_factory);
    }
    {
        ClassFactory roi_sensor_factory("rs2::roi_sensor");
        // rs2::roi_sensor::constructor(rs2::sensor)                    [?]
        roi_sensor_factory.record("set_region_of_interest", 0, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::roi_sensor>(inv[0]);
            auto roi = MatlabParamParser::parse<rs2::region_of_interest>(inv[1]);
            thiz.set_region_of_interest(roi);
        });
        roi_sensor_factory.record("get_region_of_interest", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::roi_sensor>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_region_of_interest());
        });
        // rs2::roi_sensor::operator bool                               [?]
        factory->record(roi_sensor_factory);
    }
    {
        ClassFactory depth_sensor_factory("rs2::depth_sensor");
        // rs2::depth_sensor::constructor(rs2::sensor)                  [?]
        depth_sensor_factory.record("get_depth_scale", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::depth_sensor>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_depth_scale());
        });
        // rs2::depth_sensor::operator bool                             [?]
        factory->record(depth_sensor_factory);
    }
    {
        ClassFactory depth_stereo_sensor_factory("rs2::depth_stereo_sensor");
        // rs2::depth_stereo_sensor::constructor(rs2::sensor)           [?]
        depth_stereo_sensor_factory.record("get_stereo_baseline", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::depth_stereo_sensor>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_stereo_baseline());
        });
        factory->record(depth_stereo_sensor_factory);
    }

    // rs_device.hpp
    {
        ClassFactory device_factory("rs2::device");
        // rs2::device::constructor()                                   [?]
        // extra helper function for constructing device from device_list
        device_factory.record("init", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto list = MatlabParamParser::parse<rs2::device_list>(inv[0]);
            auto idx = MatlabParamParser::parse<uint64_t>(inv[1]);
            outv[0] = MatlabParamParser::wrap(list[idx]);
            MatlabParamParser::destroy<rs2::device_list>(inv[0]);
        });
        // destructor in case device was never initialized
        device_factory.record("delete#uninit", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            MatlabParamParser::destroy<rs2::device_list>(inv[0]);
        });
        // destructor in case device was initialized
        device_factory.record("delete#init", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            MatlabParamParser::destroy<rs2::device>(inv[0]);
        });
        device_factory.record("query_sensors", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::device>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.query_sensors());
        });
        device_factory.record("first", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            // TODO: better, more maintainable implementation?
            auto thiz = MatlabParamParser::parse<rs2::device>(inv[0]);
            auto type = MatlabParamParser::parse<std::string>(inv[1]);
            try {
                if (type == "sensor")
                    outv[0] = MatlabParamParser::wrap(thiz.first<rs2::sensor>());
                else if (type == "roi_sensor")
                    outv[0] = MatlabParamParser::wrap(thiz.first<rs2::roi_sensor>());
                else if (type == "depth_sensor")
                    outv[0] = MatlabParamParser::wrap(thiz.first<rs2::depth_sensor>());
                else if (type == "depth_stereo_sensor")
                    outv[0] = MatlabParamParser::wrap(thiz.first<rs2::depth_stereo_sensor>());
                else mexErrMsgTxt("rs2::device::first: Could not find requested sensor type!");
            }
            catch (rs2::error) {
                mexErrMsgTxt("rs2::device::first: Could not find requested sensor type!");
            }
        });
        device_factory.record("supports", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::device>(inv[0]);
            auto info = MatlabParamParser::parse<rs2_camera_info>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.supports(info));
        });
        device_factory.record("get_info", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::device>(inv[0]);
            auto info = MatlabParamParser::parse<rs2_camera_info>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.get_info(info));
        });
        device_factory.record("hardware_reset", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::device>(inv[0]);
            thiz.hardware_reset();
        });
        // rs2::device::operator=                                       [?]
        // rs2::device::operator bool                                   [?]
        device_factory.record("is", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            // TODO: something more maintainable?
            auto thiz = MatlabParamParser::parse<rs2::device>(inv[0]);
            auto type = MatlabParamParser::parse<std::string>(inv[1]);
            if (type == "device")
                outv[0] = MatlabParamParser::wrap(thiz.is<rs2::device>());
            else if (type == "debug_protocol") {
                mexWarnMsgTxt("rs2::device::is: Debug Protocol not supported in MATLAB");
                outv[0] = MatlabParamParser::wrap(thiz.is<rs2::debug_protocol>());
            }
            else if (type == "advanced_mode") {
                mexWarnMsgTxt("rs2::device::is: Advanced Mode not supported in MATLAB");
                outv[0] = MatlabParamParser::wrap(false);
            }
            else if (type == "recorder")
                outv[0] = MatlabParamParser::wrap(thiz.is<rs2::recorder>());
            else if (type == "playback")
                outv[0] = MatlabParamParser::wrap(thiz.is<rs2::playback>());
            else {
                // TODO: need warn/error message? which? if so, fill in
                mexWarnMsgTxt("rs2::device::is: ...");
                outv[0] = MatlabParamParser::wrap(false);
            }
        });
        // rs2::device::as                                              [Pure Matlab]
        factory->record(device_factory);
    }
    // rs2::debug_protocol                                              [?]
    // rs2::device_list                                                 [Pure Matlab]

    // rs2_record_playback.hpp
    {
        ClassFactory playback_factory("rs2::playback");
        // rs2::playback::constructor(rs2::device)                      [?]
        playback_factory.record("pause", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::playback>(inv[0]);
            thiz.pause();
        });
        playback_factory.record("resume", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::playback>(inv[0]);
            thiz.resume();
        });
        playback_factory.record("file_name", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::playback>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.file_name());
        });
        playback_factory.record("get_position", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::playback>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_position());
        });
        playback_factory.record("get_duration", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::playback>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_duration());
        });
        playback_factory.record("seek", 0, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::playback>(inv[0]);
            auto time = MatlabParamParser::parse<std::chrono::nanoseconds>(inv[1]);
            thiz.seek(time);
        });
        playback_factory.record("is_real_time", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::playback>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.is_real_time());
        });
        playback_factory.record("set_real_time", 0, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::playback>(inv[0]);
            auto real_time = MatlabParamParser::parse<bool>(inv[1]);
            thiz.set_real_time(real_time);
        });
        playback_factory.record("set_playback_speed", 0, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::playback>(inv[0]);
            auto speed = MatlabParamParser::parse<float>(inv[1]);
            thiz.set_playback_speed(speed);
        });
        // rs2::playback::set_status_changed_callback()                 [?/Callbacks]
        playback_factory.record("current_status", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::playback>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.current_status());
        });
        playback_factory.record("stop", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::playback>(inv[0]);
            thiz.stop();
        });
        factory->record(playback_factory);
    }
    {
        ClassFactory recorder_factory("rs2::recorder");
        // rs2::recorder::constructor(rs2::device)                      [?]
        recorder_factory.record("new#string_device", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto file = MatlabParamParser::parse<std::string>(inv[0]);
            auto device = MatlabParamParser::parse<rs2::device>(inv[1]);
            outv[0] = MatlabParamParser::wrap(rs2::recorder(file, device));
        });
        recorder_factory.record("pause", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::recorder>(inv[0]);
            thiz.pause();
        });
        recorder_factory.record("resume", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::recorder>(inv[0]);
            thiz.resume();
        });
        recorder_factory.record("filename", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::recorder>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.filename());
        });
        factory->record(recorder_factory);
    }

    // rs2_processing.hpp
    // rs2::processing_block                                            [?]
    {
        ClassFactory frame_queue_factory("rs2::frame_queue");
        frame_queue_factory.record("new", 1, 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            if (inc == 0) {
                outv[0] = MatlabParamParser::wrap(rs2::frame_queue());
            }
            else if (inc == 1) {
                auto capacity = MatlabParamParser::parse<unsigned int>(inv[0]);
                outv[0] = MatlabParamParser::wrap(rs2::frame_queue(capacity));
            }
        });
        // rs2::frame_queue::enqueue(frame)                             [?]
        frame_queue_factory.record("wait_for_frame", 1, 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frame_queue>(inv[0]);
            if (inc == 0) {
                outv[0] = MatlabParamParser::wrap(thiz.wait_for_frame());
            }
            else if (inc == 1) {
                auto timeout_ms = MatlabParamParser::parse<unsigned int>(inv[1]);
                outv[0] = MatlabParamParser::wrap(thiz.wait_for_frame(timeout_ms));
            }
        });
        // rs2::frame_queue::poll_for_frame(T*)                         [TODO] [T = {frame, video_frame, points, depth_frame, disparity_frame, motion_frame, pose_frame, frameset}]
        frame_queue_factory.record("new", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::frame_queue>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.capacity());
        });
        factory->record(frame_queue_factory);
    }
    {
        ClassFactory pointcloud_factory("rs2::pointcloud");
        pointcloud_factory.record("new", 1, 0, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            outv[0] = MatlabParamParser::wrap(rs2::pointcloud());
        });
        pointcloud_factory.record("calculate", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::pointcloud>(inv[0]);
            auto depth = MatlabParamParser::parse<rs2::frame>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.calculate(depth));
        });
        pointcloud_factory.record("map_to", 0, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::pointcloud>(inv[0]);
            auto mapped = MatlabParamParser::parse<rs2::frame>(inv[1]);
            thiz.map_to(mapped);
        });
        factory->record(pointcloud_factory);
    }
    {
        ClassFactory syncer_factory("rs2::syncer");
        syncer_factory.record("new", 1, 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            if (inc == 0) {
                outv[0] = MatlabParamParser::wrap(rs2::syncer());
            }
            else if (inc == 1) {
                auto queue_size = MatlabParamParser::parse<int>(inv[0]);
                outv[0] = MatlabParamParser::wrap(rs2::syncer(queue_size));
            }
        });
        syncer_factory.record("wait_for_frames", 1, 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::syncer>(inv[0]);
            if (inc == 0) {
                outv[0] = MatlabParamParser::wrap(thiz.wait_for_frames());
            }
            else if (inc == 1) {
                auto timeout_ms = MatlabParamParser::parse<unsigned int>(inv[1]);
                outv[0] = MatlabParamParser::wrap(thiz.wait_for_frames(timeout_ms));
            }
        });
        // rs2::syncer::poll_for_frames(frameset*)                      [?]
        factory->record(syncer_factory);
    }
    {
        ClassFactory align_factory("rs2::align");
        align_factory.record("new", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto align_to = MatlabParamParser::parse<rs2_stream>(inv[0]);
            outv[0] = MatlabParamParser::wrap(rs2::align(align_to));
        });
        align_factory.record("process", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::align>(inv[0]);
            auto frame = MatlabParamParser::parse<rs2::frameset>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.process(frame));
        });
        factory->record(align_factory);
    }
    {
        ClassFactory colorizer_factory("rs2::colorizer");
        colorizer_factory.record("new", 1, 0, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            outv[0] = MatlabParamParser::wrap(rs2::colorizer());
        });
        colorizer_factory.record("colorize", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::colorizer>(inv[0]);
            auto depth = MatlabParamParser::parse<rs2::frame>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.colorize(depth));
        });
        factory->record(colorizer_factory);
    }
    {
        ClassFactory process_interface_factory("rs2::process_interface");
        process_interface_factory.record("process", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            // TODO: will this work?
            auto *thiz = MatlabParamParser::parse<rs2::process_interface*>(inv[0]);
            auto frame = MatlabParamParser::parse<rs2::frame>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz->process(frame));
        });
        factory->record(process_interface_factory);
    }
    {
        ClassFactory decimation_filter_factory("rs2::decimation_filter");
        decimation_filter_factory.record("new", 1, 0, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            outv[0] = MatlabParamParser::wrap(rs2::decimation_filter());
        });
        factory->record(decimation_filter_factory);
    }
    {
        ClassFactory temporal_filter_factory("rs2::temporal_filter");
        temporal_filter_factory.record("new", 1, 0, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            outv[0] = MatlabParamParser::wrap(rs2::temporal_filter());
        });
        factory->record(temporal_filter_factory);
    }
    {
        ClassFactory spatial_filter_factory("rs2::spatial_filter");
        spatial_filter_factory.record("new", 1, 0, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            outv[0] = MatlabParamParser::wrap(rs2::spatial_filter());
        });
        factory->record(spatial_filter_factory);
    }
    {
        ClassFactory disparity_transform_factory("rs2::disparity_transform");
        disparity_transform_factory.record("new", 1, 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            if (inc == 0) {
                outv[0] = MatlabParamParser::wrap(rs2::disparity_transform());
            }
            else if (inc == 1) {
                auto transform_to_disparity = MatlabParamParser::parse<bool>(inv[0]);
                outv[0] = MatlabParamParser::wrap(rs2::disparity_transform(transform_to_disparity));
            }
        });
        factory->record(disparity_transform_factory);
    }
    {
        ClassFactory hole_filling_filter_factory("rs2::hole_filling_filter");
        hole_filling_filter_factory.record("new", 1, 0, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            outv[0] = MatlabParamParser::wrap(rs2::hole_filling_filter());
        });
        factory->record(hole_filling_filter_factory);
    }

    // rs_context.hpp
    // rs2::event_information                                           [?]
    {
        ClassFactory context_factory("rs2::context");
        // This lambda feels like it should be possible to generate automatically with templates
        context_factory.record("new", 1, 0, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            outv[0] = MatlabParamParser::wrap(rs2::context());
        });
        context_factory.record("delete", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            MatlabParamParser::destroy<rs2::context>(inv[0]);
        });
        context_factory.record("query_devices", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::context>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.query_devices());
        });
        context_factory.record("query_all_sensors", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::context>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.query_all_sensors());
        });
        context_factory.record("get_sensor_parent", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::context>(inv[0]);
            auto sensor = MatlabParamParser::parse<rs2::sensor>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.get_sensor_parent(sensor));
        });
        context_factory.record("load_device", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::context>(inv[0]);
            auto file = MatlabParamParser::parse<std::string>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.load_device(file));
        });
        context_factory.record("unload_device", 0, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::context>(inv[0]);
            auto file = MatlabParamParser::parse<std::string>(inv[1]);
            thiz.unload_device(file);
        });
        factory->record(context_factory);
    }
    {
        ClassFactory device_hub_factory("rs2::device_hub");
        device_hub_factory.record("new", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto ctx = MatlabParamParser::parse<rs2::context>(inv[0]);
            outv[0] = MatlabParamParser::wrap(rs2::device_hub(ctx));
        });
        device_hub_factory.record("delete", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            MatlabParamParser::destroy<rs2::device_hub>(inv[0]);
        });
        device_hub_factory.record("wait_for_device", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::device_hub>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.wait_for_device());
        });
        device_hub_factory.record("is_connected", 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::device_hub>(inv[0]);
            auto dev = MatlabParamParser::parse<rs2::device>(inv[1]);
            outv[0] = MatlabParamParser::wrap(thiz.is_connected(dev));
        });
        factory->record(device_hub_factory);
    }

    // rs_pipeline.hpp
    {
        ClassFactory pipeline_profile_factory("rs2::pipeline_profile");
        // rs2::pipeline_profile::constructor()                         [?]
        pipeline_profile_factory.record("get_streams", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::pipeline_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_streams()); // TODO: switch to make_array
        });
        pipeline_profile_factory.record("get_stream", 1, 2, 3, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::pipeline_profile>(inv[0]);
            auto stream_type = MatlabParamParser::parse<rs2_stream>(inv[1]);
            if (inc == 2)
                outv[0] = MatlabParamParser::wrap(thiz.get_stream(stream_type));
            else {
                auto stream_index = MatlabParamParser::parse<int>(inv[2]);
                outv[0] = MatlabParamParser::wrap(thiz.get_stream(stream_type, stream_index));
            }
        });
        pipeline_profile_factory.record("get_device", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::pipeline_profile>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_device()); // TODO: switch to make_array
        });
        // rs2::pipeline_profile::bool()                                [?]
        factory->record(pipeline_profile_factory);
    }
    {
        ClassFactory config_factory("rs2::config");
        config_factory.record("new", 1, 0, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            outv[0] = MatlabParamParser::wrap(rs2::config());
        });
        // rs2::config::enable_stream(rs2_stream, int, int, int, rs2_format=DEF, int=DEF)   [TODO]
        // rs2::config::enable_stream(rs2_stream, int=DEF)              [TODO]
        // rs2::config::enable_stream(rs2_stream, int, int, rs2_format=DEF, int=DEF)        [TODO]
        // rs2::config::enable_stream(rs2_stream, rs2_format, int=DEF)  [TODO]
        // rs2::config::enable_stream(rs2_stream, int, rs2_format, int=DEF)                 [TODO]
        config_factory.record("enable_all_streams", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::config>(inv[0]);
            thiz.enable_all_streams();
        });
        config_factory.record("enable_device", 0, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::config>(inv[0]);
            auto serial = MatlabParamParser::parse<std::string>(inv[1]);
            thiz.enable_device(serial);
        });
        config_factory.record("enable_device_from_file", 0, 2, 3, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::config>(inv[0]);
            auto file_name = MatlabParamParser::parse<std::string>(inv[1]);
            if (inc == 2)
                thiz.enable_device_from_file(file_name);
            else if (inc == 3) {
                auto repeat_playback = MatlabParamParser::parse<bool>(inv[2]);
                thiz.enable_device_from_file(file_name, repeat_playback);
            }
        });
        config_factory.record("enable_record_to_file", 0, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::config>(inv[0]);
            auto file_name = MatlabParamParser::parse<std::string>(inv[1]);
            thiz.enable_record_to_file(file_name);
        });
        config_factory.record("disable_stream", 0, 2, 3, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::config>(inv[0]);
            auto stream = MatlabParamParser::parse<rs2_stream>(inv[1]);
            if (inc == 2)
                thiz.disable_stream(stream);
            else if (inc == 3) {
                auto index = MatlabParamParser::parse<int>(inv[2]);
                thiz.disable_stream(stream, index);
            }
        });
        config_factory.record("disable_all_streams", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::config>(inv[0]);
            thiz.disable_all_streams();
        });
        factory->record(config_factory);
    }
    {
        ClassFactory pipeline_factory("rs2::pipeline");
        pipeline_factory.record("new", 1, 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            if (inc == 0) {
                outv[0] = MatlabParamParser::wrap(rs2::pipeline());
            } else if (inc == 1) {
                auto ctx = MatlabParamParser::parse<rs2::context>(inv[0]);
                outv[0] = MatlabParamParser::wrap(rs2::pipeline(ctx));
            }
        });
        pipeline_factory.record("delete", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            MatlabParamParser::destroy<rs2::pipeline>(inv[0]);
        });
        pipeline_factory.record("start", 1, 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::pipeline>(inv[0]);
            if (inc == 1)
                outv[0] = MatlabParamParser::wrap(thiz.start());
            else if (inc == 2) {
                auto config = MatlabParamParser::parse<rs2::config>(inv[1]);
                outv[0] = MatlabParamParser::wrap(thiz.start(config));
            }
        });
        pipeline_factory.record("stop", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::pipeline>(inv[0]);
            thiz.stop();
        });
        pipeline_factory.record("wait_for_frames", 1, 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::pipeline>(inv[0]);
            if (inc == 1) {
                outv[0] = MatlabParamParser::wrap(thiz.wait_for_frames());
            } else if (inc == 2) {
                auto timeout_ms = MatlabParamParser::parse<unsigned int>(inv[1]);
                outv[0] = MatlabParamParser::wrap(thiz.wait_for_frames(timeout_ms));
            }
        });
        // rs2::pipeline::poll_for_frames                               [TODO/HOW] [multi-output?]
        pipeline_factory.record("get_active_profile", 1, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[])
        {
            auto thiz = MatlabParamParser::parse<rs2::pipeline>(inv[0]);
            outv[0] = MatlabParamParser::wrap(thiz.get_active_profile());
        });
        factory->record(pipeline_factory);
    }

    // rs.hpp
    {
        ClassFactory free_funcs_factory("rs2");
        free_funcs_factory.record("log_to_console", 0, 1, [](int outc, mxArray* outv[], int inc, const mxArray* inv[]) {
            auto min_severity = MatlabParamParser::parse<rs2_log_severity>(inv[0]);
            rs2::log_to_console(min_severity);
        });
        free_funcs_factory.record("log_to_file", 0, 1, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[]) {
            auto min_severity = MatlabParamParser::parse<rs2_log_severity>(inv[0]);
            if (inc == 1)
                rs2::log_to_file(min_severity);
            else if (inc == 2)
                auto file_path = MatlabParamParser::parse<const char *>(inv[1]);
                rs2::log_to_file(min_severity, file_path);
        });
        free_funcs_factory.record("log", 0, 2, [](int outc, mxArray* outv[], int inc, const mxArray* inv[]) {
            auto severity = MatlabParamParser::parse<rs2_log_severity>(inv[0]);
            auto message = MatlabParamParser::parse<const char *>(inv[1]);
            rs2::log(min_severity, message);
        });
    }

    mexAtExit([]() { delete factory; });
}

void mexFunction(int nOutParams, mxArray *outParams[], int nInParams, const mxArray *inParams[])
{
    // does this need to be made threadsafe? also maybe better idea than global object?
    if (!factory) make_factory();

    if (nInParams < 2) {
        mexErrMsgTxt("At least class and command name are needed.");
        return;
    }
    
    auto cname = MatlabParamParser::parse<std::string>(inParams[0]);
    auto fname = MatlabParamParser::parse<std::string>(inParams[1]);

    auto f_data = factory->get(cname, fname);
    if (!f_data.f) {
        mexErrMsgTxt("Unknown Command received.");
        return;
    }

    if (f_data.out != nOutParams) {
        std::string errmsg = cname + "::" + fname.substr(0, fname.find("#", 0)) + ": Wrong number of outputs";
        mexErrMsgTxt(errmsg.c_str());
    }

    if (f_data.in_min > nInParams - 2 || f_data.in_max < nInParams - 2) {
        std::string errmsg = cname + "::" + fname.substr(0, fname.find("#", 0)) + ": Wrong number of inputs";
        mexErrMsgTxt(errmsg.c_str());
    }
    try {
        f_data.f(nOutParams, outParams, nInParams - 2, inParams + 2); // "eat" the two function specifiers
    } catch (std::exception &e) {
        mexErrMsgTxt(e.what());
    }
}