
The following instructions are for Linux; however, it should be able to build and run on Windows and macOS as well, provided that the building tools (mainly CMake, Clang, and Git) are properly installed.


## Prerequsites

(for ubuntu)
```
sudo apt install clang
sudo apt-get install cmake xvfb bison autoconf autoconf-archive automake libtool
```

xvfb is only required for headless environment.


Clone this repo, download [Cubism SDK](https://www.live2d.com/en/sdk/download/native/) (We're using CubismSdkForNative-5-r.4.1), extract CubismSdkForNative-5-r.4.1 and copy these folders into this repo's directory:
```
Samples
Core
```

Install vcpkg some where:
```
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
```

Open `CMakePresets.json`, set `VCPKG_ROOT` to your vcpkg install path.


## Build the project
If you're in a conda environment, some installed packages may mess up the build process, deactive conda or activate a clean env.
```
cmake --preset part_extract_release
cmake --build ./build
```

## Run
```
cd build/bin/PartExtr
./PartExtr ../../../configs/test.yaml
```
Extraction results will be saved to `partextr_output`, please refer to the config file `test.yaml` for more information.

If you're in a headless environment, run xvfb in the background first:
```
Xvfb :99
```
export DISPLAY=:99 and run the executable.



## Debugging (VSCode)

Install these extension:
```
cmake tools
clangd
Extension Pack for C/C++

```

In the CMake extension panel, set `Configure` to `part_extract` and set the debug option to `(lldb) part extract`.


For headless env, run xvfb in the background and set the debug option to `(lldb) xvfb part extract`


## LICENSE
The codebase is primarily built upon https://github.com/Live2D/CubismNativeSamples

[LICENSE](LICENSE.md)