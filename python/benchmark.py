"""Basic benchmark against sklearn."""

import timeit

from sklearn import cluster, datasets

from cpp import py_dbscan

n_samples = 100000
noisy_moons = datasets.make_moons(n_samples=n_samples, noise=0.02)
X, y = noisy_moons


for name, algorithm in [
    ("sklearn", cluster.DBSCAN(eps=0.0025, min_samples=100, algorithm="ball_tree")),
    ("cpp", py_dbscan.DBSCAN(0.0025, 100)),
]:
    runtime = timeit.timeit(
        "y_pred = algorithm.fit_predict(X)", number=10, globals=globals()
    )
    print(f"{name:8}: {(runtime * 1000):>8.2f} ms")
