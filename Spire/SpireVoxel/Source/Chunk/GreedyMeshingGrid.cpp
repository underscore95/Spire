#include "GreedyMeshingGrid.h"

static_assert(SPIRE_VOXEL_CHUNK_SIZE == 64); // required for greedy meshing

SpireVoxel::GreedyMeshingGrid::GreedyMeshingGrid(
    const std::vector<glm::u64> &source,
    std::size_t index) {
    assert(index + SPIRE_VOXEL_CHUNK_SIZE < source.size());
    memcpy(m_bits.data(), &source[index],SPIRE_VOXEL_CHUNK_SIZE * sizeof(source[index]));
}

void SpireVoxel::GreedyMeshingGrid::SetBit(glm::u32 row, glm::u32 col) {
    m_bits[col] |= static_cast<glm::u64>(1) << row;
}

bool SpireVoxel::GreedyMeshingGrid::GetBit(glm::u32 row, glm::u32 col) const {
    glm::u64 mask = static_cast<glm::u64>(1) << row; // e.g. 000100
    mask = m_bits[col] & mask; // if this is 0, the bit was 0
    return mask != 0;
}

void SpireVoxel::GreedyMeshingGrid::SetEmptyVoxels(glm::u32 col, glm::u32 row, glm::u32 height) {
    assert(height != 0);
    glm::u64 mask = UINT64_MAX; // this is for height == 64, since normally we'd be setting it to uint64 max + 1
    if (height < SPIRE_VOXEL_CHUNK_SIZE) {
        // todo: could a uint128 be used as the mask so we don't need an if statement?
        mask = static_cast<glm::u64>(1) << height; // if height == 2: 0000100
        mask--; // 0000011
    }
    mask = mask << row; // if row == 3: 0011000
    mask = ~mask; // flip the bits: 1100111

    m_bits[col] = m_bits[col] & mask; // set the voxels to empty
}

glm::uvec3 SpireVoxel::GreedyMeshingGrid::GetChunkCoords(glm::u32 slice, glm::u32 row, glm::u32 col, glm::u32 face) {
    switch (face) {
        case SPIRE_VOXEL_FACE_POS_Z:
        case SPIRE_VOXEL_FACE_NEG_Z:
            return {col, row, slice};
        case SPIRE_VOXEL_FACE_POS_X:
        case SPIRE_VOXEL_FACE_NEG_X:
            return {slice, row, col};
        case SPIRE_VOXEL_FACE_POS_Y:
        case SPIRE_VOXEL_FACE_NEG_Y:
            return {col, slice, row};
        default:
            assert(false);
            return {};
    }
}

void SpireVoxel::GreedyMeshingGrid::Print() const {
    std::array<std::string, SPIRE_VOXEL_CHUNK_SIZE> strings;
    for (std::size_t col = 0; col < m_bits.size(); col++) {
        for (glm::u32 row = 0; row < SPIRE_VOXEL_CHUNK_SIZE; row++) {
            strings[col] += GetBit(row, col) ? "1" : "0";
        }
    }

    for (const auto &str : strings) {
        Spire::info(str);
    }
}
