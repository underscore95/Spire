# .sprc File Format Specification

CURRENT VERSION: 2

This file is intended to store all the data required for a single 64x64x64 chunk.

HEADER:
identifier - "SPRVXLCHNK" const char*
format version - u32
chunk x position - i32
chunk x position - i32
chunk x position - i32
voxel types - u16[64*64*64]