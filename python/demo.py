"""Demonstrate basic usage."""

import time

import numpy as np
from sklearn import cluster, datasets

from cpp import dbscan_cpp

n_samples = 100000
noisy_moons = datasets.make_moons(n_samples=n_samples, noise=0.02)
X, y = noisy_moons


for name, algorithm in [
    ("sklearn", cluster.DBSCAN(eps=0.03, min_samples=10)),
    ("cpp", dbscan_cpp.DBSCAN(0.03, 10)),
]:
    begin = time.time()
    y_pred = algorithm.fit_predict(X)
    end = time.time()
    print(
        f"{name}: #noise={np.count_nonzero(y_pred == -1)} #clusters={np.max(y_pred) + 1} (took {(end - begin) * 1000} ms)"
    )
