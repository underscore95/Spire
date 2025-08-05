#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "Core/Engine.h"
#include "Core/Application.h"
#include "Window/Window.h"

#include "Engine/Rendering/RenderingManager.h"
#include "Engine/Rendering/RenderingCommandManager.h"
#include "Engine/Rendering/VulkanQueue.h"
#include "Engine/Rendering/ShaderCompiler.h"
#include "Engine/Rendering/GraphicsPipeline.h"
#include "Engine/Rendering/BufferManager.h"
#include "Engine/Rendering/PipelineDescriptorSetsManager.h"
#include "Engine/Rendering/TextureManager.h"
#include "Engine/Rendering/Swapchain.h"
#include "Engine/Rendering/LogicalDevice.h"
#include "Engine/Rendering/RenderingSync.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/VulkanBuffer.h"

#include "Engine/Utils/Camera/Camera.h"
#include "Engine/Utils/Timer.h"