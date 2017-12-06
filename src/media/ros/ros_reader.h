// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once
#include <chrono>
#include <mutex>
#include <regex>
#include <core/serialization.h>
#include "rosbag/view.h"
#include "sensor_msgs/Imu.h"
#include "sensor_msgs/Image.h"
#include "diagnostic_msgs/KeyValue.h"
#include "std_msgs/UInt32.h"
#include "std_msgs/Float32.h"
#include "std_msgs/String.h"
#include "realsense_msgs/StreamInfo.h"
#include "realsense_legacy_msgs/legacy_headers.h"
#include "sensor_msgs/CameraInfo.h"
#include "ros_file_format.h"

namespace librealsense
{
    using namespace device_serializer;

    class ros_reader: public device_serializer::reader
    {
    public:
        ros_reader(const std::string& file, const std::shared_ptr<context>& ctx) :
            m_total_duration(0),
            m_file_path(file),
            m_context(ctx),
            m_version(0),
            m_metadata_parser_map(create_metadata_parser_map())
        {
            try
            {
                reset(); //Note: calling a virtual function inside c'tor, safe while base function is pure virtual
                m_total_duration = get_file_duration(m_file, m_version);
            }
            catch (const std::exception& e)
            {
                //Rethrowing with better clearer message
                throw io_exception(to_string() << "Failed to create ros reader: " << e.what());
            }
        }

        device_snapshot query_device_description(const nanoseconds& time) override
        {
            return read_device_description(time);
        }

        std::shared_ptr<serialized_data> read_next_data() override
        {
            if (m_samples_view == nullptr || m_samples_itrator == m_samples_view->end())
            {
                LOG_DEBUG("End of file reached");
                return std::make_shared<serialized_end_of_file>();
            }

            rosbag::MessageInstance next_msg = *m_samples_itrator;
            ++m_samples_itrator;

            if (next_msg.isType<sensor_msgs::Image>() || next_msg.isType<sensor_msgs::Imu>() || next_msg.isType<realsense_legacy_msgs::pose>())
            {
                LOG_DEBUG("Next message is a frame");
                return create_frame(next_msg);
            }

            if (m_version >= 3)
            {
                if (next_msg.isType<std_msgs::Float32>())
                {
                    LOG_DEBUG("Next message is an option");
                    auto timestamp = to_nanoseconds(next_msg.getTime());
                    auto sensor_id = ros_topic::get_sensor_identifier(next_msg.getTopic());
                    auto option = create_option(m_file, next_msg);
                    return std::make_shared<serialized_option>(timestamp, sensor_id, option.first, option.second);
                }
            }

            std::string err_msg = to_string() << "Unknown message type: " << next_msg.getDataType() << "(Topic: " << next_msg.getTopic() << ")";
            LOG_ERROR(err_msg);
            throw invalid_value_exception(err_msg);
        }

        void seek_to_time(const nanoseconds& seek_time) override
        {
            if (seek_time > m_total_duration)
            {
                throw invalid_value_exception(to_string() << "Requested time is out of playback length. (Requested = " << seek_time.count() << ", Duration = " << m_total_duration.count() << ")");
            }
            auto seek_time_as_secs = std::chrono::duration_cast<std::chrono::duration<double>>(seek_time);
            auto seek_time_as_rostime = ros::Time(seek_time_as_secs.count());

            m_samples_view.reset(new rosbag::View(m_file, FalseQuery()));
            
            //Using cached topics here and not querying them (before reseting) since a previous call to seek
            // could have changed the view and some streams that should be streaming were dropped.
            //E.g:  Recording Depth+Color, stopping Depth, starting IR, stopping IR and Color. Play IR+Depth: will play only depth, then only IR, then we seek to a point only IR was streaming, and then to 0.
            for (auto topic : m_enabled_streams_topics)
            {
                m_samples_view->addQuery(m_file, rosbag::TopicQuery(topic), seek_time_as_rostime);
            }
            m_samples_itrator = m_samples_view->begin();
        }

        nanoseconds query_duration() const override
        {
            return m_total_duration;
        }

        void reset() override
        {
            m_file.close();
            m_file.open(m_file_path, rosbag::BagMode::Read);
            m_version = read_file_version(m_file);
            m_samples_view = nullptr;
            m_frame_source = std::make_shared<frame_source>(m_version == 1 ? 128 : 16);
            m_frame_source->init(m_metadata_parser_map);
            m_initial_device_description = read_device_description(get_static_file_info_timestamp(), true);
        }

        virtual void enable_stream(const std::vector<device_serializer::stream_identifier>& stream_ids) override
        {
            ros::Time start_time = ros::TIME_MIN + ros::Duration{ 0, 1 }; //first non 0 timestamp and afterward
            if (m_samples_view == nullptr) //Starting to stream
            {
                m_samples_view = std::unique_ptr<rosbag::View>(new rosbag::View(m_file, FalseQuery()));
                m_samples_view->addQuery(m_file, OptionsQuery(), start_time);
                m_samples_itrator = m_samples_view->begin();
            }
            else //Already streaming
            {
                if (m_samples_itrator != m_samples_view->end())
                {
                    rosbag::MessageInstance sample_msg = *m_samples_itrator;
                    start_time = sample_msg.getTime();
                }
            }
            auto currently_streaming = get_topics(m_samples_view);
            //empty the view
            m_samples_view = std::unique_ptr<rosbag::View>(new rosbag::View(m_file, FalseQuery()));

            for (auto&& stream_id : stream_ids)
            {
                //add new stream to view
                if (m_version == legacy_file_format::file_version())
                {
                    m_samples_view->addQuery(m_file, legacy_file_format::StreamQuery(stream_id), start_time);
                }
                else
                {
                    m_samples_view->addQuery(m_file, StreamQuery(stream_id), start_time);
                }
            }

            //add already existing streams
            for (auto topic : currently_streaming)
            {
                m_samples_view->addQuery(m_file, rosbag::TopicQuery(topic), start_time);
            }
            m_samples_itrator = m_samples_view->begin();
            m_enabled_streams_topics = get_topics(m_samples_view);
        }

