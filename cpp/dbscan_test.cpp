#include "cpp/dbscan.hpp"
#include "cpp/scoped_timer.hpp"

#include "doctest/doctest.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <unordered_map>

TEST_CASE("basic synthetic data test")
{
    constexpr auto seed{3812};
    std::mt19937 rd{seed};
    std::uniform_real_distribution dis{0.0, 1.0};

    std::vector<std::pair<dbscan::Dbscan::Label, dbscan::Dbscan::Point>> labels_and_points{};
    constexpr auto num_clusters{400};
    for (auto i{0UL}; i < num_clusters; ++i) {
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

    std::shuffle(std::begin(labels_and_points), std::end(labels_and_points), rd);
    std::vector<std::vector<int>> label_index_map{num_clusters};
    for (auto i{0UL}; i < std::size(labels_and_points); ++i) {
        auto const label{labels_and_points[i].first};
        label_index_map.at(label).push_back(i);
    }

    std::cout << "have " << std::size(labels_and_points) << " points" << std::endl;

    std::vector<dbscan::Dbscan::Point> points{};
    points.reserve(std::size(labels_and_points));
    for (auto const& lp : labels_and_points) {
        points.push_back(lp.second);
    }
    dbscan::Dbscan dbscan{0.8, 10, 200'000};
    ScopedTimer timer{};
    auto const labels = dbscan.fit_predict(points);
    std::cout << "took " << (timer.elapsed_ns() / 1000) << " µs" << std::endl;

    std::unordered_map<dbscan::Dbscan::Label, size_t> counts{};
    for (auto label : labels) counts[label]++;
    // for (auto [key, count] : counts) {
    //     std::cout << key << ": " << count << std::endl;
    // }

    auto num_above_threshold{0};
    constexpr double threshold{0.9};
    for (auto label{0}; label < num_clusters; ++label) {
        std::unordered_map<int, int> label_count;
        for (auto point_index : label_index_map[label]) {
            if (labels[point_index] >= 0) {
                label_count[labels[point_index]] += 1;
            }
        }
        int total_points{};
        int max_points{0};
        for (auto const [_, count] : label_count) {
            total_points += count;
            max_points = std::max(max_points, count);
        }
        REQUIRE(total_points > 0);
        auto ratio = static_cast<double>(max_points) / static_cast<double>(total_points);
        CHECK(ratio > 0.7);
        if (ratio > threshold) {
            num_above_threshold += 1;
        }
    }
    CHECK(num_above_threshold >= num_clusters - 10);
}