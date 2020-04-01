#ifndef INFERER_H_MNL8ET2L
#define INFERER_H_MNL8ET2L

#include <cuda_runtime.h>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <retinanet/engine.h>
#include <string>
#include <vector>

namespace retinanet
{
class Inferer
{
  public:
    int const CHANNELS = 3;

    Inferer(std::string const &model_path, cv::Mat const &first_f);
    ~Inferer();
    void run(cv::Mat const &raw);

    cv::Mat const &resized() const { return resized_; }

    size_t max_detection() const { return max_detection_; }
    float const *scores() const { return scores_.get(); }
    float const *boxes() const { return boxes_.get(); }
    float const *classes() const { return classes_.get(); }

  private:
    // Buffers, used for data conversions
    retinanet::Engine engine_;
    cv::Size input_size_;
    cv::Mat resized_;
    cv::Mat converted_;
    std::vector<float> normalized_;
    std::vector<float> img_;
    size_t data_size_;
    int max_detection_;

    cv::Size input_size_as_cv(retinanet::Engine const &e);

    // mean / std of imagenet
    // used for normalization
    std::vector<float> mean_{0.485, 0.456, 0.406};
    std::vector<float> std_{0.229, 0.224, 0.225};

    // Buffers to cuda memory (both input and outputs)
    // The pointers are as follows:
    // data_d
    // scores_d
    // boxes_d
    // classes_d
    std::vector<void *> buffers_d_;

    std::unique_ptr<float[]> scores_;
    std::unique_ptr<float[]> boxes_;
    std::unique_ptr<float[]> classes_;
};
} // namespace retinanet

#endif /* end of include guard: INFERER_H_MNL8ET2L */
