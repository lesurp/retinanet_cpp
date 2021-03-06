/*
 * Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <string>
#include <vector>

#include <NvInfer.h>

#include <cuda_runtime.h>

namespace retinanet {

// RetinaNet wrapper around TensorRT CUDA engine
class Engine {
public:
    // Create engine from engine path
    Engine(const std::string &engine_path, bool verbose=false);

    // Create engine from serialized onnx model
    Engine(const char *onnx_model, size_t onnx_size, size_t batch, std::string precision,
        float score_thresh, int top_n, const std::vector<std::vector<float>>& anchors,
        float nms_thresh, int detections_per_im, const std::vector<std::string>& calibration_files,
        std::string model_name, std::string calibration_table, bool verbose, size_t workspace_size=(1ULL << 30));

    ~Engine();

    // Save model to path
    void save(const std::string &path);

    // Infer using pre-allocated GPU buffers {data, scores, boxes, classes}
    void infer(std::vector<void *> &buffers);

    // Get (h, w) size of the fixed input
    std::vector<int> getInputSize();

    // Get max allowed batch size
    int getMaxBatchSize();

    // Get max number of detections
    int getMaxDetections();

    // Get stride
    int getStride();

private:
    nvinfer1::IRuntime *_runtime = nullptr;
    nvinfer1::ICudaEngine *_engine = nullptr;
    nvinfer1::IExecutionContext *_context = nullptr;
    cudaStream_t _stream = nullptr;

    void _load(const std::string &path);
    void _prepare();

};

}
