// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#pragma once

#include "align.h"
#include "types.h"


namespace librealsense
{
    std::shared_ptr<librealsense::align> create_align(rs2_stream align_to);

    class processing_block_factory
    {
    public:
        processing_block_factory() {};

        processing_block_factory(const std::vector<stream_profile>& from,
            const std::vector<stream_profile>& to,
            std::function<std::shared_ptr<processing_block>(void)> generate_func);

        bool operator==(const processing_block_factory& rhs) const;

        std::vector<stream_profile> get_source_info() const { return _source_info; }
        std::vector<stream_profile> get_target_info() const { return _target_info; }
        std::shared_ptr<processing_block> generate();
        
        static processing_block_factory create_id_pbf(rs2_format format, rs2_stream stream, int idx = 0);

        stream_profiles find_satisfied_requests(const stream_profiles& sp, const stream_profiles& supported_profiles) const;
        bool has_source(const std::shared_ptr<stream_profile_interface>& source) const;

    protected:
        std::vector<stream_profile> _source_info;
        std::vector<stream_profile> _target_info;
        std::function<std::shared_ptr<processing_block>(void)> generate_processing_block;
    };
}
