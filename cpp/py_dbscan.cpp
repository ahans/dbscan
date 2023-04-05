#include "cpp/dbscan.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

#include <stdexcept>

PYBIND11_MODULE(py_dbscan, m)
{
    py::class_<dbscan::Dbscan>(m, "DBSCAN")
        .def(py::init<float, std::uint32_t>())
        .def("fit_predict",
             [](dbscan::Dbscan* self, py::array_t<float, py::array::c_style | py::array::forcecast> const& array) {
                 auto const buffer_info{array.request()};
                 if (buffer_info.ndim != 2 || buffer_info.shape.at(1) != 2) {
                     throw std::runtime_error{
                         "only C-style float arrays of shape (N, 2) supported (N = number of points)"};
                 }
                 // TODO(ahans) get rid of the copies here
                 std::vector<dbscan::Dbscan::Point> points(buffer_info.shape[0]);
                 static_assert(sizeof(dbscan::Dbscan::Point) == 2 * sizeof(float));
                 std::memcpy(points.data(), buffer_info.ptr, buffer_info.shape[0] * sizeof(dbscan::Dbscan::Point));
                 auto labels{self->fit_predict(points)};
                 auto np_array{py::array_t<int>({buffer_info.shape[0]}, {sizeof(int)}, labels.data())};
                 return np_array;
             });
}
