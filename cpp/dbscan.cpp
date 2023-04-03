
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <nanoflann.hpp>

#include <algorithm>
#include <cstdint>
#include <array>
#include <chrono>
#include <iostream>
#include <vector>

namespace {

class ScopedTimer
{
public:
    ScopedTimer()
    {
    }

    auto elapsed_ns() const
    {
        return (std::chrono::high_resolution_clock::now() - begin_).count();
    }

private:
    std::chrono::high_resolution_clock::time_point begin_{std::chrono::high_resolution_clock::now()};
};

namespace py = pybind11;

class DbScan
{
    template <typename T, std::uint32_t num_dims>
    class ArrayVectorAdaptor
    {
    public:
        explicit ArrayVectorAdaptor(py::array_t<T> const& points)
            : points_{points.template unchecked<num_dims>()}
        {
        }

        std::size_t kdtree_get_point_count() const
        {
            return points_.shape(0);
        }

        float kdtree_get_pt(std::size_t idx, int dim) const
        {
            return *points_.data(idx, dim);
        }

        template <typename Bbox>
        bool kdtree_get_bbox(Bbox&) const
        {
            return false;
        }

    private:
        py::detail::unchecked_reference<T, num_dims> points_;
    };

public:
    static constexpr std::int32_t undefined{-2};
    static constexpr std::int32_t noise{-1};

    static constexpr auto num_dims{2};

    DbScan(float eps, std::uint32_t min_samples)
        : eps_{eps * eps}
        , min_samples_{min_samples}
    {
    }

    auto fit_predict(py::array_t<float> const& points_in)
    {
        ScopedTimer timer{};
        using Adapter = ArrayVectorAdaptor<float, num_dims>;
        Adapter adapter{points_in};

        auto const points{points_in.unchecked<num_dims>()};

        nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Adaptor<float, Adapter>, Adapter, num_dims> points_kd_tree{
            num_dims, adapter, nanoflann::KDTreeSingleIndexAdaptorParams{32}};
        points_kd_tree.buildIndex();

        std::cout << "building index: " << (static_cast<double>(timer.elapsed_ns()) / 1000.0) << " µs" << std::endl;

        nanoflann::SearchParams params{};
        params.sorted = false;

        labels_.clear();
        labels_.resize(points.shape(0), undefined);

        to_visit_.clear();
        visited_.clear();

        int32_t cluster_count{0};

        std::vector<int> clusters;

        for (auto i{0UL}; i < static_cast<std::size_t>(points.shape(0)); ++i) {
            // skip point if it has already been processed
            if (labels_[i] != undefined) {
                continue;
            }

            // find number of neighbors of current point
            if (points_kd_tree.radiusSearch(points.data(i, 0), eps_, neighbors_, params) < min_samples_) {
                labels_[i] = noise;
                continue;
            }

            // This point has at least min_samples_ in its eps neighborhood, so it's considered a core point. Time to
            // start a new cluster.

            auto const current_cluster_id{cluster_count++};
            labels_[i] = current_cluster_id;

            to_visit_.clear();
            visited_.clear();
            visited_.resize(points.shape(0), false);

            for (auto const& n : neighbors_) {
                if (!visited_[n.first]) {
                    to_visit_.push_back(n.first);
                }
                visited_[n.first] = true;
            }

            for (auto j{0UL}; j < std::size(to_visit_); ++j) {
                auto const neighbor = to_visit_[j];

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
                if (points_kd_tree.radiusSearch(points.data(neighbor, 0), eps_, neighbors_, params) < min_samples_) {
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

        auto const np_array{py::array_t<int>({labels_.size()}, {sizeof(int)}, labels_.data())};
        return np_array;
    }

private:
    float eps_;
    uint32_t min_samples_;
    std::vector<std::int32_t> labels_;
    std::vector<std::pair<uint32_t, float>> neighbors_;
    std::vector<bool> visited_;
    std::vector<uint32_t> to_visit_;
};

}  // namespace

PYBIND11_MODULE(dbscan_cpp, m)
{
    py::class_<DbScan>(m, "DBSCAN").def(py::init<float, std::uint32_t>()).def("fit_predict", &DbScan::fit_predict);
}
