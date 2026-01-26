# Spire

This is documentation for the Spire project.

# Entry Point

Game is an executable, in the main function you can create an instance of Spire::Engine and pass in a unique pointer to a class extending Spire::Application

Spire::Engine contains references to all the subsystems and utilities which Spire provides.

# Applications

Application contains functions which run every tick, every frame, handle window resizing, and handle closing the window.

If any of your initialization logic requires Spire::Engine, do it in Start which is called right after your constructor.

# Camera

Spire contains a Camera class which can be used to create a camera which can be moved around using user input.
It generates a view projection matrix which can be passed into shaders.

# Threading

Spire contains a thread pool singleton which can be accessed via `Spire::ThreadPool::Instance()`.

This uses a threading library: https://github.com/bshoshany/thread-pool

# Delegates

Sometimes you need to broadcast an event to other code, you can do this using delegates in Spire.

You can create a delegate by initializing a `Spire::Delegate<>`, you can also pass in arguments such as `Spire::Delegate<int, std::string>`.

You can subscribe to a delegate using `delegate.GetSubscribers().AddCallback(lambda);`
This function returns an id which can be used to unsubscribe later.


You can also use the `LocalDelegateSubscriber<>` class which subscribes to a delegate in its constructor and unsubscribes in its destructor.

It also contains logic for ensuring the delegate is still valid - so it won't attempt to unsubscribe from a delegate that has already been destroyed.


You can call all subscribers to a delegate using `delegate.Broadcast();`

Note you will need to pass in any arguments your delegate requires.

# Timing

You can use Spire::Timer for timing, internally it uses std::chrono::high_resolution_clock

It will automatically start ticking when constructed and you can use `SecondsSinceStart` or `MillisSinceStart` to get the time.

You can call `Restart` to restart the timer.

## Delta Time

You can get the time since last frame in seconds using Spire::Engine.GetDeltaTime()

# RNG

Spire contains a Random utility, see engine.GetRandom().

It can generate random ints, floats, bools in a cleaner and more concise way than the standard library provides.

# Window

Spire::Engine contains a window, you can get a reference using engine.GetWindow().

Spire::Window is an abstraction over GLFW and contains functions for handling user input, handling window resize, setting window properties (e.g. title)

# Logging

The functions `info` `warn` and `error` are available in the Spire namespace for logging. 
These functions log to the standard output.

These functions support formatting just like std::format does, for example:
```
Spire::info("Number Chunks Loaded: {}", numChunksLoaded);
```

# Rendering

RenderingManager contains references for all rendering related subsystems and utilities, you can get a reference using engine.GetRenderingManager()

RenderingManager automatically initializes most things you need to start rendering in Vulkan, including the swapchain, device, queue, and more.

You only need to intialize shaders, descriptors, pipeline, and command buffers.
Spire contains utilities for all of these.

In general Spire abstracts Vulkan by creating RAII objects which wrap raw Vulkan objects, and sometimes provide additional utilities and functionality.

## How Does Rendering Work?

You need to be familiar with Vulkan to use Spire.

In Spire, and Vulkan, multiple frames are rendered at the same time, in your Render function you can get the index of the image being rendered into for the current frame using `rm.GetQueue().AcquireNextImage();`

The range of this index will vary depending on your project and device, for example if you use double buffering it will be 0 or 1, if you use triple buffering (the default) it will be 0, 1, or 2.
It may also return INVALID_IMAGE_INDEX.

You can submit your render commands using `rm.GetQueue().SubmitRenderCommands(commandBuffersToSubmit.size(), commandBuffersToSubmit.data());`

If you want, you can use the below functions to get command buffers which will handle dynamic rendering for you, so you don't need to worry about that or render passes.
```
rm.GetRenderer().GetBeginRenderingCommandBuffer(m_swapchainImageIndex)
rm.GetRenderer().GetEndRenderingCommandBuffer(m_swapchainImageIndex)
```

## ImGui

Spire contains ImGui setup code, simply submit the command buffer that the ImGuiRenderer provides after all your render commands and before you end rendering, e.g:
```
std::array commandBuffersToSubmit = {
    rm.GetRenderer().GetBeginRenderingCommandBuffer(m_swapchainImageIndex),
    commandBuffer,
    rm.GetImGuiRenderer().PrepareCommandBuffer(m_swapchainImageIndex),
    rm.GetRenderer().GetEndRenderingCommandBuffer(m_swapchainImageIndex)
};
rm.GetQueue().SubmitRenderCommands(commandBuffersToSubmit.size(), commandBuffersToSubmit.data());
```
You can do all your UI rendering in the RenderUi function, e.g:
```
ImGui_ImplVulkan_NewFrame();
ImGui_ImplGlfw_NewFrame();
ImGui::NewFrame();

ImGui::Begin(GetApplicationName(), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
ImGui::Text("Application average %.3f ms/frame (%.1f FPS) (frame %d (swapchain image %d))", m_engine->GetDeltaTime() * 1000, 1.0f / m_engine->GetDeltaTime(), m_frame,
                m_swapchainImageIndex);

ImGui::End();
ImGui::Render();
```

## Debugging

