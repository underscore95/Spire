#pragma once

#include "VoxelRenderer.h"

class Profiling {
public:
    explicit Profiling(Spire::Engine &engine, SpireVoxel::VoxelRenderer &voxelRenderer);

public:
    void Update();

    void RenderUI();

public:
    struct ProfileStrategy {
        typedef const char *DynamicState;
        static constexpr DynamicState STATIC = "static"; // no changes to world
        static constexpr DynamicState DYNAMIC = "dynamic"; // change 1 voxel in every chunk every frame

        DynamicState Dynamic;
        glm::u64 FramesToProfile;
    };

    static constexpr const char *PROFILE_WORLD_NAME = "Test6";

    static constexpr ProfileStrategy PROFILE_STATIC = {ProfileStrategy::STATIC, 10000};
    static constexpr ProfileStrategy PROFILE_STATIC_1000 = {ProfileStrategy::STATIC, 1000};
    static constexpr ProfileStrategy PROFILE_DYNAMIC_10 = {ProfileStrategy::DYNAMIC, 10};
    static constexpr ProfileStrategy PROFILE_DYNAMIC = {ProfileStrategy::DYNAMIC, 100};
    static constexpr std::array<ProfileStrategy, 2> PROFILE_STRATEGIES = {PROFILE_STATIC_1000, PROFILE_DYNAMIC_10};
    //   static constexpr std::array PROFILE_STRATEGIES = {PROFILE_STATIC_1000};

    static constexpr bool BEGIN_PROFILING_AUTOMATICALLY = true;

#ifndef NDEBUG
    static constexpr bool IS_PROFILING = false; // debug
#else
    static constexpr bool IS_PROFILING = true; // release
#endif

private:
    Spire::Engine &m_engine;
    SpireVoxel::VoxelRenderer &m_voxelRenderer;
    std::size_t m_profileStrategyIndex = 0;
    std::string m_profileJson;
    Spire::Timer m_timeSinceBeginProfiling;
    glm::u32 m_profilingStartedFrame = 0;
};