        virtual void disable_stream(const std::vector<device_serializer::stream_identifier>& stream_ids) override
        {
            if (m_samples_view == nullptr)
            {
                return;
            }
            ros::Time curr_time;
            if (m_samples_itrator == m_samples_view->end())
            {
                curr_time = m_samples_view->getEndTime();
            }
            else
            {
                rosbag::MessageInstance sample_msg = *m_samples_itrator;
                curr_time = sample_msg.getTime();
            }
            auto currently_streaming = get_topics(m_samples_view);
            m_samples_view = std::unique_ptr<rosbag::View>(new rosbag::View(m_file, FalseQuery()));
            for (auto topic : currently_streaming)
            {
                //Find if this topic is one of the streams that should be disabled
                auto it = std::find_if(stream_ids.begin(), stream_ids.end(), [&topic](const device_serializer::stream_identifier& s) {
                    //return topic.starts_with(s);
                    return topic.find(ros_topic::stream_full_prefix(s)) != std::string::npos;
                });
                bool should_topic_remain = (it == stream_ids.end());
                if (should_topic_remain)
                {
                    m_samples_view->addQuery(m_file, rosbag::TopicQuery(topic), curr_time);
                }
            }
            m_samples_itrator = m_samples_view->begin();
            m_enabled_streams_topics = get_topics(m_samples_view);
        }

        const std::string& get_file_name() const override
        {
            return m_file_path;
        }

    private:

        template <typename ROS_TYPE>      
        static typename ROS_TYPE::ConstPtr instantiate_msg(const rosbag::MessageInstance& msg)
        {
            typename ROS_TYPE::ConstPtr msg_instnance_ptr = msg.instantiate<ROS_TYPE>();
            if (msg_instnance_ptr == nullptr)
            {
                throw io_exception(to_string() 
                    << "Invalid file format, expected " 
                    << ros::message_traits::DataType<ROS_TYPE>::value()
                    << " message but got: " << msg.getDataType()
                    << "(Topic: " << msg.getTopic() << ")");
            }
            return msg_instnance_ptr;
        }
        
        std::shared_ptr<serialized_frame> create_frame(const rosbag::MessageInstance& msg)
        {
            auto next_msg_topic = msg.getTopic();
            auto next_msg_time = msg.getTime();

            nanoseconds timestamp = to_nanoseconds(next_msg_time);
            stream_identifier stream_id;
            if (m_version == legacy_file_format::file_version())
            {
                stream_id = legacy_file_format::get_stream_identifier(next_msg_topic);
            }
            else
            {
                stream_id = ros_topic::get_stream_identifier(next_msg_topic);
            }
            if (msg.isType<sensor_msgs::Image>())
            {
                frame_holder frame = create_image_from_message(msg);
                return std::make_shared<serialized_frame>(timestamp, stream_id, std::move(frame));
            }

            if (msg.isType<sensor_msgs::Imu>())
            {
                frame_holder frame = create_motion_sample(msg);
                return std::make_shared<serialized_frame>(timestamp, stream_id, std::move(frame));
            }

            if (msg.isType<realsense_legacy_msgs::pose>())
            {
                frame_holder frame = create_pose_sample(msg);
                return std::make_shared<serialized_frame>(timestamp, stream_id, std::move(frame));
            }
            std::string err_msg = to_string() << "Unknown frame type: " << msg.getDataType() << "(Topic: " << next_msg_topic << ")";
            LOG_ERROR(err_msg);
            throw invalid_value_exception(err_msg);
        }

        static std::shared_ptr<metadata_parser_map> create_metadata_parser_map()
        {
            auto md_parser_map = std::make_shared<metadata_parser_map>();
            for (int i = 0; i < static_cast<int>(rs2_frame_metadata_value::RS2_FRAME_METADATA_COUNT); ++i)
            {
                auto frame_md_type = static_cast<rs2_frame_metadata_value>(i);
                md_parser_map->insert(std::make_pair(frame_md_type, std::make_shared<md_constant_parser>(frame_md_type)));
            }
            return md_parser_map;
        }

        static nanoseconds get_file_duration(const rosbag::Bag& file, uint32_t version)
        {
            std::function<bool(rosbag::ConnectionInfo const* info)> query;
            if (version == legacy_file_format::file_version())
                query = legacy_file_format::FrameQuery();
            else
                query = FrameQuery();
            rosbag::View all_frames_view(file, query);
            auto streaming_duration = all_frames_view.getEndTime() - all_frames_view.getBeginTime();
            return nanoseconds(streaming_duration.toNSec());
        }

