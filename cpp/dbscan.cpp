#include "cpp/dbscan.hpp"

#include <nanoflann.hpp>

namespace dbscan {

namespace {

class PointsVectorAdaptor
{
public:
    explicit PointsVectorAdaptor(std::vector<Dbscan::Point> const& points)
        : points_{points}
    {
    }

    std::size_t kdtree_get_point_count() const
    {
        return std::size(points_);
    }

    float kdtree_get_pt(std::size_t idx, int dim) const
    {
        return points_[idx][dim];
    }

    template <typename Bbox>
    bool kdtree_get_bbox(Bbox&) const
    {
        return false;
    }

private:
    std::vector<Dbscan::Point> const& points_;
};

}  // namespace

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
    PointsVectorAdaptor adapter{points};

    constexpr auto num_dims{2};
    constexpr auto leaf_size{32};
    nanoflann::
        KDTreeSingleIndexAdaptor<nanoflann::L2_Adaptor<float, PointsVectorAdaptor>, PointsVectorAdaptor, num_dims>
            points_kd_tree{num_dims, adapter, nanoflann::KDTreeSingleIndexAdaptorParams{leaf_size}};
    points_kd_tree.buildIndex();

    nanoflann::SearchParams params{};
    params.sorted = false;

    labels_.assign(std::size(points), undefined);

    Label cluster_count{0};
    std::vector<Label> clusters{};

    for (auto i{0UL}; i < std::size(points); ++i) {
        // skip point if it has already been processed
        if (labels_[i] != undefined) {
            continue;
        }

        // find number of neighbors of current point
        if (points_kd_tree.radiusSearch(points[i].data(), eps_squared_, neighbors_, params) < min_samples_) {
            labels_[i] = noise;
            continue;
        }

        // This point has at least min_samples_ in its eps neighborhood, so it's considered a core point. Time to
        // start a new cluster.

        auto const current_cluster_id{cluster_count++};
        labels_[i] = current_cluster_id;

        to_visit_.clear();
        visited_.assign(std::size(points), false);

        for (auto const& n : neighbors_) {
            if (!visited_[n.first]) {
                to_visit_.push_back(n.first);
            }
            visited_[n.first] = true;
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
            if (points_kd_tree.radiusSearch(points[neighbor].data(), eps_squared_, neighbors_, params) < min_samples_) {
                continue;
            }
            for (auto const& n : neighbors_) {
                if (!visited_[n.first]) {
                    to_visit_.push_back(n.first);
                }
                visited_[n.first] = true;
            }
        }
    }

    return labels_;
}

}  // namespace dbscan
