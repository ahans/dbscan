"""Tests of DBSCAN C++ modules from Python."""

import numpy as np
import pytest
from sklearn import datasets

from cpp import dbscan_cpp


def test_moon():
    """Basic test using two moons sample."""
    X, y = datasets.make_moons(n_samples=1000)
    dbscan = dbscan_cpp.DBSCAN(0.05, 10)
    y_pred = dbscan.fit_predict(X)

    num_class0 = np.count_nonzero(y_pred == 0)
    num_class1 = np.count_nonzero(y_pred == 1)
    assert num_class0 == pytest.approx(500, 5)
    assert num_class1 == pytest.approx(500, 5)


pytest.main([__file__, "-rP"])
