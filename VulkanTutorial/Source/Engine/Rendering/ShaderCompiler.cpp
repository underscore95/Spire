#include "ShaderCompiler.h"
#include <fstream>
#include <unordered_set>
#include <glm/glm.hpp>
#include <vector>
#include <spdlog/spdlog.h>
#include <glslang/Include/glslang_c_interface.h>
#include <libassert/assert.hpp>
#include "Engine/Utils/FileIO.h"

// https://github.com/KhronosGroup/glslang
// https://github.com/emeiri/ogldev/blob/VULKAN_13/Vulkan/VulkanCore/Source/shader.cpp

struct CompiledShader
{
    std::vector<glm::u32> SPIRV;
    VkShaderModule ShaderModule = nullptr;

    void Initialize(glslang_program_t* program)
    {
        size_t program_size = glslang_program_SPIRV_get_size(program);
        SPIRV.resize(program_size);
        glslang_program_SPIRV_get(program, SPIRV.data());
    }
};

static void PrintShaderSource(const char* text)
{
    int line = 1;

    printf("\n(%3i) ", line);

    while (text && *text++)
    {
        if (*text == '\n')
        {
            printf("\n(%3i) ", ++line);
        }
        else if (*text == '\r')
        {
            // nothing to do
        }
        else
        {
            printf("%c", *text);
        }
    }

    printf("\n");
}


