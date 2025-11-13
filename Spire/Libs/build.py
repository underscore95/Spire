import os
import subprocess
import shutil
import time

start_time = time.time()
os.environ["PATH"] += r";C:\Program Files\CMake\bin"

libs = ["assimp", "glfw", "glm", "glslang", "imgui", "googletest"]
build_root_release = os.path.abspath("build-release")
build_root_debug = os.path.abspath("build-debug")
cmake_generator = "Visual Studio 17 2022"

for build_type, build_root in [("Release", build_root_release), ("Debug", build_root_debug)]:
    os.makedirs(build_root, exist_ok=True)

    for lib in libs:
        src_dir = os.path.abspath(lib)
        lib_build_dir = os.path.join(build_root, lib)
        os.makedirs(lib_build_dir, exist_ok=True)

        print(f"Configuring {lib} ({build_type})...")
        subprocess.run(
            ["cmake", "-S", src_dir, "-B", lib_build_dir, f"-G{cmake_generator}", f"-DCMAKE_BUILD_TYPE={build_type}"],
            check=True
        )

        print(f"Building {lib} ({build_type})...")
        subprocess.run(["cmake", "--build", lib_build_dir, "--config", build_type], check=True)

        # Copy built libraries
        for root, _, files in os.walk(lib_build_dir):
            for file in files:
                if file.endswith((".lib", ".a", ".dll", ".so")):
                    shutil.copy(os.path.join(root, file), build_root)

    print(f"All {build_type} libraries built and copied to:", build_root)

end_time = time.time()
elapsed = end_time - start_time
print(f"Total build time: {elapsed:.2f} seconds")
