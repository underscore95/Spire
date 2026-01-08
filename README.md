# Spire

Spire is a low level Vulkan voxel renderer.

See [Documentation](https://github.com/underscore95/VulkanTutorial/tree/main/Documentation) for documentation, building instructions,  etc.

![Screenshot](https://i.ibb.co/ZRHGkdJw/voxelrend.png)
Above is a screenshot showing Spire rendering 32x32 Minecraft chunks (36,031,803 non-empty voxels) at over 1600 FPS on an RTX 4060 Ti with 248 MB VRAM.

The entire world can be remeshed and uploaded to the GPU in 150ms on an i7-12700F.

While Spire supports up to 2^32-1 voxel types, only grass and dirt have been configured which is why the world does not look like Minecraft.