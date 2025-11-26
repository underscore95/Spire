
Minecraft was tested using OpenGL 3.3.0 NVIDIA 560.94

CPU bottlenecks minecraft

Note a Minecraft chunk is 16x16x384 voxels while a Spire chunk is 64x64x64 voxels.
Note that this isn't a fair comparison as Minecraft has additional overheads that Spire doesn't, for example:
- Transparent voxels
- Entity AI & physics
- Networking (even singleplayer launches a local server separate from the client)

Test6 is a Minecraft world, default settings were used with the following changes:
- Simulation Distance: 5 Minecraft chunks (This is how far to process entity AI, physics, etc, and was set to the minimum value due to this logic not being present in Spire)
- Render Distance: 32 Minecraft chunks (Minecraft worlds are split into regions of 32x32 Minecraft chunks, and each region is stored in its own file. This render distance renders a single region.
- Maximum FPS: Uncapped
