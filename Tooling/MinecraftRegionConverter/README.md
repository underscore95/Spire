# Minecraft Region Converter v2

This is a C++ tool which can convert a Minecraft region file into a Spire world. 

On an 17-12700F (20 threads) it takes ~500ms to execute.

## Usage

This is a command line tool. It takes in an input region file and output directory.

```
mre r.0.0.mca out
```

## Credits

Depends on a modified version of https://github.com/dougbinks/enkiMI/tree/master which itself uses https://github.com/madler/zlib

## History

This tool used to be implemented as a single threaded Python script which took over 20 minutes to execute.

Python script source code available here: https://github.com/underscore95/Spire/blob/72020a0f66de73615fd32f257f1c804d6cc50b80/Tooling/MinecraftRegionConverter/convert.py