        static void get_legacy_frame_metadata(const rosbag::Bag& bag, 
            const device_serializer::stream_identifier& stream_id, 
            const rosbag::MessageInstance &msg, 
            frame_additional_data& additional_data)
        {
            uint32_t total_md_size = 0;
            rosbag::View frame_metadata_view(bag, legacy_file_format::FrameInfoExt(stream_id), msg.getTime(), true);
            assert(frame_metadata_view.size() == 1);
            for (auto message_instance : frame_metadata_view)
            {
                auto info = instantiate_msg<realsense_legacy_msgs::frame_info>(message_instance);
                for (auto&& fmd : info->frame_metadata)
                {
                    if (fmd.type == legacy_file_format::SYSTEM_TIMESTAMP)
                    {
                        additional_data.system_time = *reinterpret_cast<const int64_t*>(fmd.data.data());
                    }
                    else
                    {
                        rs2_frame_metadata_value type;
                        if (!legacy_file_format::convert_metadata_type(fmd.type, type))
                        {
                            continue;
                        }
                        rs2_metadata_type value = *reinterpret_cast<const rs2_metadata_type*>(fmd.data.data());
                        auto size_of_enum = sizeof(rs2_frame_metadata_value);
                        auto size_of_data = sizeof(rs2_metadata_type);
                        if (total_md_size + size_of_enum + size_of_data > 255)
                        {
                            continue; //stop adding metadata to frame
                        }
                        memcpy(additional_data.metadata_blob.data() + total_md_size, &type, size_of_enum);
                        total_md_size += static_cast<uint32_t>(size_of_enum);
                        memcpy(additional_data.metadata_blob.data() + total_md_size, &value, size_of_data);
                        total_md_size += static_cast<uint32_t>(size_of_data);
                    }
                }
                additional_data.timestamp_domain = legacy_file_format::convert(info->time_stamp_domain);
            }
        }
        
        static void get_frame_metadata(const rosbag::Bag& bag, 
            const std::string& topic,
            const device_serializer::stream_identifier& stream_id, 
            const rosbag::MessageInstance &msg, 
            frame_additional_data& additional_data)
        {
            uint32_t total_md_size = 0;
            rosbag::View frame_metadata_view(bag, rosbag::TopicQuery(topic), msg.getTime(), msg.getTime());
            for (auto message_instance : frame_metadata_view)
            {
                auto key_val_msg = instantiate_msg<diagnostic_msgs::KeyValue>(message_instance);

                if (key_val_msg->key == "timestamp_domain") //TODO: use constants
                {
                    convert(key_val_msg->value, additional_data.timestamp_domain);
                }
                else if (key_val_msg->key == "system_time") //TODO: use constants
                {
                    additional_data.system_time = std::stod(key_val_msg->value);
                }
                else
                {
                    rs2_frame_metadata_value type;
                    try
                    {
                        convert(key_val_msg->key, type);
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR(e.what());
                        continue;
                    }
                    auto size_of_enum = sizeof(rs2_frame_metadata_value);
                    rs2_metadata_type md = static_cast<rs2_metadata_type>(std::stoll(key_val_msg->value));
                    auto size_of_data = sizeof(rs2_metadata_type);
                    if (total_md_size + size_of_enum + size_of_data > 255)
                    {
                        continue; //stop adding metadata to frame
                    }
                    memcpy(additional_data.metadata_blob.data() + total_md_size, &type, size_of_enum);
                    total_md_size += static_cast<uint32_t>(size_of_enum);
                    memcpy(additional_data.metadata_blob.data() + total_md_size, &md, size_of_data);
                    total_md_size += static_cast<uint32_t>(size_of_data);
                }
            }
            additional_data.metadata_size = total_md_size;
        }

        frame_holder create_image_from_message(const rosbag::MessageInstance &image_data) const
        {
            LOG_DEBUG("Trying to create an image frame from message");
            auto msg = image_data.instantiate<sensor_msgs::Image>();
            if (msg == nullptr)
            {
                throw io_exception(to_string()
                    << "Invalid file format, expected "
                    << ros::message_traits::DataType<sensor_msgs::Image>::value()
                    << " message but got: " << image_data.getDataType()
                    << "(Topic: " << image_data.getTopic() << ")");
            }

            frame_additional_data additional_data{};
            std::chrono::duration<double, std::milli> timestamp_ms(std::chrono::duration<double>(msg->header.stamp.toSec()));
            additional_data.timestamp = timestamp_ms.count();
            additional_data.frame_number = msg->header.seq;
            additional_data.fisheye_ae_mode = false;

            stream_identifier stream_id;
            if (m_version == legacy_file_format::file_version())
            {
                //Version 1 legacy
                stream_id = legacy_file_format::get_stream_identifier(image_data.getTopic());
                get_legacy_frame_metadata(m_file, stream_id, image_data, additional_data);
            }
            else
            {
                //Version 2 and above
                stream_id = ros_topic::get_stream_identifier(image_data.getTopic());
                auto info_topic = ros_topic::frame_metadata_topic(stream_id);
                get_frame_metadata(m_file, info_topic, stream_id, image_data, additional_data);
            }

            frame_interface* frame = m_frame_source->alloc_frame((stream_id.stream_type == RS2_STREAM_DEPTH) ? RS2_EXTENSION_DEPTH_FRAME : RS2_EXTENSION_VIDEO_FRAME,
                msg->data.size(), additional_data, true);
            if (frame == nullptr)
            {
                throw invalid_value_exception("Failed to allocate new frame");
            }
            librealsense::video_frame* video_frame = static_cast<librealsense::video_frame*>(frame);
            video_frame->assign(msg->width, msg->height, msg->step, msg->step / msg->width * 8);
            rs2_format stream_format;
            convert(msg->encoding, stream_format);
            //attaching a temp stream to the frame. Playback sensor should assign the real stream
            frame->set_stream(std::make_shared<video_stream_profile>(platform::stream_profile{}));
            frame->get_stream()->set_format(stream_format);
            frame->get_stream()->set_stream_index(stream_id.stream_index);
            frame->get_stream()->set_stream_type(stream_id.stream_type);
            video_frame->data = std::move(msg->data);
            librealsense::frame_holder fh{ video_frame };
            LOG_DEBUG("Created image frame: " << stream_id << " " << video_frame->get_width() << "x" << video_frame->get_height() << " " << stream_format);

            return std::move(fh);
        }