In debug mode, Vulkan validation layers are enabled and printed to console.

Asserts are also used throughout the project to ensure valid state.

## FrameDeleter

Sometimes you need to delete a resource once it is no longer being used by the GPU, Spire provides a `FrameDeleter` class for this.

You can create a `FrameDeleter<T>` instance and then call `Push(std::move(t), numFrames);` to delete the object once a certain number of rames has passed.

For example, if you are triple buffering you can pass in 3 which will delete the object after 3 frames passed (so no render commands are still using it)

Note you need to keep the frame deleter instance around until all objects are deleted, and you also need to call `Update` every tick.

## Push Constants

You can use `pipeline.CmdSetPushConstants` to pass push constants into your shader, this is ideal for small (<128 bytes) pieces of data.

## Resources

Internally Spire will track the number of GPU resource allocation and deallocations to ensure you are aware if there is any memory leaks.

In debug mode it will trigger an exception if you try to destroy the `RenderingManager` while allocated GPU resources still exist.

### Buffers

RenderingManager contains a BufferManager which has utilities for creating and destroying buffers.

You can use UpdateBuffer to write into a buffer, alternatively you can use Map which returns MappedMemory to read or write.
Note that the only way to read from a buffer is using mapped memory.

Internally Spire uses vulkan-memory-allocator by AMD so that all buffers are created in the same memory allocation, as per Vulkan best practice.

#### BufferAllocator

It is also best practice to use a single large buffer rather than many small buffers, `BufferManager` does not contain this functionality because you may need different flags or memory properties for your buffers.

You can use `BufferAllocator` to handle this. Internally it uses 1 or more large buffers and you can smaller allocate sections of memory within those large buffers.
The reason it may use more than 1 internal buffer is because in Vulkan, there is a maximum buffer size (4GB on most high end GPUs)

It supports resizing, so if you have 5 buffers of 128MB and try to allocate but run out of memory, if canResize is true it will automatically allocate a 6th 128MB buffer for you.

The constructor looks like this:
```
BufferAllocator(RenderingManager &renderingManager,
                        const std::function<void()> &recreatePipelineCallback,
                        glm::u32 elementSize,
                        glm::u32 numSwapchainImages,
                        std::size_t sizePerInternalBuffer,
                        glm::u32 numInternalBuffers,
                        bool canResize);
```

This allows you to control the number of internal buffers it uses and their size.

You only to pass in recreatePipelineCallback if you want resizing supported, since descriptors will be changed.


You can allocate memory using `std::optional<Allocation> Allocate(std::size_t requestedSize);`

This will tell you where you can write your data.

It will return a nullopt if there is no space left. It will never return a nullopt if resizing is enabled.


You can write to your buffer using `void Write(const Allocation &allocation, const void *data, std::size_t size);`

Alternatively you can use `MapMemory()` which will map all the internal buffers and provide read write access.

You can deallocate using `void ScheduleFreeAllocation(Allocation allocation);` this will deallocate after numSwapchainImages frames to ensure no render commands are still using the allocation.

#### PerImageBuffer

Sometimes you need one buffer per swapchain image to ensure correct synchronization, BufferManager provdes functions to create a `PerImageBuffer` which handles this.

### Images

RenderingManager contains an `ImageManager` which can be used to create and destroy images.

Creating an image automatically creates an associated view and sampler, Spire does not support a single sampler for multiple images.

#### Loading Images

You can use `Spire::ImageLoader::LoadImageFromFile` to load a PNG into CPU memory which can then be used to create a GPU image.

This is an abstraction of STB.

#### Swapchain Images

The swapchain images are automatically allocated for you and exist inside Spire::VulkanQueue

### Models

You can use `Spire::ModelLoader::LoadModel` to load a model into CPU memory which you can then upload to the GPU.

Models are very basic at the moment (1 material per mesh, no material properties other than diffuse texture) and do not work with SpireVoxel.

It will also output a list of paths to images that the model requires.

### Descriptors

You can create a `Spire::DescriptorManager` to manage descriptors for a pipeline.

This is Spire's abstraction over Vulkan descriptors, it is fairly high level however you do have to tell Spire all your descriptors.

You do this by passing a `Spire::DescriptorSetLayoutList` into the descriptor manager's constructor.
This cannot be modified once you have given it to the descriptor manager, you can use `descriptorManager.WriteDescriptor` if you need to update something later.
If you need more flexibility you will need to recreate the pipeline and descriptors.

Descriptors are split into per image descriptors and regular descriptors in Spire, all the below functions exist for both however this documentation will only cover regular descriptors.
PerImageDescriptor is intended to be used with PerImageBuffer.

You can push a `Spire::DescriptorSetLayout` (typedef of a vector of `Spire::Descriptor`) to `Spire::DescriptorSetLayoutList`
The set binding will be the index in the list, so the first set you push will have binding 0, second binding 1, etc.

