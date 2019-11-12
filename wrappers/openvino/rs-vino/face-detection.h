// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2019 Intel Corporation. All Rights Reserved.

#pragma once

#include "base-detection.h"


namespace openvino_helpers
{
    /*
        Detects face bounding boxes in an image.


    */
    struct face_detection : public base_detection
    {
    public:
        struct Result
        {
            int label;
            float confidence;
            cv::Rect location;
        };

    private:
        // User arguments via the ctor
        double _detection_threshold;

        // Intermediates and helpers
        std::string _input_layer_name;
        std::string _output_layer_name;
        size_t _max_results;
        int _n_enqued_frames;
        float _width;                     // of the queued image
        float _height;

    public:
        face_detection( const std::string &pathToModel,
            double detectionThreshold,
            bool isAsync = true, 
            int maxBatch = 1, bool isBatchDynamic = false,
            bool doRawOutputMessages = false );

        InferenceEngine::CNNNetwork read_network() override;
        void submit_request() override;

        void enqueue( const cv::Mat &frame );
        std::vector< Result > fetch_results();
    };
}
