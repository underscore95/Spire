#include "Profiling.h"

#include "GameCamera.h"
#include "Chunk/VoxelWorld.h"

using namespace Spire;

Profiling::Profiling(
    Engine &engine,
    SpireVoxel::VoxelRenderer &voxelRenderer,
    GameCamera &camera)
    : m_engine(engine),
      m_voxelRenderer(voxelRenderer),
      m_camera(camera) {
    if constexpr (BEGIN_PROFILING_AUTOMATICALLY) {
        m_profilingStartedFrame = 1;
    }
}

void Profiling::Update() {
    if constexpr (!IS_PROFILING) return;
    if (m_profilingStartedFrame == 0) return;
    if (m_profileStrategyIndex >= PROFILE_STRATEGIES.size()) return;
    glm::u64 currentFrame = m_voxelRenderer.GetCurrentFrame();
    if (currentFrame < m_profilingStartedFrame) return;
    if (currentFrame == m_profilingStartedFrame + 1) {
        info("Profiling... (world: {})", PROFILE_WORLD_NAME);
#ifndef NDEBUG
        warn("Profiling in debug mode, results will be inaccurate!");
#endif
        m_timeSinceBeginProfiling.Restart();
    }

    const ProfileStrategy &profileStrategy = PROFILE_STRATEGIES[m_profileStrategyIndex];

    glm::u32 profileEndFrame = profileStrategy.FramesToProfile + m_profilingStartedFrame;
    for (std::size_t i = 0; i < m_profileStrategyIndex; i++) {
        profileEndFrame += PROFILE_STRATEGIES[i].FramesToProfile;
    }

    SpireVoxel::VoxelWorld &world = m_voxelRenderer.GetWorld();

    if (profileStrategy.Dynamic == ProfileStrategy::DYNAMIC) {
        for (auto &[_,chunk] : world) {
            chunk->VoxelData[SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(1, 1, 1)] = chunk->VoxelData[SPIRE_VOXEL_POSITION_XYZ_TO_INDEX(1, 1, 1)] == 0 ? 1 : 0;
            world.GetRenderer().NotifyChunkEdited(*chunk);
        }
    } else {
        assert(profileStrategy.Dynamic == ProfileStrategy::STATIC);
    }

    if (currentFrame == profileEndFrame) {
        m_profileStrategyIndex++;
        SpireVoxel::VoxelWorld::Settings settings = m_voxelRenderer.GetWorld().GetSettings();
        m_profileJson += std::format(
            R"({{"time_ms": {}, "frames": {}, "chunks": {}, "world": "{}", "dynamic_state": "{}", "frustum_culling": "{}", "face_culling": "{}", "chunk_gpu_memory": {}, "chunk_cpu_memory": {}, "window_width": {}, "window_height": {}}}, )",
            m_timeSinceBeginProfiling.MillisSinceStart(),
            profileStrategy.FramesToProfile,
            world.NumLoadedChunks(),
            PROFILE_WORLD_NAME,
            profileStrategy.Dynamic,
            settings.AllowFrustumCulling,
            settings.AllowBackfaceCulling,
            world.CalculateGPUMemoryUsageForChunks(),
            world.CalculateCPUMemoryUsageForChunks(),
            m_engine.GetWindow().GetDimensions().x,
            m_engine.GetWindow().GetDimensions().y
        );
        m_timeSinceBeginProfiling.Restart();

        if (m_profileStrategyIndex >= PROFILE_STRATEGIES.size()) {
            info("Finished profiling! {}", m_profileJson);
        } else {
            info("Using profile strategy index {}...", m_profileStrategyIndex);
        }
    }
}

void Profiling::RenderUI() {
    if constexpr (!BEGIN_PROFILING_AUTOMATICALLY) {
        if (ImGui::CollapsingHeader("Profiling")) {
            if (m_profilingStartedFrame > 0) {
                ImGui::Text("Profiling Started");
            } else {
                if (ImGui::Button("Begin Profiling")) {
                    m_profilingStartedFrame = m_voxelRenderer.GetCurrentFrame();
                }
            }

            if (ImGui::Button("Teleport to profiling location")) {
                m_camera.GetCamera().SetPosition({32, 73, 35});
                m_camera.GetCamera().SetYawPitch(50.0f, 0.0f);
            }
        }
    }
}
