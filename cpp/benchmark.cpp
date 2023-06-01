#include "cpp/dbscan.hpp"

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {

auto read_points(std::string const& filename) -> std::vector<dbscan::Dbscan::Point>
{
    std::vector<dbscan::Dbscan::Point> points{};
    std::ifstream fin{filename};
    while (!fin.eof()) {
        float x, y;
        fin >> x >> y;
        points.push_back({x, y});
    }
    return points;
}

auto gen_points()
{
    constexpr auto seed{3812};
    std::mt19937 rd{seed};
    std::uniform_real_distribution dis{0.0, 1.0};

    std::vector<std::pair<dbscan::Dbscan::Label, dbscan::Dbscan::Point>> labels_and_points{};
    for (auto i{0UL}; i < 500; ++i) {
        float const mid_x = 400.0f * dis(rd);
        float const mid_y = -250.0f + 500.f * dis(rd);

        constexpr auto width = 5.f;

        std::uniform_int_distribution num_points_dis{30, 500};
        auto const num_points{num_points_dis(rd)};
        for (auto j{0}; j < num_points; ++j) {
            float const x = mid_x - 0.5f * width * dis(rd);
            float const y = mid_y - 0.5f * width * dis(rd);
            labels_and_points.push_back(std::make_pair(i, dbscan::Dbscan::Point{x, y}));
        }
    }
    return labels_and_points;
}

}  // namespace

int main(int argc, char** argv)
{
    // if (argc < 2 || std::string{argv[1]} == "--help") {
    //     std::cerr << "usage: " << argv[0] << " FILE" << std::endl;
    //     std::cerr << "where FILE is a text file with 2-D points, one point per line" << std::endl;
    //     return 1;
    // }
    // std::string const filename{argv[1]};
    // if (!std::filesystem::exists(filename)) {
    //     std::cerr << "file doesn't exist: " << filename << std::endl;
    //     return 1;
    // }
    // auto const points{read_points(filename)};
    auto labels_and_points{gen_points()};
    std::vector<dbscan::Dbscan::Point> points{};
    points.reserve(std::size(labels_and_points));
    for (auto const& lp : labels_and_points) {
        points.push_back(lp.second);
    }
    std::ofstream out{"/home/ahans/src/dbscan/2d.txt"};
    for (auto const& p : points) {
        out << p[0] << "," << p[1] << std::endl;
    }
    std::cout << "have " << std::size(points) << " points" << std::endl;
    // dbscan::Dbscan dbscan{0.8, 10, std::size(points)};
    // ankerl::nanobench::Bench().run("fit_predict", [&dbscan, &points] {
    //     auto labels{dbscan.fit_predict(points)};
    //     ankerl::nanobench::doNotOptimizeAway(labels);
    // });
}
