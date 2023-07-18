"""Basic benchmark against sklearn."""

import timeit
from dataclasses import dataclass

import numpy as np
from sklearn import cluster, datasets

from cpp import py_dbscan


@dataclass
class Test:
    """Collect information about a benchmark test case."""

    name: str
    X: np.ndarray
    eps: float
    min_samples: int


def get_test_cases():
    """Generate benchmark test cases."""
    random_state = 42
    rng = np.random.default_rng(seed=random_state)
    return [
        Test(
            "noisy moons",
            X=datasets.make_moons(
                n_samples=20_000, noise=0.02, random_state=random_state
            )[0],
            eps=0.03,
            min_samples=10,
        ),
        Test(
            "blobs",
            X=datasets.make_blobs(
                n_samples=20_000,
                centers=100.0 * rng.random((100, 2)),
                cluster_std=0.1,
                random_state=42,
            )[0],
            eps=0.1,
            min_samples=10,
        ),
    ]


def benchmark():
    """Run benchmark."""
    for test in get_test_cases():
        print(f"Test: {test.name}")
        for name, algorithm in [
            ("sklearn", cluster.DBSCAN(eps=test.eps, min_samples=test.min_samples)),
            ("cpp", py_dbscan.DBSCAN(test.eps, test.min_samples)),
        ]:
            runtime = timeit.timeit(
                "y_pred = algorithm.fit_predict(test.X)", number=200, globals=locals()
            )
            print(f"    {name:8}: {(runtime * 1000):>8.2f} ms")


if __name__ == "__main__":
    benchmark()
