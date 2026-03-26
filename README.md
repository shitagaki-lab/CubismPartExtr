## Prerequsites

Clone this repo, download [Cubism SDK](https://www.live2d.com/en/sdk/download/native/) (We're using CubismSdkForNative-5-r.4.1), extract CubismSdkForNative-5-r.4.1 and copy following content into this repo's directory:
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


```
sudo apt install clang
sudo apt-get install cmake xvfb bison autoconf autoconf-archive automake libtool
```

xvfb is only required for headless environment

## Build the project
```
cmake --preset part_extract_release
cmake --build ./build --target Demo
```

## Run
```
cd build/bin/PartExtr
./PartExtr ../../../configs/test.yaml
```
If you're in a headless env, run xvfb on the background
```
Xvfb :99
```
export DISPLAY=:99 and run the executable.



### Debugging (vscode)

Install these extension:
```
cmake tools
clangd
Extension Pack for C/C++

```

In the panel of CMake extension, set Configure to `part_extract`


Run xvfb on the background
```
Xvfb :99
```
Select `part_extract_release`
Select debugging option `(lldb) xvfb part extract`