static bool CompileShader(const char* shaderName, const VkDevice& device, glslang_stage_t stage,
                          const char* pShaderCode,
                          CompiledShader& ShaderModule)
{
#pragma region defaultResource
    // https://chromium.googlesource.com/external/github.com/KhronosGroup/glslang/%2B/HEAD/glslang/ResourceLimits/ResourceLimits.cpp
    constexpr glslang_resource_t defaultResource = {
        32, // max_lights
        6, // max_clip_planes
        32, // max_texture_units
        32, // max_texture_coords
        64, // max_vertex_attribs
        4096, // max_vertex_uniform_components
        64, // max_varying_floats
        32, // max_vertex_texture_image_units
        80, // max_combined_texture_image_units
        32, // max_texture_image_units
        4096, // max_fragment_uniform_components
        8, // max_draw_buffers
        256, // max_vertex_uniform_vectors
        8, // max_varying_vectors
        16, // max_fragment_uniform_vectors
        16, // max_vertex_output_vectors
        15, // max_fragment_input_vectors
        -8, // min_program_texel_offset
        7, // max_program_texel_offset
        8, // max_clip_distances
        65535, 65535, 65535, // max_compute_work_group_count_{x,y,z}
        1024, 1024, 64, // max_compute_work_group_size_{x,y,z}
        1024, // max_compute_uniform_components
        16, // max_compute_texture_image_units
        8, // max_compute_image_uniforms
        8, // max_compute_atomic_counters
        1, // max_compute_atomic_counter_buffers
        60, // max_varying_components
        64, // max_vertex_output_components
        64, // max_geometry_input_components
        128, // max_geometry_output_components
        128, // max_fragment_input_components
        8, // max_image_units
        8, // max_combined_image_units_and_fragment_outputs
        8, // max_combined_shader_output_resources
        0, // max_image_samples
        8, // max_vertex_image_uniforms
        8, // max_tess_control_image_uniforms
        8, // max_tess_evaluation_image_uniforms
        8, // max_geometry_image_uniforms
        8, // max_fragment_image_uniforms
        8, // max_combined_image_uniforms
        16, // max_geometry_texture_image_units
        256, // max_geometry_output_vertices
        1024, // max_geometry_total_output_components
        1024, // max_geometry_uniform_components
        64, // max_geometry_varying_components
        128, // max_tess_control_input_components
        128, // max_tess_control_output_components
        16, // max_tess_control_texture_image_units
        1024, // max_tess_control_uniform_components
        4096, // max_tess_control_total_output_components
        128, // max_tess_evaluation_input_components
        128, // max_tess_evaluation_output_components
        16, // max_tess_evaluation_texture_image_units
        1024, // max_tess_evaluation_uniform_components
        120, // max_tess_patch_components
        32, // max_patch_vertices
        64, // max_tess_gen_level
        16, // max_viewports
        0, 0, 0, 0, 0, // max_vertex_atomic_counters ... max_fragment_atomic_counters
        1, // max_combined_atomic_counters
        1, // max_atomic_counter_bindings
        0, 0, 0, 0, 0, // max_vertex_atomic_counter_buffers ... max_fragment_atomic_counter_buffers
        1, // max_combined_atomic_counter_buffers
        16384, // max_atomic_counter_buffer_size
        4, // max_transform_feedback_buffers
        64, // max_transform_feedback_interleaved_components
        8, // max_cull_distances
        8, // max_combined_clip_and_cull_distances
        4, // max_samples
        32, 32, 32, // max_mesh_output_vertices_nv, max_mesh_output_primitives_nv, max_mesh_work_group_size_x_nv
        1, 1, // max_mesh_work_group_size_y_nv, z_nv
        32, 1, 1, // max_task_work_group_size_x_nv, y_nv, z_nv
        4, // max_mesh_view_count_nv
        256, 256, 128, // max_mesh_output_vertices_ext, max_mesh_output_primitives_ext, max_mesh_work_group_size_x_ext
        1, 1, // max_mesh_work_group_size_y_ext, z_ext
        128, 1, 1, // max_task_work_group_size_x_ext, y_ext, z_ext
        4, // max_mesh_view_count_ext
        1, // maxDualSourceDrawBuffersEXT

        // limits
        {
            true, // non_inductive_for_loops
            true, // while_loops
            true, // do_while_loops
            true, // general_uniform_indexing
            true, // general_attribute_matrix_vector_indexing
            true, // general_varying_indexing
            true, // general_sampler_indexing
            true, // general_variable_indexing
            true // general_constant_matrix_vector_indexing
        }
    };
#pragma endregion

    glslang_input_t input = {
        .language = GLSLANG_SOURCE_GLSL,
        .stage = stage,
        .client = GLSLANG_CLIENT_VULKAN,
        .client_version = GLSLANG_TARGET_VULKAN_1_1,
        .target_language = GLSLANG_TARGET_SPV,
        .target_language_version = GLSLANG_TARGET_SPV_1_3,
        .code = pShaderCode,
        .default_version = 100,
        .default_profile = GLSLANG_NO_PROFILE,
        .force_default_version_and_profile = false,
        .forward_compatible = false,
        .messages = GLSLANG_MSG_DEFAULT_BIT,
        .resource = &defaultResource
    };

    glslang_shader_t* shader = glslang_shader_create(&input);

    ASSERT(shader);
    if (!glslang_shader_preprocess(shader, &input))
    {
        fprintf(stderr, "GLSL preprocessing failed\n");
        fprintf(stderr, "\n%s", glslang_shader_get_info_log(shader));
        fprintf(stderr, "\n%s", glslang_shader_get_info_debug_log(shader));
        PrintShaderSource(input.code);
        return false;
    }

    if (!glslang_shader_parse(shader, &input))
    {
        fprintf(stderr, "GLSL parsing failed\n");
        fprintf(stderr, "\n%s", glslang_shader_get_info_log(shader));
        fprintf(stderr, "\n%s", glslang_shader_get_info_debug_log(shader));
        PrintShaderSource(glslang_shader_get_preprocessed_code(shader));
        return false;
    }

    glslang_program_t* program = glslang_program_create();
    glslang_program_add_shader(program, shader);

    if (!glslang_program_link(program, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
    {
        fprintf(stderr, "GLSL linking failed\n");
        fprintf(stderr, "\n%s", glslang_program_get_info_log(program));
        fprintf(stderr, "\n%s", glslang_program_get_info_debug_log(program));
        return false;
    }

    glslang_program_SPIRV_generate(program, stage);

    ShaderModule.Initialize(program);

    const char* messagesSPIRV = glslang_program_SPIRV_get_messages(program);

    if (messagesSPIRV)
    {
        fprintf(stderr, "SPIR-V message: '%s'", messagesSPIRV);
    }

    VkShaderModuleCreateInfo shaderCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = ShaderModule.SPIRV.size() * sizeof(uint32_t),
        .pCode = static_cast<const uint32_t*>(ShaderModule.SPIRV.data())
    };

    VkResult res = vkCreateShaderModule(device, &shaderCreateInfo, nullptr, &ShaderModule.ShaderModule);
    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create shader module");
    }

    glslang_program_delete(program);
    glslang_shader_delete(shader);

    bool ret = ShaderModule.SPIRV.size() > 0 && res == VK_SUCCESS;

    return ret;
}