You can create descriptors simply by initializing a `Spire::Descriptor` struct, descriptors do not exist on the GPU until passed to the DescriptorManager. For example:
```
Spire::Descriptor{
    .ResourceType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
    .Binding = binding, // Binding within the set
    .Stages = VK_SHADER_STAGE_FRAGMENT_BIT,
    .Resources = {{.Buffer = &m_voxelTypesBuffer}},
#ifndef NDEBUG
    .DebugName = "Voxel Types Buffer" // See "Debugging" subsection below
#endif
};
```

One thing to note is the `Resources` field, this should be a vector of pointers to resources, so if you want to create a descriptor which represents resources Buffer1 and Buffer2, you should pass in
```
{{.Buffer=&Buffer1}, {.Buffer=&Buffer2}}
```

Spire only supports creating descriptors for VulkanBuffer (SSBO and UBO) and VulkanImage (sampler, image, image view) at the moment.

You can use a `Spire::DescriptorCreator` to create `Spire::PerImageDescriptor`, it requires less code repetition since you would basically be creating the same descriptor multiple times.
See comments of `Spire::DescriptorCreator` for more information.

#### Debugging

Each descriptor has a debug name string associated with it in debug mode only.

Whenever you use DescriptorManager to allocate a descriptor, the GPU object's debug name is set so programs like RenderDoc can name resources correctly.

#### Example

```
    void VoxelRenderer::SetupDescriptors() {
        if (m_descriptorManager) {
            m_oldDescriptorManagers.Push(std::move(m_descriptorManager), m_engine.GetRenderingManager().GetSwapchain().GetNumImages());
        }

        DescriptorSetLayoutList layouts(m_engine.GetRenderingManager().GetSwapchain().GetNumImages());

        DescriptorSetLayout constantSet;
        PerImageDescriptorSetLayout perFrameSet;
        DescriptorSetLayout chunkSet;

        // Images
        constantSet.push_back(m_voxelImageManager->GetDescriptor());

        // Camera
        perFrameSet.push_back(m_camera.GetDescriptor(SPIRE_SHADER_BINDINGS_CAMERA_UBO_BINDING));

        // Chunks
        m_world->GetRenderer().PushDescriptors(perFrameSet, chunkSet);

        // Voxel types
        constantSet.push_back(m_voxelTypeRegistry->GetVoxelTypesBufferDescriptor(SPIRE_VOXEL_SHADER_BINDINGS_VOXEL_TYPE_UBO_BINDING));

        assert(layouts.Size() == SPIRE_SHADER_BINDINGS_CONSTANT_SET);
        layouts.Push(constantSet);
        assert(layouts.Size() == SPIRE_SHADER_BINDINGS_PER_FRAME_SET);
        layouts.Push(perFrameSet);
        assert(layouts.Size() == SPIRE_VOXEL_SHADER_BINDINGS_CONSTANT_CHUNK_SET);
        layouts.Push(chunkSet);

        // Create descriptor manager
        m_descriptorManager = std::make_unique<DescriptorManager>(m_engine.GetRenderingManager(), layouts);
    }
```

### Shaders

You can create a `Spire::ShaderCompiler` to compile your shaders. 

You only need to pass in a path to your file and it spits out a VkShaderModule.

This is an abstraction over glslang and can only compile glsl shaders.

It supports multithreaded compilation, so you can compile multiple shaders in parallel.

Shaders are compiled to SPRV and stored in filename.extension.spv and the SPRV is compiled if there has been no source changes since the last SPRV compilation, this greatly increases compilation time.
You should distribute SPRV files with your game rather than raw GLSL, it will speed up loading time while keeping the benefits of optimising specifically for the GPU the user has.
You should gitignore SPRV files.

Shader files must have a specific extension: `.vert`, `.frag`, `.geom`, `.comp`, `.tesc` (tessellation control), `.tese` (tessellation evaluation)
Otherwise Spire is not capable of figuring out the correct shader stage for you.

Spire shaders support `#include` so you can include other shaders, the same limitations on file extension do not apply for included files.
It is possible to write code which is both loaded into a shader and compiled C++ side, providing you use the preprocessor to ensure your code is valid GLSL and C++ at the same time.

`#define NDEBUG` is appended to the top of your file if you are compiling in release.
This can be used for debugging purposes without reducing release mode performance.
Take care you don't accidentally distribute a debug SPRV in a release build, as this may cause reduced performance.

Main CMake file defines `INCLUDE_SHADER_DEBUG_SYMBOLS` in ALL builds, make sure to disable this if you don't want debug symbols.

### Assets Directory

CMake function setup_assets_directory can be used to handle the assets directory, see Assets,cmake for more information.

SpireVoxel asset directory can be accessed via `SpireVoxel::GetAssetsDirectory()`

`ASSETS_DIRECTORY` can be used to get your assets directory

### Command Buffers

`Spire::RenderingCommandManager` can be used to handle command buffer creation/deletion and recording.

## Queue / Swapchain

VulkanQueue wraps VkQueue and VkSwapchain and handles submitting command buffers to the queue and swapping the active swapchain image.

## Pipelines

Spire has an abstract Pipeline class which wraps VkPipeline, with GraphicsPipeline and ComputePipeline classes that implement additional functionality.

## Window Resizing

The pipeline needs to be recreated whenever the window size changes, Spire provides an onWindowResize function in Application to facilitate this.