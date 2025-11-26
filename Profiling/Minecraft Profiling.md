
Minecraft was tested using OpenGL 3.3.0 NVIDIA 560.94 on 1280x720

CPU bottlenecks minecraft

Note a Minecraft chunk is 16x16x384 voxels while a Spire chunk is 64x64x64 voxels.
Note that this isn't a fair comparison as Minecraft has additional overheads that Spire doesn't, for example:
- Transparent voxels
- Networking (even singleplayer launches a local server separate from the client)

Profiling on Minecraft was done during the day (minimal lighting updates) and with entities disabled (to minimise irrelevant workload). 
Time was given for Minecraft to procedurally generate all chunks and create chunk meshes before profiling.

Test6 is a Minecraft world, default settings were used with the following changes:
- Simulation Distance: 5 Minecraft chunks (This is how far to process entity AI, physics, etc, and was set to the minimum value due to this logic not being present in Spire)
- Render Distance: 32 Minecraft chunks (Minecraft worlds are split into regions of 32x32 Minecraft chunks, and each region is stored in its own file. This render distance renders a single region.
- Maximum FPS: Uncapped

Minecraft Profiling:
- ~700 FPS or ~1.43 ms/frame
- ~800 MB CPU memory usage
- ~0.5 GB GPU memory usage
- 20% GPU usage

Spire v1.10.2 Profiling:
- ~1193 FPS or 0.838 ms/frame
- 384 MB CPU memory usage
- 131 MB GPU memory usage
- 97% GPU usage

{"time_ms": 832.8078, "frames": 1000, "chunks": 384, "world": "Test6", "dynamic_state": "stat
ic", "chunk_gpu_memory": 136829376, "chunk_cpu_memory": 402674688, "window_width": 1280, "window_height": 720}