"""Basic benchmark against sklearn."""

import timeit

from sklearn import cluster, datasets

from cpp import py_dbscan

n_samples = 100000
noisy_moons = datasets.make_blobs(n_samples=n_samples, centers=[(0,0), (10,10), (2.5, 2), (-3, 0)])
X, y = noisy_moons


for name, algorithm in [
    ("sklearn", cluster.DBSCAN(eps=0.1, min_samples=20, algorithm="ball_tree")),
    ("cpp", py_dbscan.DBSCAN(0.1, 20)),
]:
    runtime = timeit.timeit(
        "y_pred = algorithm.fit_predict(X)", number=1, globals=globals()
    )
    print(f"{name:8}: {(runtime * 1000):>8.2f} ms")
