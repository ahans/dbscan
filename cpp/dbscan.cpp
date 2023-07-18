#include "cpp/dbscan.hpp"

#include <cmath>
#include <numeric>
#include <climits>
#include <unordered_map>

namespace dbscan {

Dbscan::Dbscan(float const eps, std::uint32_t const min_samples, std::size_t const num_points_hint)
    : eps_{eps}
    , eps_squared_{square(eps)}
    , min_samples_{min_samples}
{
    if (num_points_hint > 0) {
        labels_.reserve(num_points_hint);
        neighbors_.reserve(num_points_hint);
        visited_.reserve(num_points_hint);
        to_visit_.reserve(num_points_hint);
        counts_.reserve(num_points_hint);
        offsets_.reserve(num_points_hint);
    }
}

auto Dbscan::fit_predict(std::vector<Dbscan::Point> const& points) -> std::vector<Dbscan::Label>
{
    labels_.assign(std::size(points), undefined);
    visited_.assign(std::size(points), false);

    if (std::size(points) <= 1) {
        return labels_;
    }

    // calculate min_max of the current point cloud
    Dbscan::Point min{points[0]};
    Dbscan::Point max{points[0]};
    for (auto const& pt : points) {
        min[0] = std::min(min[0], pt[0]);
        min[1] = std::min(min[1], pt[1]);
        max[0] = std::max(max[0], pt[0]);
        max[1] = std::max(max[1], pt[1]);
    }

    // derive num_bins out of it
    float const range_x{max[0] - min[0]};
    float const range_y{max[1] - min[1]};
    auto const num_bins_x{static_cast<std::uint32_t>(std::ceil(range_x / eps_))};
    auto const num_bins_y{static_cast<std::uint32_t>(std::ceil(range_y / eps_))};

    // count number of points in every bin
    counts_.assign(num_bins_x * num_bins_y, 0);

    // FIRST PASS OVER THE POINTS
    for (auto const& pt : points) {
        auto const bin_x{static_cast<std::uint32_t>(std::floor((pt[0] - min[0]) / eps_))};
        auto const bin_y{static_cast<std::uint32_t>(std::floor((pt[1] - min[1]) / eps_))};
        auto const index{bin_y * num_bins_x + bin_x};
        counts_[index] += 1;
    }

    // calculate the offsets for each cell (bin)
    offsets_.clear();
    std::exclusive_scan(std::cbegin(counts_), std::cend(counts_), std::back_inserter(offsets_), 0);

    // re-sorting the points (calculating index mapping) based on the bin indices
    auto scratch = offsets_;
    std::vector<Point> new_points(std::size(points));
    std::vector<std::uint32_t> new_point_to_point_index_map(std::size(points));
    std::uint32_t i{0};
    for (auto const& pt : points) {
        auto const bin_x{static_cast<std::uint32_t>(std::floor((pt[0] - min[0]) / eps_))};
        auto const bin_y{static_cast<std::uint32_t>(std::floor((pt[1] - min[1]) / eps_))};
        auto const index{bin_y * num_bins_x + bin_x};
        auto const new_pt_index{scratch[index]};
        scratch[index] += 1;
        new_points[new_pt_index] = pt;
        new_point_to_point_index_map[new_pt_index] = i++;
    }

    std::vector<std::uint32_t> num_neighbors(std::size(new_points), 0);

    constexpr auto num_core_points_entries{3U};
    std::vector<std::array<std::int32_t, num_core_points_entries>> core_points_ids;
    core_points_ids.assign(new_points.size(), {-1, -1, -1});

#pragma omp parallel for
    for (auto i = 0UL; i < std::size(new_points); ++i) {
        auto const pt{new_points[i]};
        auto const bin_x{static_cast<std::int32_t>(std::floor((pt[0] - min[0]) / eps_))};
        auto const bin_y{static_cast<std::int32_t>(std::floor((pt[1] - min[1]) / eps_))};

        std::vector<std::uint32_t> local_neighbors;

        constexpr std::array<int, 9> dx = {-1, +0, +1, -1, +0, +1, -1, +0, +1};
        constexpr std::array<int, 9> dy = {-1, -1, -1, +0, +0, +0, +1, +1, +1};

        for (auto ni{0}; ni < 9; ++ni) {
            auto const nx{bin_x + dx[ni]};
            auto const ny{bin_y + dy[ni]};
            if (nx < 0 || ny < 0 || nx >= static_cast<std::int32_t>(num_bins_x) ||
                ny >= static_cast<std::int32_t>(num_bins_y)) {
                continue;
            }
            auto const neighbor_bin{ny * num_bins_x + nx};

            for (auto j{0U}; j < counts_[neighbor_bin]; ++j) {
                auto const neighbor_pt_index{offsets_[neighbor_bin] + j};
                if (neighbor_pt_index == i) {
                    continue;
                }
                auto const& neighbor_pt{new_points[neighbor_pt_index]};
                if ((square(neighbor_pt[0] - pt[0]) + square(neighbor_pt[1] - pt[1])) < eps_squared_) {
                    local_neighbors.push_back(neighbor_pt_index);
                }
            }
        }

        if (std::size(local_neighbors) > min_samples_) {
            for (auto const n : local_neighbors) {
                auto& cps{core_points_ids[n]};
                for (auto cp_id{0U}; cp_id < num_core_points_entries; ++cp_id) {
                    if (cps[cp_id] == -1) {
                        cps[cp_id] = i;
                        break;
                    }
                }
            }
        }
    }

    for (auto i{0UL}; i < std::size(new_points); ++i) {
        if (core_points_ids[i][0] >= 0) {
            labels_[i] = static_cast<Label>(i);
        } else {
            labels_[i] = noise;
        }
    }

    bool converged{false};
    while (!converged) {
        converged = true;
        for (auto i{0UL}; i < std::size(new_points); ++i) {
            if (labels_[i] == -1) {
                continue;
            }
            for (auto const current_core_idx : core_points_ids[i]) {
                if (current_core_idx == -1) {
                    continue;
                }
                if (labels_[i] < labels_[current_core_idx]) {
                    labels_[current_core_idx] = labels_[i];
                    converged = false;
                } else if (labels_[i] > labels_[current_core_idx]) {
                    labels_[i] = labels_[current_core_idx];
                    converged = false;
                }
            }
        }
    }

    std::unordered_map<Label, Label> labels_map;
    labels_map.reserve(labels_.size());

    Label num_labels{0};
    labels_map[noise] = noise;
    for (auto const l : labels_) {
        if (labels_map.find(l) == labels_map.end()) {
            labels_map[l] = num_labels;
            num_labels++;
        }
    }

    std::vector<Label> labels(std::size(labels_));
    for (auto i{0U}; i < std::size(labels_); ++i) {
        labels[new_point_to_point_index_map[i]] = labels_map[labels_[i]];
    }

    return labels;
}

}  // namespace dbscan
