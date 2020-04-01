#include "inferer.h"

namespace retinanet
{
Inferer::Inferer(std::string const &model_path, cv::Mat const &first_f)
    : engine_(model_path), input_size_(input_size_as_cv(engine_)),
      resized_(input_size_, first_f.type()), converted_(input_size_, CV_32FC3),
      normalized_(CHANNELS * input_size_.area()),
      max_detection_(engine_.getMaxDetections()),
      scores_(std::make_unique<float[]>(max_detection_)),
      boxes_(std::make_unique<float[]>(max_detection_ * 4)),
      classes_(std::make_unique<float[]>(max_detection_)),
      data_size_(normalized_.size() * sizeof(float))
{
    buffers_d_.resize(4);
    cudaMalloc(&buffers_d_[0], 3 * input_size_.area() * sizeof(float));
    cudaMalloc(&buffers_d_[1], max_detection_ * sizeof(float));
    cudaMalloc(&buffers_d_[2], max_detection_ * 4 * sizeof(float));
    cudaMalloc(&buffers_d_[3], max_detection_ * sizeof(float));
}

Inferer::~Inferer()
{
    for (auto cuda_buf : buffers_d_)
    {
        cudaFree(cuda_buf);
    }
}

void Inferer::run(cv::Mat const &raw)
{
    cv::resize(raw, resized_, input_size_);
    resized_.convertTo(converted_, CV_32FC3, 1.0 / 255, 0);

    auto in = reinterpret_cast<float const *>(converted_.datastart);
    for (int j = 0, hw = input_size_.area(); j < hw; ++j)
    {
        for (int c = 0; c < CHANNELS; ++c)
        {
            normalized_[c * hw + j] = (in[CHANNELS * j + 2 - c] - mean_[c]) / std_[c];
        }
    }

    cudaMemcpy(
        buffers_d_.front(), normalized_.data(), data_size_, cudaMemcpyHostToDevice);
    engine_.infer(buffers_d_);

    cudaMemcpy(scores_.get(),
               buffers_d_[1],
               sizeof(float) * max_detection_,
               cudaMemcpyDeviceToHost);
    cudaMemcpy(boxes_.get(),
               buffers_d_[2],
               sizeof(float) * max_detection_ * 4,
               cudaMemcpyDeviceToHost);
    cudaMemcpy(classes_.get(),
               buffers_d_[3],
               sizeof(float) * max_detection_,
               cudaMemcpyDeviceToHost);
}

cv::Size Inferer::input_size_as_cv(retinanet::Engine const &e)
{
    auto input_size = engine_.getInputSize();
    return cv::Size{input_size[1], input_size[0]};
}
} // namespace retinanet
