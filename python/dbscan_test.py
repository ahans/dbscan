"""Tests of DBSCAN C++ modules from Python."""

import numpy as np
import pytest
from sklearn import cluster, datasets

from cpp import py_dbscan


def test_moon():
    """Basic test using two moons sample."""
    X, y = datasets.make_moons(n_samples=1000)
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


pytest.main([__file__, "-rP"])
