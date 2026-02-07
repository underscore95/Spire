import numpy as np

size = 64 * 64 * 64

data = np.random.rand(size).astype(np.float32)

with open("LODSamplingOffsets.bin", "wb") as f:
    data.tofile(f)