static glslang_stage_t ShaderStageFromFilename(const char* pFilename)
{
    std::string s(pFilename);

    if (s.ends_with(".vert"))
    {
        return GLSLANG_STAGE_VERTEX;
    }

    if (s.ends_with(".frag"))
    {
        return GLSLANG_STAGE_FRAGMENT;
    }

    if (s.ends_with(".geom"))
    {
        return GLSLANG_STAGE_GEOMETRY;
    }

    if (s.ends_with(".comp"))
    {
        return GLSLANG_STAGE_COMPUTE;
    }

    if (s.ends_with(".tesc"))
    {
        return GLSLANG_STAGE_TESSCONTROL;
    }

    if (s.ends_with(".tese"))
    {
        return GLSLANG_STAGE_TESSEVALUATION;
    }

    spdlog::error("Unknown shader stage in '{}'\n", pFilename);

    return GLSLANG_STAGE_VERTEX;
}

struct ParsedShader
{
    std::string FullSource;
    std::unordered_set<std::string> FilePaths; // includes root
    bool Success = false;
};

static ParsedShader ParseShader(const std::string& filePath)
{
    ParsedShader parsed = {};
    parsed.FilePaths.insert(filePath);

    // Get shader source
    if (!FileIO::ReadFile(filePath.c_str(), parsed.FullSource))
    {
        spdlog::error("Failed to read file {} when creating shader from text", filePath);
        return {};
    }

    // Find and replace #includes
    size_t pos = 0;
    while ((pos = parsed.FullSource.find("\n#include ", pos)) != std::string::npos)
    {
        // Get the path
        size_t quoteStart = parsed.FullSource.find('"', pos);
        if (quoteStart == std::string::npos)
        {
            spdlog::error("Invalid #include in shader (missing opening quote)");
            return {};
        }

        size_t quoteEnd = parsed.FullSource.find('"', quoteStart + 1);
        if (quoteEnd == std::string::npos)
        {
            spdlog::error("Invalid #include in shader (missing closing quote)");
            return {};
        }

        std::string includedPath = parsed.FullSource.substr(quoteStart + 1, quoteEnd - quoteStart - 1);

        // Get path relative to file
        std::string actualPath = filePath;
        size_t lastSlash = actualPath.find_last_of("/\\");
        if (lastSlash != std::string::npos)
            actualPath = actualPath.substr(0, lastSlash + 1);
        else
            actualPath.clear();
        actualPath += includedPath;

        // Check for recursive includes
        if (!parsed.FilePaths.insert(actualPath).second)
        {
            spdlog::error("Recursively including {}", actualPath);
            return {};
        }

        // Get the included source
        std::string includedSource;
        if (!FileIO::ReadFile(actualPath.c_str(), includedSource))
        {
            spdlog::error("Failed to read file {} when creating shader from text", actualPath);
            return {};
        }

        // Replace the #include line with the included source
        size_t includeLineEnd = parsed.FullSource.find('\n', quoteEnd);
        if (includeLineEnd == std::string::npos)
            includeLineEnd = parsed.FullSource.size();
        else
            includeLineEnd += 1; // include the newline

        parsed.FullSource.replace(pos, includeLineEnd - pos, includedSource);
    }

    parsed.Success = true;
    return parsed;
}

