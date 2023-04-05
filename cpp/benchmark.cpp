#include "cpp/dbscan.hpp"

#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

#include <filesystem>
#include <fstream>
#include <iostream>
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

}  // namespace

int main(int argc, char** argv)
{
    if (argc < 2 || std::string{argv[1]} == "--help") {
        std::cerr << "usage: " << argv[0] << " FILE" << std::endl;
        std::cerr << "where FILE is a text file with 2-D points, one point per line" << std::endl;
        return 1;
    }
    std::string const filename{argv[1]};
    if (!std::filesystem::exists(filename)) {
        std::cerr << "file doesn't exist: " << filename << std::endl;
        return 1;
    }
    auto const points{read_points(filename)};
    dbscan::Dbscan dbscan{0.01, 20, std::size(points)};
    ankerl::nanobench::Bench().run("fit_predict", [&dbscan, &points] {
        auto labels{dbscan.fit_predict(points)};
        ankerl::nanobench::doNotOptimizeAway(labels);
    });
}
