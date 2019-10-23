// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2016 Intel Corporation. All Rights Reserved.

#include <mutex>
#include <chrono>
#include <vector>
#include <iterator>
#include <cstddef>

#include "device.h"
#include "context.h"
#include "image.h"
#include "metadata-parser.h"

#include "ds5-nonmonochrome.h"
#include "ds5-private.h"
#include "ds5-options.h"
#include "ds5-timestamp.h"
#include "proc/color-formats-converter.h"
#include "proc/depth-formats-converter.h"

namespace librealsense
{
    ds5_nonmonochrome::ds5_nonmonochrome(std::shared_ptr<context> ctx,
                                         const platform::backend_device_group& group)
        : device(ctx, group), ds5_device(ctx, group)
    {
        using namespace ds;

        auto pid = group.uvc_devices.front().pid;
        if ((_fw_version >= firmware_version("5.5.8.0")) && (pid != RS_USB2_PID))
        {
            auto& depth_ep = get_depth_sensor();
            depth_ep.register_option(RS2_OPTION_ENABLE_AUTO_WHITE_BALANCE,
                std::make_shared<uvc_xu_option<uint8_t>>(get_raw_depth_sensor(),
                                                         depth_xu,
                                                         DS5_ENABLE_AUTO_WHITE_BALANCE,
                                                         "Enable Auto White Balance"));

            // RS400 rolling-shutter Skus allow to get low-quality color image from the same viewport as the depth
            depth_ep.register_processing_block({ {RS2_FORMAT_UYVY} }, { {RS2_FORMAT_RGB8, RS2_STREAM_INFRARED} }, []() { return std::make_shared<uyvy_converter>(RS2_FORMAT_RGB8, RS2_STREAM_INFRARED); });
            depth_ep.register_processing_block({ {RS2_FORMAT_UYVY} }, { {RS2_FORMAT_RGBA8, RS2_STREAM_INFRARED} }, []() { return std::make_shared<uyvy_converter>(RS2_FORMAT_RGBA8, RS2_STREAM_INFRARED); });
            depth_ep.register_processing_block({ {RS2_FORMAT_UYVY} }, { {RS2_FORMAT_BGR8, RS2_STREAM_INFRARED} }, []() { return std::make_shared<uyvy_converter>(RS2_FORMAT_BGR8, RS2_STREAM_INFRARED); });
            depth_ep.register_processing_block({ {RS2_FORMAT_UYVY} }, { {RS2_FORMAT_BGRA8, RS2_STREAM_INFRARED} }, []() { return std::make_shared<uyvy_converter>(RS2_FORMAT_BGRA8, RS2_STREAM_INFRARED); });
            depth_ep.register_processing_block(processing_block_factory::create_id_pbf(RS2_FORMAT_UYVY, RS2_STREAM_INFRARED));

            depth_ep.register_processing_block({ {RS2_FORMAT_BGR8} }, { {RS2_FORMAT_RGB8, RS2_STREAM_INFRARED} }, []() { return std::make_shared<bgr_to_rgb>(); });

            depth_ep.register_processing_block(processing_block_factory::create_id_pbf(RS2_FORMAT_Z16, RS2_STREAM_DEPTH));
            depth_ep.register_processing_block({ {RS2_FORMAT_W10} }, { {RS2_FORMAT_RAW10, RS2_STREAM_INFRARED, 1} }, []() { return std::make_shared<w10_converter>(RS2_FORMAT_RAW10); });
            depth_ep.register_processing_block({ {RS2_FORMAT_W10} }, { {RS2_FORMAT_Y10BPACK, RS2_STREAM_INFRARED, 1} }, []() { return std::make_shared<w10_converter>(RS2_FORMAT_Y10BPACK); });

        }

        get_depth_sensor().unregister_option(RS2_OPTION_EMITTER_ON_OFF);

        if ((_fw_version >= firmware_version("5.9.13.6") &&
             _fw_version < firmware_version("5.9.15.1")))
        {
            get_depth_sensor().register_option(RS2_OPTION_INTER_CAM_SYNC_MODE,
                std::make_shared<external_sync_mode>(*_hw_monitor));
        }
    }
}
