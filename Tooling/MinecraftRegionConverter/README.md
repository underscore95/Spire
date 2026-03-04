# Minecraft Region Converter v2

This is a C++ tool which can convert a Minecraft region file into a Spire world. 

On an 17-12700F (20 threads) it takes ~700ms to execute.

It can convert 379 Minecraft blocks into Spire blocks, approximately 25% of all Minecraft blocks. It supports:
- Cubes with a single texture, e.g. dirt
- Cubes with the same side texture, different top and different bottom texture, e.g. grass block
- Cubes with the same side texture, top and bottom textures are same but different from side, e.g. oak log
- Non flowing/non waterlogged water and lava are converted to blue_wool and red_wool
- Leaves are supported (without transparency)

## Usage

This is a command line tool.

Example Usage:

```
mre r.0.0.mca out minecraft MinecraftTypes.h -overwrite
```

Arguments:
- Name of region file to import, this can be found in `<world root dir>/regions/` and represents 32x32 Minecraft chunks
- Name of directory to store the Spire world in
- Name of minecraft folder, this can be obtained by going to `%appdata%/.minecraft/versions`, picking a version, opening the jar using an archive manager like 7zip (or renaming it to a .zip file) and moving the contents into this folder. If this argument is `minecraft`, you should have a directory called `minecraft` in the working directory which contains Minecraft's `assets` folder.
- Name of header to generate which registers some Minecraft block types into Spire
- optionally -overwrite to allow the converter to overwrite the output directory and file

The generated header assumes texture files all exist in `<assets>/Minecraft/`, you can copy the contents of `<minecraft folder>/assets/minecraft/textures/block/` into `<assets>/Minecraft`.

Copy the contents of the generated header into a header in your game application. Then register voxel types like so:
```
m_voxelRenderer = std::make_unique<VoxelRenderer>(*m_engine, *m_camera, std::move(tempWorld), [](VoxelTypeRegistry &voxelTypeRegistry) {
    RegisterMinecraftVoxelTypes(voxelTypeRegistry);
});
```

## Greenifier

Some Minecraft textures, such as the top of a grass block, or leaves, are grayscale. Minecraft will apply a modulate based on the current biome to render them correctly.

For example, jungle applies a vibrant green, desert applies a dried out greeny yellow colour.

Spire does not support this. A greenifier python script exists to automatically make grayscale images greener.

### Usage

- Copy all Minecraft block textures into a folder
- Run Python script which greenifies some textures
- Copy output textures into Spire assets folder

Example Usage:
`python greenifier.py block Minecraft`

Currently converts:
- oak_leaves.png
- birch_leaves.png
- spruce_leaves.png
- dark_oak_leaves.png
- jungle_leaves.png
- mangrove_leaves.png
- pale_oak_leaves.png
- acacia_leaves.png
- grass_block_top.png

A constant RGB is added/subtracted, meaning that textures will not look exactly like Minecraft, but similar.

## Credits

Depends on a modified version of https://github.com/dougbinks/enkiMI/tree/master which itself uses https://github.com/madler/zlib

https://github.com/nlohmann/json

https://www.minecraft.net/en-us

## History

This tool used to be implemented as a single threaded Python script which took over 20 minutes to execute.

Python script source code available here: https://github.com/underscore95/Spire/blob/72020a0f66de73615fd32f257f1c804d6cc50b80/Tooling/MinecraftRegionConverter/convert.py