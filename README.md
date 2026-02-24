# Spire

Spire is a low level Vulkan voxel renderer.

See [Documentation](https://github.com/underscore95/VulkanTutorial/tree/main/Documentation) for documentation, building instructions,  etc.

![Screenshot](Documentation/Images/ao_screenshot.png)
Above is a screenshot showing Spire rendering 32x32 Minecraft chunks (36,031,803 non-empty voxels) at over 2600 FPS on an RTX 4060 Ti with 90 MB VRAM.

Minecraft (1.21.1) renders the same world region at 700 FPS with ~0.5GB VRAM. Obviously an unfair comparison because Minecraft has lighting, more voxels, entities, etc.

The entire world can be remeshed and uploaded to the GPU in 180ms (125ms without ambient occlusion) on an i7-12700F (20 threads).

While Spire supports up to 2^16-1 voxel types, only grass and dirt have been configured which is why the world does not look like Minecraft.

![Screenshot](Documentation/Images/spider_screenshot.png)

Above is a screenshot of a spider model being rendered by Spire. It takes up 8x4x8 chunks where each chunk is 64^3 voxels. This version of Spire is slightly outdated and doesn't haev ambient occlusion and some optimisations.

spider.obj was taken from https://github.com/assimp/assimp/tree/master/test/models/OBJ and then converted to .xyzrgb using https://github.com/eisenwave/obj2voxel (scaled up 4x) and then converted into .sprc via Tooling/XYZRGBConverter which Spire is capable of rendering.

46 MB RAM Used

59 MB VRAM Used

0.148ms/frame (6700 FPS)