        frame_holder create_motion_sample(const rosbag::MessageInstance &motion_data) const
        {
            LOG_DEBUG("Trying to create a motion frame from message");

            auto msg = instantiate_msg<sensor_msgs::Imu>(motion_data);

            frame_additional_data additional_data{};
            std::chrono::duration<double, std::milli> timestamp_ms(std::chrono::duration<double>(msg->header.stamp.toSec()));
            additional_data.timestamp = timestamp_ms.count();
            additional_data.frame_number = msg->header.seq;
            additional_data.fisheye_ae_mode = false; //TODO: where should this come from?

            stream_identifier stream_id;
            if (m_version == legacy_file_format::file_version())
            {
                //Version 1 legacy
                stream_id = legacy_file_format::get_stream_identifier(motion_data.getTopic());
                get_legacy_frame_metadata(m_file, stream_id, motion_data, additional_data);
            }
            else
            {
                //Version 2 and above
                stream_id = ros_topic::get_stream_identifier(motion_data.getTopic());
                auto info_topic = ros_topic::frame_metadata_topic(stream_id);
                get_frame_metadata(m_file, info_topic, stream_id, motion_data, additional_data);
            }

            frame_interface* frame = m_frame_source->alloc_frame(RS2_EXTENSION_MOTION_FRAME, 3 * sizeof(float), additional_data, true);
            if (frame == nullptr)
            {
                throw invalid_value_exception("Failed to allocate new frame");
            }
            librealsense::motion_frame* motion_frame = static_cast<librealsense::motion_frame*>(frame);
            //attaching a temp stream to the frame. Playback sensor should assign the real stream
            frame->set_stream(std::make_shared<motion_stream_profile>(platform::stream_profile{}));
            frame->get_stream()->set_format(RS2_FORMAT_MOTION_XYZ32F);
            frame->get_stream()->set_stream_index(stream_id.stream_index);
            frame->get_stream()->set_stream_type(stream_id.stream_type);
            if (stream_id.stream_type == RS2_STREAM_ACCEL)
            {
                auto data = reinterpret_cast<float*>(motion_frame->data.data());
                data[0] = static_cast<float>(msg->linear_acceleration.x);
                data[1] = static_cast<float>(msg->linear_acceleration.y);
                data[2] = static_cast<float>(msg->linear_acceleration.z);
            }
            else if (stream_id.stream_type == RS2_STREAM_GYRO)
            {
                auto data = reinterpret_cast<float*>(motion_frame->data.data());
                data[0] = static_cast<float>(msg->angular_velocity.x);
                data[1] = static_cast<float>(msg->angular_velocity.y);
                data[2] = static_cast<float>(msg->angular_velocity.z);
            }
            else
            {
                throw io_exception(to_string() << "Unsupported stream type " << stream_id.stream_type);
            }
            librealsense::frame_holder fh{ motion_frame };
            LOG_DEBUG("Created motion frame: " << stream_id);

            return std::move(fh);
        }
        
        frame_holder create_pose_sample(const rosbag::MessageInstance &pose_data) const
        {
            LOG_DEBUG("Trying to create a pose frame from message");

            auto msg = instantiate_msg<realsense_legacy_msgs::pose>(pose_data);

            std::array<float,3> translation{ msg->translation.x,msg->translation.y, msg->translation.z };
            std::array<float,3> velocity{ msg->velocity.x,msg->velocity.y, msg->velocity.z };
            std::array<float,3> angular_velocity{ msg->angular_velocity.x,msg->angular_velocity.y, msg->angular_velocity.z };
            std::array<float,3> acceleration{ msg->acceleration.x,msg->acceleration.y, msg->acceleration.z };
            std::array<float,3> angular_acceleration{ msg->angular_acceleration.x,msg->angular_acceleration.y, msg->angular_acceleration.z };
            std::array<float, 4> rotation{ msg->rotation.x,msg->rotation.y, msg->rotation.z, msg->rotation.w };

            size_t frame_size = sizeof(translation) +  sizeof(velocity) +  sizeof(angular_velocity) +  sizeof(acceleration) +  sizeof(angular_acceleration) + sizeof(rotation);
            
            rs2_extension frame_type = RS2_EXTENSION_POSE_FRAME;
            frame_additional_data additional_data{};
            std::chrono::duration<double, std::milli> timestamp_ms(static_cast<double>(msg->timestamp));
            additional_data.timestamp = timestamp_ms.count();
            additional_data.frame_number = 0; //No support for frame numbers
            additional_data.fisheye_ae_mode = false;
            
            stream_identifier stream_id;
            if (m_version == legacy_file_format::file_version())
            {
                //Version 1 legacy
                stream_id = legacy_file_format::get_stream_identifier(pose_data.getTopic());
                get_legacy_frame_metadata(m_file, stream_id, pose_data, additional_data);
            }
            else
            {
                //Version 2 and above
                stream_id = ros_topic::get_stream_identifier(pose_data.getTopic());
                auto info_topic = ros_topic::frame_metadata_topic(stream_id);
                get_frame_metadata(m_file, info_topic, stream_id, pose_data, additional_data);
            }
            frame_interface* new_frame = m_frame_source->alloc_frame(frame_type, frame_size, additional_data, true);
            librealsense::pose_frame* pose_frame = static_cast<librealsense::pose_frame*>(new_frame);
            //attaching a temp stream to the frame. Playback sensor should assign the real stream
            new_frame->set_stream(std::make_shared<stream_profile_base>(platform::stream_profile{}));
            new_frame->get_stream()->set_format(RS2_FORMAT_6DOF);
            new_frame->get_stream()->set_stream_index(stream_id.stream_index);
            new_frame->get_stream()->set_stream_type(stream_id.stream_type);
            byte* data = pose_frame->data.data();
            memcpy(data, &translation, sizeof(translation));
            data += sizeof(translation);
            memcpy(data, &velocity, sizeof(velocity));
            data += sizeof(velocity);
            memcpy(data, &angular_velocity, sizeof(angular_velocity));
            data += sizeof(angular_velocity);
            memcpy(data, &acceleration, sizeof(acceleration));
            data += sizeof(acceleration);
            memcpy(data, &angular_acceleration, sizeof(angular_acceleration));
            data += sizeof(angular_acceleration);
            memcpy(data, &rotation, sizeof(rotation));

            frame_holder fh{ new_frame };
            LOG_DEBUG("Created new frame " << frame_type);
            return std::move(fh);
        }

