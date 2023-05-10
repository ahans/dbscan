#include "cpp/dbscan.hpp"

#include <cmath>
#include <iostream>
#include <numeric>

namespace dbscan {

Dbscan::Dbscan(float const eps, std::uint32_t const min_samples, std::size_t const num_points_hint)
    : eps_squared_{eps * eps}
    , min_samples_{min_samples}
{
    if (num_points_hint > 0) {
        labels_.reserve(num_points_hint);
        neighbors_.reserve(num_points_hint);
        visited_.reserve(num_points_hint);
        to_visit_.reserve(num_points_hint);
    }
}

auto Dbscan::fit_predict(std::vector<Dbscan::Point> const& points) -> std::vector<Dbscan::Label>
{
    labels_.assign(std::size(points), undefined);

    if (std::size(points) <= 1) {
        return labels_;
    }

    Label cluster_count{0};
    std::vector<Label> clusters{};

    // reorg point cloud

    auto const eps = std::sqrt(eps_squared_);

    Dbscan::Point min{points[0]};
    Dbscan::Point max{points[0]};
    for (auto const& pt : points) {
        min[0] = std::min(min[0], pt[0]);
        min[1] = std::min(min[1], pt[1]);
        max[0] = std::max(max[0], pt[0]);
        max[1] = std::max(max[1], pt[1]);
    }

    float const range_x = max[0] - min[0];
    float const range_y = max[1] - min[1];
    auto const num_bins_x = static_cast<std::uint32_t>(std::ceil(range_x / eps));
    auto const num_bins_y = static_cast<std::uint32_t>(std::ceil(range_y / eps));

    std::vector<std::uint32_t> counts(num_bins_x * num_bins_y);

    for (auto const& pt : points) {
        auto const bin_x = static_cast<std::uint32_t>(std::floor((pt[0] - min[0]) / eps));
        auto const bin_y = static_cast<std::uint32_t>(std::floor((pt[1] - min[1]) / eps));
        auto const index = bin_y * num_bins_x + bin_x;
        counts[index] += 1;
    }
    std::vector<std::uint32_t> offsets{};
    offsets.reserve(std::size(counts));
    std::exclusive_scan(std::cbegin(counts), std::cend(counts), std::back_inserter(offsets), 0);

    auto scratch = offsets;
    std::vector<Point> new_points(std::size(points));
    std::vector<std::uint32_t> new_point_to_point_index_map(std::size(points));
    std::uint32_t i{0};
    for (auto const& pt : points) {
        auto const bin_x = static_cast<std::uint32_t>(std::floor((pt[0] - min[0]) / eps));
        auto const bin_y = static_cast<std::uint32_t>(std::floor((pt[1] - min[1]) / eps));
        auto const index = bin_y * num_bins_x + bin_x;
        auto new_pt_index = scratch[index];
        scratch[index] += 1;
        new_points[new_pt_index] = pt;
        new_point_to_point_index_map[new_pt_index] = i++;
    }

    std::vector<std::uint32_t> neighbors;
    neighbors.reserve(16364);

    auto square = [](float const v) -> float {
        return v * v;
    };

    auto radius_search = [&](std::uint32_t pt_index) -> std::uint32_t {
        neighbors.clear();
        auto const& pt = new_points[pt_index];
        auto const bin_x = static_cast<std::uint32_t>(std::floor((pt[0] - min[0]) / eps));
        auto const bin_y = static_cast<std::uint32_t>(std::floor((pt[1] - min[1]) / eps));
        for (auto neighbor_bin_y = bin_y - 1; neighbor_bin_y <= bin_y + 1; ++neighbor_bin_y) {
            for (auto neighbor_bin_x = bin_x - 1; neighbor_bin_x <= bin_x + 1; ++neighbor_bin_x) {
                if (neighbor_bin_x < 0 || neighbor_bin_x >= num_bins_x || neighbor_bin_y < 0 ||
                    neighbor_bin_y >= num_bins_y) {
                    continue;
                }
                auto const neighbor_bin = neighbor_bin_y * num_bins_x + neighbor_bin_x;
                for (auto i{0U}; i < counts[neighbor_bin]; ++i) {
                    auto const neighbor_pt_index = offsets[neighbor_bin] + i;
                    if (neighbor_pt_index == pt_index /*|| visited_[neighbor_pt_index]*/) {
                        continue;
                    }
                    auto const neighbor_pt = new_points[neighbor_pt_index];
                    if ((square(neighbor_pt[0] - pt[0]) + square(neighbor_pt[1] - pt[1])) < eps_squared_) {
                        neighbors.push_back(neighbor_pt_index);
                    }
                }
            }
        }
        return neighbors.size();
    };

    for (auto i{0UL}; i < std::size(new_points); ++i) {
        // skip point if it has already been processed
        if (labels_[i] != undefined) {
            continue;
        }

        // find number of neighbors of current point
        if (radius_search(i) < min_samples_) {
            labels_[i] = noise;
            continue;
        }

        // std::cout << "cont" << std::endl;

        // This point has at least min_samples_ in its eps neighborhood, so it's considered a core point. Time to
        // start a new cluster.

        auto const current_cluster_id{cluster_count++};
        labels_[i] = current_cluster_id;

        to_visit_.clear();
        visited_.assign(std::size(new_points), false);

        for (auto const& n : neighbors) {
            if (!visited_[n]) {
                to_visit_.push_back(n);
            }
            visited_[n] = true;
        }

        for (auto j{0UL}; j < std::size(to_visit_); ++j) {
            auto const neighbor{to_visit_[j]};

            if (labels_[neighbor] == noise) {
                // This was considered as a seed before, but didn't have enough points in its eps neighborhood.
                // Since it's in the current seed's neighborhood, we label it as belonging to this label, but it
                // won't be used as a seed again.
                labels_[neighbor] = current_cluster_id;
                continue;
            }

            if (labels_[neighbor] != undefined) {
                // Point belongs already to a cluster: skip it.
                continue;
            }

            // assign the current cluster's label to the neighbor
            labels_[neighbor] = current_cluster_id;

            // and query its neighborhood to see if it also to be considered as a core point
            if (radius_search(neighbor) < min_samples_) {
                continue;
            }
            for (auto const& n : neighbors) {
                if (!visited_[n]) {
                    to_visit_.push_back(n);
                }
                visited_[n] = true;
            }
        }
    }

    std::vector<Label> labels(std::size(labels_));
    for (auto i{0U}; i < std::size(labels_); ++i) {
        labels[new_point_to_point_index_map[i]] = labels_[i];
    }

    return labels;
}

}  // namespace dbscan
