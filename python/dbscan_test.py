"""Tests of DBSCAN C++ modules from Python."""
import sys

import numpy as np
import pytest
from sklearn import cluster, datasets

from cpp import py_dbscan


def test_moon():
    """Basic test using two moons sample."""
    X, _ = datasets.make_moons(n_samples=1000, random_state=42)
    dbscan = py_dbscan.DBSCAN(0.05, 10)
    y_pred = dbscan.fit_predict(X)

    num_class0 = np.count_nonzero(y_pred == 0)
    num_class1 = np.count_nonzero(y_pred == 1)
    assert num_class0 == pytest.approx(500, abs=5)
    assert num_class1 == pytest.approx(500, abs=5)

    # also compare against sklearn
    sklearn_dbscan = cluster.DBSCAN(eps=0.05, min_samples=10)
    y_pred_sklearn = sklearn_dbscan.fit_predict(X)
    assert y_pred.shape[0] == y_pred_sklearn.shape[0]
    num_matching = np.count_nonzero(y_pred == y_pred_sklearn)
    assert num_matching == pytest.approx(1000, abs=10)


def test_blobs():
    """Test using 20 2-D blobs samples."""
    centers = (
        (16, 0),
        (2, 2),
        (10, 2),
        (15, 3),
        (1, 5),
        (18, 5),
        (7, 6),
        (13, 7),
        (3, 9),
        (15, 10),
        (9, 11),
        (1, 13),
        (5, 14),
        (18, 14),
        (10, 15),
        (15, 15),
        (2, 18),
        (6, 19),
        (17, 19),
        (11, 22),
    )
    n_blobs = len(centers)
    X, _ = datasets.make_blobs(
        n_samples=10_000, centers=centers, cluster_std=0.1, random_state=42
    )

    dbscan = py_dbscan.DBSCAN(0.5, 20)
    y_pred = dbscan.fit_predict(X)
    assert all(np.count_nonzero(y_pred == label) == 500 for label in range(n_blobs))


if __name__ == "__main__":
    sys.exit(pytest.main([__file__, "-rP"]))
