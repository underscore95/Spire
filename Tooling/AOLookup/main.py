# Generates a C++ file containing what ijk vectors are for each face and vertex position
# i is the unit vector equal to the normal of the face
# j and k are unit vectors which are perpendicular to themselves and to i where
# (voxel pos + j or k) is an adjacent neighbouring voxel closest to the vertex
# (voxel pos + j + k) is an adjacent neighbouring voxel closest to the vertex
#
# EXAMPLE FOR VERTEX B: (i is perendicular to your screen)
#
#      j (j+k)
#    a-b k
#    | |
#    d-c
#
# k=jxi j=ixk

def invert(v):
    return (-v[0],-v[1],-v[2])

def is_neg_face(face):
    return face % 2 == 1

def generate(path):
    FACES = [
        (1,0,0),
        (-1,0,0),
        (0,1,0),
        (0,-1,0),
        (0,0,1),
        (0,0,-1),
    ]

    table = []
    for i in range(4*6*3):
        table.append((0,0,0))

    for face in range(len(FACES)):
        for vertex in range(4):
            i = FACES[face]
            if face == 0 or face == 1: # x axis
                j = (0, 1, 0)
                k = (0, 0, 1)
                if vertex == 0 or vertex == 1:
                    j = invert(j)
                if vertex == 1 or vertex == 2:
                    k = invert(k)
                if is_neg_face(face):
                    k = invert(k)
            elif face == 2 or face == 3: # y axis
                j = (0, 0, 1)
                k = (1, 0, 0)
                if vertex == 2 or vertex == 3:
                    j = invert(j)
                if vertex == 0 or vertex == 3:
                    k = invert(k)
                if is_neg_face(face):
                    j = invert(j)
            elif face == 4 or 5: # z axis
                j = (1, 0, 0)
                k = (0, 1, 0)
                if vertex == 0 or vertex == 3:
                    j = invert(j)
                if vertex == 0 or vertex == 1:
                    k = invert(k)
                if is_neg_face(face):
                    j = invert(j)
            table[face * 4 * 3 + vertex * 3 + 0] = i;
            table[face * 4 * 3 + vertex * 3 + 1] = j;
            table[face * 4 * 3 + vertex * 3 + 2] = k;

    with open(path, "w") as file:
        file.write("#pragma once\n\n#include \"EngineIncludes.h\"\n\n// This file was generated automatically by Tooling/AOLookup\n\nnamespace SpireVoxel {")
        file.write("\n    inline void GetAmbientOcclusionOffsetVectors(glm::u32 face, glm::u32 vertexPos, glm::ivec3 &i, glm::ivec3 &j, glm::ivec3 &k) {\n")
        file.write("        assert(face < 6);\n")
        file.write("        assert(vertexPos < 4);\n")
        file.write("        constexpr std::array AO_OFFSET_TABLE = {\n")
        for vec in table:
            file.write("            glm::ivec3{" + str(vec[0]) + ", " + str(vec[1]) + ", " + str(vec[2])  + "},\n");
        file.write("        };\n\n")
        file.write("        i = AO_OFFSET_TABLE[face * 4 * 3 + vertexPos * 3 + 0];\n")
        file.write("        j = AO_OFFSET_TABLE[face * 4 * 3 + vertexPos * 3 + 1];\n")
        file.write("        k = AO_OFFSET_TABLE[face * 4 * 3 + vertexPos * 3 + 2];\n")

        file.write("    }\n}\n")

generate("AOLookupTable.h")