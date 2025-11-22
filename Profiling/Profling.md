
Sample Output:
```
{
    "time_ms": 2036.5746, // Milliseconds to render all frames
    "frames": 10000, // Number of frames to render
    "chunks": 1, // Number of chunks loaded
    "world": "Test3", // World name
    "dynamic_state": "static", // See below
    "chunk_gpu_memory": 19174656, // GPU memory usage to store voxel data in bytes
    "chunk_cpu_memory": 1048616, // CPU memory to store voxel data in bytes
    "window_width": 1280,
    "window_height": 720
}
```

Dynamic State:
If "static", no voxel modifications.
If "dynamic", a voxel is being changed in every chunk every frame which forces the mesh to be regenerated for every chunk.

Test Worlds:
5x5x5 chunks each with 16 voxels in random locations (Test1)

1 chunk full of voxels (Test2)
1 chunk full of voxels + another loaded chunk with a single voxel (Test2B)

1 chunk (50% chance for a voxel to be empty) (Test3)

1 chunk (10% chance for a voxel to be empty) (Test4)

100 chunks (50% chance for a voxel to be empty) (Test5)