ShaderCompiler::ShaderCompiler(VkDevice device,
                               bool alwaysCompileFromSource)
    : m_device(device),
      m_alwaysCompileFromSource(alwaysCompileFromSource)
{
    glslang_initialize_process();
}

ShaderCompiler::~ShaderCompiler()
{
    Await();
    glslang_finalize_process();
}

void ShaderCompiler::CreateShaderModuleAsync(VkShaderModule* out, const std::string& fileName)
{
    ++m_currentTasks;
    std::thread([this, out, fileName]()
    {
        VkShaderModule result = CreateShaderModule(fileName);
        if (out) *out = result;

        --m_currentTasks;
        m_waitForTasksCompleteCv.notify_all();
    }).detach();
}

void ShaderCompiler::Await()
{
    std::unique_lock lock(m_waitForTasksMutex);
    m_waitForTasksCompleteCv.wait(lock, [&] { return m_currentTasks == 0; });
}

VkShaderModule ShaderCompiler::CreateShaderModule(const std::string& fileName) const
{
    std::filesystem::path compiledShader = fileName;
    compiledShader += ".spv";

    // Parse
    ParsedShader parsed = ParseShader(fileName);
    if (!parsed.Success)
    {
        spdlog::error("Error when parsing shader {}", fileName);
        return VK_NULL_HANDLE;
    }

    // Compiling from binary or source?
    bool compileFromText = true;
    if (std::filesystem::exists(compiledShader))
    {
        compileFromText = false;

        for (auto& includedFile : parsed.FilePaths)
        {
            auto srcTime = std::filesystem::last_write_time(includedFile);
            auto binTime = std::filesystem::last_write_time(compiledShader);
            compileFromText = srcTime >= binTime;
            if (compileFromText)break;
        }
    }

    VkShaderModule shader = (m_alwaysCompileFromSource || compileFromText)
                                ? CreateShaderModuleFromSource(fileName, parsed.FullSource)
                                : CreateShaderModuleFromBinaryFile(compiledShader.string());
    spdlog::info("{} shader '{}'", shader == VK_NULL_HANDLE ? "Failed to compile" : "Successfully compiled", fileName);
    return shader;
}

VkShaderModule ShaderCompiler::CreateShaderModuleFromBinaryFile(const std::string& fileName) const
{
    int codeSize = 0;
    char* pShaderCode = FileIO::ReadBinaryFile(fileName.c_str(), codeSize);
    assert(pShaderCode);

    VkShaderModuleCreateInfo shaderCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = static_cast<size_t>(codeSize),
        .pCode = reinterpret_cast<const uint32_t*>(pShaderCode)
    };

    VkShaderModule shaderModule;
    VkResult res = vkCreateShaderModule(m_device, &shaderCreateInfo, nullptr, &shaderModule);

    if (res != VK_SUCCESS)
    {
        spdlog::error("Failed to create shader module (from binary) {}", fileName);
    }

    free(pShaderCode);

    return shaderModule;
}

// ReSharper disable once CppDFAConstantFunctionResult
VkShaderModule ShaderCompiler::CreateShaderModuleFromSource(const std::string& fileName,
                                                            const std::string& shaderSource) const
{
    CompiledShader shaderModule;

    glslang_stage_t shaderStage = ShaderStageFromFilename(fileName.c_str());

    VkShaderModule ret = nullptr;

    bool success = CompileShader(shaderSource.c_str(), m_device, shaderStage, shaderSource.c_str(), shaderModule);

    if (success)
    {
        ret = shaderModule.ShaderModule;
        std::string binaryFilename = std::string(fileName.c_str()) + ".spv";
        FileIO::WriteBinaryFile(binaryFilename.c_str(), shaderModule.SPIRV.data(),
                                static_cast<int>(shaderModule.SPIRV.size()) * sizeof(uint32_t));
        //spdlog::info("Compiled shader from {}", shaderSource);
    }
    else
    {
        spdlog::error("Failed to compile {} into a shader", shaderSource);
    }

    return ret;
}
