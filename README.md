# Fast 2D DBSCAN 

Fast C++ implementation of DBSCAN for 2D data that follows the `sklearn` interface.

It's not exactly fast yet. TODO:

- [ ] Add tests
- [ ] Add benchmark (maybe together with tests using Catch2)
- [ ] Use 2D grid-based hashing instead of kd-tree for neighborhood lookup
- [ ] Use SIMD vectorization
- [ ] port to GPU