        static uint32_t read_file_version(const rosbag::Bag& file)
        {
            auto version_topic = ros_topic::file_version_topic();
            rosbag::View view(file, rosbag::TopicQuery(version_topic));

            auto legacy_version_topic = legacy_file_format::file_version_topic();
            rosbag::View legacy_view(file, rosbag::TopicQuery(legacy_version_topic));
            if(legacy_view.size() == 0 && view.size() == 0)
            {
                throw io_exception(to_string() << "Invalid file format, file does not contain topic \"" << version_topic << "\" nor \"" << legacy_version_topic << "\"");
            }
            assert((view.size() + legacy_view.size()) == 1); //version message is expected to be a single one
            if (view.size() != 0)
            {
                auto item = *view.begin();
                auto msg = instantiate_msg<std_msgs::UInt32>(item);
                if (msg->data < get_minimum_supported_file_version())
                {
                    throw std::runtime_error(to_string() << "Unsupported file version \"" << msg->data << "\"");
                }
                return msg->data;
            }
            else if (legacy_view.size() != 0)
            {
                auto item = *legacy_view.begin();
                auto msg = instantiate_msg<std_msgs::UInt32>(item);
                if (msg->data > legacy_file_format::get_maximum_supported_legacy_file_version())
                {
                    throw std::runtime_error(to_string() << "Unsupported legacy file version \"" << msg->data << "\"");
                }
                return msg->data;
            }
            throw std::logic_error("Unreachable code path");
        }
        bool try_read_legacy_stream_extrinsic(const stream_identifier& stream_id, uint32_t& group_id, rs2_extrinsics& extrinsic) const
        {
            return false; //TODO: Implement
        }
        bool try_read_stream_extrinsic(const stream_identifier& stream_id, uint32_t& group_id, rs2_extrinsics& extrinsic) const
        {
            if (m_version == legacy_file_format::file_version())
                return try_read_legacy_stream_extrinsic(stream_id, group_id, extrinsic);

            rosbag::View tf_view(m_file, ExtrinsicsQuery(stream_id));
            if (tf_view.size() == 0)
            {
                return false;
            }
            assert(tf_view.size() == 1); //There should be 1 message per stream
            auto msg = *tf_view.begin();
            auto tf_msg = instantiate_msg<geometry_msgs::Transform>(msg);
            group_id = ros_topic::get_extrinsic_group_index(msg.getTopic());
            convert(*tf_msg, extrinsic);
            return true;
        }

        static void update_sensor_options(const rosbag::Bag& file, uint32_t sensor_index, const nanoseconds& time, uint32_t file_version, snapshot_collection& sensor_extensions, uint32_t version)
        {
            if (version == legacy_file_format::file_version())
            {
                return; //TODO: Implement
            }
            auto sensor_options = read_sensor_options(file, { get_device_index(), sensor_index }, time, file_version);
            sensor_extensions[RS2_EXTENSION_OPTIONS] = sensor_options;
            if (sensor_options->supports_option(RS2_OPTION_DEPTH_UNITS))
            {
                auto&& dpt_opt = sensor_options->get_option(RS2_OPTION_DEPTH_UNITS);
                sensor_extensions[RS2_EXTENSION_DEPTH_SENSOR] = std::make_shared<depth_sensor_snapshot>(dpt_opt.query());
            }
        }

