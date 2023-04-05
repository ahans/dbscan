"""Basic benchmark against sklearn."""

import timeit

from sklearn import cluster, datasets

from cpp import py_dbscan

n_samples = 20000
noisy_moons = datasets.make_moons(n_samples=n_samples, noise=0.02)
X, y = noisy_moons


for name, algorithm in [
    ("sklearn", cluster.DBSCAN(eps=0.03, min_samples=10)),
    ("cpp", py_dbscan.DBSCAN(0.03, 10)),
]:
    runtime = timeit.timeit(
        "y_pred = algorithm.fit_predict(X)", number=10, globals=globals()
    )
    print(f"{name:8}: {(runtime * 1000):>8.2f} ms")
