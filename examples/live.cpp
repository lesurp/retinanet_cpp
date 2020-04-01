#include "inferer.h"
#include <chrono>
#include <cstdlib>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <variant>

using CliSettings = std::tuple<std::string, std::string, std::string>;

std::variant<std::string, CliSettings> parse_cli_impl(int argc, char **argv)
{
    std::optional<std::string> device;
    std::optional<std::string> labels_path;
    std::optional<std::string> model_path;

    std::stringstream ss;
    for (int i = 0; i < argc; ++i)
    {
        auto arg = std::string_view(argv[i]);

        if (arg.front() != '-')
        {
            continue;
        }
        if (i + 1 >= argc)
        {
            ss << "Unfinished parameter list: expected value after '" << arg << '\'';
            return ss.str();
        }

        // in case long version is given
        int offset;
        if (arg.length() > 1 && arg.at(1) == '-')
        {
            offset = 2;
        }
        else
        {
            offset = 1;
        }
        auto argument_key = arg.substr(offset);
        auto argument_value = std::string_view(argv[i + 1]);

        if (argument_key == "d" || argument_key == "device")
        {
            if (device)
            {
                ss << "Device already defined";
                return ss.str();
            }
            else
            {
                device = argument_value;
            }
        }
        else if (argument_key == "m" || argument_key == "model")
        {
            if (model_path)
            {
                ss << "Model path already defined";
                return ss.str();
            }
            else
            {
                model_path = argument_value;
            }
        }
        else if (argument_key == "l" || argument_key == "labels")
        {
            if (labels_path)
            {
                ss << "Labels path already defined";
                return ss.str();
            }
            else
            {
                labels_path = argument_value;
            }
        }
        i += 1;
    }

    if (!device || !model_path || !labels_path)
    {
        return "Missing values in CLI...";
    }

    return CliSettings{*device, *model_path, *labels_path};
}

auto parse_cli(int argc, char **argv)
{
    return std::visit(
        [](auto &&val) -> CliSettings {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, std::string>)
            {
                std::cerr << "Error parsing command line:\n\t" << val << std::endl;
                std::exit(1);
            }
            else if constexpr (std::is_same_v<T, CliSettings>)
            {
                std::cout << "\tDevice id:\t" << std::get<0>(val) << std::endl;
                std::cout << "\tModel path:\t" << std::get<1>(val) << std::endl;
                std::cout << "\tLabels path:\t" << std::get<2>(val) << std::endl;
                return val;
            }
        },
        parse_cli_impl(argc, argv));
}

class Labels
{
  public:
    static std::vector<std::string> from_disk(std::string const &labels_path)
    {
        std::ifstream f(labels_path);
        std::vector<std::string> labels;
        for (std::string label; std::getline(f, label);)
        {
            labels.emplace_back(std::move(label));
        }
        labels.shrink_to_fit();
        return labels;
    }
};

int main(int argc, char *argv[])
{
    auto [device, model_path, labels_path] = parse_cli(argc, argv);

    cv::VideoCapture camera{device};
    if (!camera.isOpened())
    {
        std::cerr << "Couldn't open camera\n";
        return 1;
    }

    cv::Mat raw;
    camera >> raw;
    retinanet::Inferer inferer(model_path, raw);
    auto labels = Labels::from_disk(labels_path);

    std::cout << "Max detection: " << inferer.max_detection() << std::endl;
    while (cv::waitKey(1) != 'q')
    {
        camera >> raw;
        auto start = std::chrono::steady_clock::now();
        inferer.run(raw);
        auto resized = inferer.resized().clone();
        for (int i = 0; i < inferer.max_detection(); i++)
        {
            auto score = inferer.scores()[i];
            if (score < 0.3f)
            {
                continue;
            }

            auto const *boxes = &inferer.boxes()[4 * i];
            float x1 = boxes[0];
            float y1 = boxes[1];
            float x2 = boxes[2];
            float y2 = boxes[3];

            auto top_left = cv::Point(x1, y1);
            auto bottom_right = cv::Point(x2, y2);
            auto bottom_left = cv::Point(x1, y2);
            auto color = cv::Scalar(0, 255, 0);
            auto const &class_label = labels[inferer.classes()[i]];

            // Draw bounding box on image
            cv::rectangle(resized, top_left, bottom_right, color);
            cv::putText(resized,
                        fmt::format("{:.2}% | {}", 100.0 * score, class_label),
                        bottom_left,
                        cv::FONT_HERSHEY_SIMPLEX,
                        1.0,
                        color);
        }
        auto duration = std::chrono::steady_clock::now() - start;
        std::cout
            << "Time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() - 1
            << "ms" << std::endl;
        cv::imshow("live with boxes", resized);
    }
    return 0;
}