        device_snapshot read_device_description(const nanoseconds& time, bool reset = false)
        {
            if (time == get_static_file_info_timestamp())
            {
                if (reset)
                {
                    snapshot_collection device_extensions;

                    auto info = read_info_snapshot(ros_topic::device_info_topic(get_device_index()));
                    device_extensions[RS2_EXTENSION_INFO] = info;

                    std::vector<sensor_snapshot> sensor_descriptions;
                    auto sensor_indices = read_sensor_indices(get_device_index());
                    std::map<stream_identifier, std::pair<uint32_t, rs2_extrinsics>> extrinsics_map;

                    for (auto sensor_index : sensor_indices)
                    {
                        snapshot_collection sensor_extensions;
                        auto streams_snapshots = read_stream_info(get_device_index(), sensor_index);
                        for (auto stream_profile : streams_snapshots)
                        {
                            auto stream_id = stream_identifier{ get_device_index(), sensor_index, stream_profile->get_stream_type(), static_cast<uint32_t>(stream_profile->get_stream_index()) };
                            uint32_t reference_id;
                            rs2_extrinsics stream_extrinsic;
                            if (try_read_stream_extrinsic(stream_id, reference_id, stream_extrinsic))
                            {
                                extrinsics_map[stream_id] = std::make_pair(reference_id, stream_extrinsic);
                            }
                        }
                        
                        //Update infos
                        std::shared_ptr<info_container> sensor_info;
                        if (m_version == legacy_file_format::file_version())
                        {
                            sensor_info = read_legacy_info_snapshot(sensor_index);
                        }
                        else
                        {
                            sensor_info = read_info_snapshot(ros_topic::sensor_info_topic({ get_device_index(), sensor_index }));
                        }
                        sensor_extensions[RS2_EXTENSION_INFO] = sensor_info;
                        //Update options
                        update_sensor_options(m_file, sensor_index, time, m_version, sensor_extensions, m_version);

                        sensor_descriptions.emplace_back(sensor_index, sensor_extensions, streams_snapshots);
                    }

                    m_initial_device_description = device_snapshot(device_extensions, sensor_descriptions, extrinsics_map);
                }
                return m_initial_device_description;
            }
            else
            {
                //update only:
                auto device_snapshot = m_initial_device_description;
                for (auto& sensor : device_snapshot.get_sensors_snapshots())
                {
                    auto& sensor_extensions = sensor.get_sensor_extensions_snapshots();
                    update_sensor_options(m_file, sensor.get_sensor_index(), time, m_version, sensor_extensions, m_version);
                }
                return device_snapshot;
            }
        }
        
        std::shared_ptr<info_container> read_legacy_info_snapshot(uint32_t sensor_index) const
        {
            std::map<rs2_camera_info, std::string> values;
            rosbag::View view(m_file, rosbag::TopicQuery(to_string() <<"/info/" << sensor_index));
            auto infos = std::make_shared<info_container>();
            //TODO: properly implement, currently assuming TM2 devices
            infos->register_info(RS2_CAMERA_INFO_NAME, to_string() << "Sensor " << sensor_index);
            for (auto message_instance : view)
            {
                auto info_msg = instantiate_msg<realsense_legacy_msgs::vendor_data>(message_instance);
                try
                {
                    rs2_camera_info info;
                    if(legacy_file_format::info_from_string(info_msg->name, info))
                    {
                        infos->register_info(info, info_msg->value);
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what() << std::endl;
                }
            }

            return infos;
        }
        std::shared_ptr<info_container> read_info_snapshot(const std::string& topic) const
        {
            auto infos = std::make_shared<info_container>();
            if (m_version == legacy_file_format::file_version())
            {
                //TODO: properly implement, currently assuming TM2 devices and Movidius PID
                infos->register_info(RS2_CAMERA_INFO_NAME, "Intel RealSense TM2");
                infos->register_info(RS2_CAMERA_INFO_PRODUCT_ID, "2150");
                infos->register_info(RS2_CAMERA_INFO_SERIAL_NUMBER, "N/A");
            }
            std::map<rs2_camera_info, std::string> values;
            rosbag::View view(m_file, rosbag::TopicQuery(topic));
            for (auto message_instance : view)
            {
                diagnostic_msgs::KeyValueConstPtr info_msg = instantiate_msg<diagnostic_msgs::KeyValue>(message_instance);
                try
                {
                    rs2_camera_info info;
                    convert(info_msg->key, info);
                    infos->register_info(info, info_msg->value);
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what() << std::endl;
                }
            }

            return infos;
        }

        std::set<uint32_t> read_sensor_indices(uint32_t device_index) const
        {
            std::set<uint32_t> sensor_indices;
            if (m_version == legacy_file_format::file_version())
            {
                rosbag::View device_info(m_file, rosbag::TopicQuery("/info/4294967295"));
                if (device_info.size() == 0)
                {
                    throw io_exception("Missing sensor count message for legacy file");
                }
                for (auto info : device_info)
                {
                    auto msg = instantiate_msg<realsense_legacy_msgs::vendor_data>(info);
                    if (msg->name == "sensor_count")
                    {
                        int sensor_count = std::stoi(msg->value);
                        while(--sensor_count >= 0)
                            sensor_indices.insert(sensor_count);
                    }
                }
            }
            else
            {
                rosbag::View sensor_infos(m_file, SensorInfoQuery(device_index));
                for (auto sensor_info : sensor_infos)
                {
                    auto msg = instantiate_msg<diagnostic_msgs::KeyValue>(sensor_info);
                    sensor_indices.insert(static_cast<uint32_t>(ros_topic::get_sensor_index(sensor_info.getTopic())));
                }
            }
            return sensor_indices;
        }
        static std::shared_ptr<video_stream_profile> create_video_stream_profile(const platform::stream_profile& sp, 
                                                                                 const sensor_msgs::CameraInfo& ci, 
                                                                                 const stream_descriptor& sd)
        {
            auto profile = std::make_shared<video_stream_profile>(sp);
            rs2_intrinsics intrinsics{};
            intrinsics.height = ci.height;
            intrinsics.width = ci.width;
            intrinsics.fx = ci.K[0];
            intrinsics.ppx = ci.K[2];
            intrinsics.fy = ci.K[4];
            intrinsics.ppy = ci.K[5];
            memcpy(intrinsics.coeffs, ci.D.data(), sizeof(intrinsics.coeffs));
            profile->set_intrinsics([intrinsics]() {return intrinsics; });
            profile->set_stream_index(sd.index);
            profile->set_stream_type(sd.type);
            profile->set_dims(ci.width, ci.height);
            profile->set_format(static_cast<rs2_format>(sp.format));
            profile->set_framerate(sp.fps);
            return profile;
        }

        stream_profiles read_legacy_stream_info(uint32_t sensor_index) const
        {
            //legacy files have the form of "/(camera|imu)/<stream type><stream index>/(image_imu)_raw/<sensor_index>
            //6DoF streams have no streaming profiles in the file - handling them seperatly
            stream_profiles streams;
            rosbag::View stream_infos_view(m_file, RegexTopicQuery(to_string() << R"RRR(/camera/(rs_stream_info|rs_motion_stream_info)/)RRR" << sensor_index));
            for (auto infos_msg : stream_infos_view)
            {
                if (infos_msg.isType<realsense_legacy_msgs::motion_stream_info>())
                {
                    //TODO: implement
                    auto motion_stream_info_msg = instantiate_msg<realsense_legacy_msgs::motion_stream_info>(infos_msg);
                    auto fps = motion_stream_info_msg->fps;
                    
                    std::string stream_name = motion_stream_info_msg->motion_type;
                    stream_descriptor stream_id = legacy_file_format::parse_stream_type(stream_name);

                    //TODO: motion_stream_info_msg->stream_extrinsics;
                    //TODO: motion_stream_info_msg->stream_intrinsics;
                    rs2_format format = RS2_FORMAT_MOTION_XYZ32F;
                    auto profile = std::make_shared<motion_stream_profile>(platform::stream_profile{ 0, 0, fps, static_cast<uint32_t>(format) });
                    profile->set_stream_index(stream_id.index);
                    profile->set_stream_type(stream_id.type);
                    profile->set_format(format);
                    profile->set_framerate(fps);
                    streams.push_back(profile);
                }
                else if (infos_msg.isType<realsense_legacy_msgs::stream_info>())
                {
                    auto stream_info_msg = instantiate_msg<realsense_legacy_msgs::stream_info>(infos_msg);
                    auto fps = stream_info_msg->fps;
                    rs2_format format;
                    convert(stream_info_msg->encoding, format);
                    std::string stream_name = stream_info_msg->stream_type;
                    stream_descriptor stream_id = legacy_file_format::parse_stream_type(stream_name);
                    auto profile = create_video_stream_profile(platform::stream_profile{ stream_info_msg->camera_info.width,
                        stream_info_msg->camera_info.height, fps, static_cast<uint32_t>(format) }, 
                        stream_info_msg->camera_info, 
                        stream_id);
                    streams.push_back(profile);
                }
                else
                {
                    throw io_exception(to_string()
                        << "Invalid file format, expected "
                        << ros::message_traits::DataType<realsense_legacy_msgs::motion_stream_info>::value()
                        << " or " << ros::message_traits::DataType<realsense_legacy_msgs::stream_info>::value()
                        << " message but got: " << infos_msg.getDataType()
                        << "(Topic: " << infos_msg.getTopic() << ")");
                }
            }
            std::unique_ptr<rosbag::View> entire_bag = std::unique_ptr<rosbag::View>(new rosbag::View(m_file, rosbag::View::TrueQuery()));
            std::vector<uint32_t> indices;
            for (auto&& topic : get_topics(entire_bag))
            {
                std::regex r(R"RRR(/camera/rs_6DoF(\d+)/\d+)RRR");
                std::smatch sm;
                if(std::regex_search(topic, sm, r))
                {
                    for (int i = 1; i<sm.size(); i++)
                    {
                        indices.push_back(std::stoul(sm[i].str()));
                    }
                }
            }
            for (auto&& index : indices)
            {
                stream_descriptor stream_id{ RS2_STREAM_POSE, static_cast<int>(index) };
                rs2_format format = RS2_FORMAT_6DOF;
                auto profile = std::make_shared<stream_profile_base>(platform::stream_profile{ 0, 0, 0, static_cast<uint32_t>(format) });
                profile->set_stream_index(stream_id.index);
                profile->set_stream_type(stream_id.type);
                profile->set_format(format);
                profile->set_framerate(0);
                streams.push_back(profile);
            }
            return streams;
        }
        
        stream_profiles read_stream_info(uint32_t device_index, uint32_t sensor_index) const
        {
            if (m_version == legacy_file_format::file_version())
                return read_legacy_stream_info(sensor_index);
            stream_profiles streams;
            //The below regex matches both stream info messages and also video \ imu stream info (both have the same prefix)
            rosbag::View stream_infos_view(m_file, RegexTopicQuery("/device_" + std::to_string(device_index) + "/sensor_" + std::to_string(sensor_index) + R"RRR(/(\w)+_(\d)+/info)RRR"));
            for (auto infos_view : stream_infos_view)
            {
                stream_identifier stream_id;
                if (m_version == legacy_file_format::file_version())
                {
                    stream_id = legacy_file_format::get_stream_identifier(infos_view.getTopic());
                }
                else
                {
                    stream_id = ros_topic::get_stream_identifier(infos_view.getTopic());
                }
                if (infos_view.isType<realsense_msgs::StreamInfo>() == false)
                {
                    continue;
                }

                auto stream_info_msg = instantiate_msg<realsense_msgs::StreamInfo>(infos_view);
                //auto is_recommended = stream_info_msg->is_recommended;
                auto fps = stream_info_msg->fps;
                rs2_format format;
                convert(stream_info_msg->encoding, format);

                auto video_stream_topic = ros_topic::video_stream_info_topic(stream_id);
                rosbag::View video_stream_infos_view(m_file, rosbag::TopicQuery(video_stream_topic));
                if (video_stream_infos_view.size() > 0)
                {
                    assert(video_stream_infos_view.size() == 1);
                    auto video_stream_msg_ptr = *video_stream_infos_view.begin();
                    auto video_stream_msg = instantiate_msg<sensor_msgs::CameraInfo>(video_stream_msg_ptr);
                    auto profile = create_video_stream_profile(platform::stream_profile{ video_stream_msg->width ,video_stream_msg->height, fps, static_cast<uint32_t>(format) }
                    , *video_stream_msg, { stream_id.stream_type, static_cast<int>(stream_id.stream_index)});
                    streams.push_back(profile);
                }

                auto imu_stream_topic = ros_topic::imu_intrinsic_topic(stream_id);
                rosbag::View imu_intrinsic_view(m_file, rosbag::TopicQuery(imu_stream_topic));
                if (imu_intrinsic_view.size() > 0)
                {
                    assert(imu_intrinsic_view.size() == 1);
                    //TODO: implement when relevant
                }

                if (video_stream_infos_view.size() == 0 && imu_intrinsic_view.size() == 0)
                {
                    throw io_exception(to_string() << "Every StreamInfo is expected to have a complementary video/imu message, but none was found");
                }

            }
            return streams;
        }

        static std::string read_option_description(const rosbag::Bag& file, const std::string& topic)
        {
            rosbag::View option_description_view(file, rosbag::TopicQuery(topic));
            if (option_description_view.size() == 0)
            {
                LOG_ERROR("File does not contain topics for: " << topic);
                return "N/A";
            }
            assert(option_description_view.size() == 1); //There should be only 1 message for each option
            auto description_message_instance = *option_description_view.begin();
            auto option_desc_msg = instantiate_msg<std_msgs::String>(description_message_instance);
            return option_desc_msg->data;
        }
        
        /*Until Version 2 (including)*/
        static std::pair<rs2_option, std::shared_ptr<librealsense::option>> create_property(const rosbag::MessageInstance& property_message_instance)
        {
            auto property_msg = instantiate_msg<diagnostic_msgs::KeyValue>(property_message_instance);
            rs2_option id;
            convert(property_msg->key, id);
            float value = std::stof(property_msg->value);
            std::string description = to_string() << "Read only option of " << id;
            return std::make_pair(id, std::make_shared<const_value_option>(description, value));
        }

        /*Starting version 3*/
        static std::pair<rs2_option, std::shared_ptr<librealsense::option>> create_option(const rosbag::Bag& file, const rosbag::MessageInstance& value_message_instance)
        {
            auto option_value_msg = instantiate_msg<std_msgs::Float32>(value_message_instance);
            std::string option_name = ros_topic::get_option_name(value_message_instance.getTopic());
            device_serializer::sensor_identifier sensor_id = ros_topic::get_sensor_identifier(value_message_instance.getTopic());
            rs2_option id;
            convert(option_name, id);
            float value = option_value_msg->data;
            std::string description = read_option_description(file, ros_topic::option_description_topic(sensor_id, id));
            return std::make_pair(id, std::make_shared<const_value_option>(description, value));
        }

        static std::shared_ptr<options_container> read_sensor_options(const rosbag::Bag& file, device_serializer::sensor_identifier sensor_id, const nanoseconds& timestamp, uint32_t file_version)
        {
            auto options = std::make_shared<options_container>();
            if (file_version == 2)
            {
                rosbag::View sensor_options_view(file, rosbag::TopicQuery(ros_topic::property_topic(sensor_id)));
                for (auto message_instance : sensor_options_view)
                {
                    auto id_option = create_property(message_instance);
                    options->register_option(id_option.first, id_option.second);
                }
            }
            else
            {
                //Taking all messages from the beginning of the bag until the time point requested
                for (int i = 0; i < static_cast<int>(RS2_OPTION_COUNT); i++)
                {
                    rs2_option id = static_cast<rs2_option>(i);
                    std::string option_topic = ros_topic::option_value_topic(sensor_id, id);
                    rosbag::View option_view(file, rosbag::TopicQuery(option_topic), to_rostime(get_static_file_info_timestamp()), to_rostime(timestamp));
                    auto it = option_view.begin();
                    if (it == option_view.end())
                    {
                        continue;
                    }
                    rosbag::View::iterator last_item;
                    while (it != option_view.end())
                    {
                        last_item = it++;
                    }
                    auto option = create_option(file, *last_item);
                    assert(id == option.first);
                    options->register_option(option.first, option.second);
                }
            }
            return options;
        }

        static std::vector<std::string> get_topics(std::unique_ptr<rosbag::View>& view)
        {
            std::vector<std::string> topics;
            if(view != nullptr)
            {
                auto connections = view->getConnections();
                std::transform(connections.begin(), connections.end(), std::back_inserter(topics), [](const rosbag::ConnectionInfo* connection) { return connection->topic; });
            }
            return topics;
        }

        device_snapshot                         m_initial_device_description;
        nanoseconds                             m_total_duration;
        std::string                             m_file_path;
        std::shared_ptr<frame_source>           m_frame_source;
        rosbag::Bag                             m_file;
        std::unique_ptr<rosbag::View>           m_samples_view;
        rosbag::View::iterator                  m_samples_itrator;
        std::vector<std::string>                m_enabled_streams_topics;
        std::shared_ptr<metadata_parser_map>    m_metadata_parser_map;
        std::shared_ptr<context>                m_context;
        uint32_t                                m_version;
    };
}
