# retinanet_cpp

Simple packaging of the code published by NVidia
[here](https://github.com/NVIDIA/retinanet-examples)


# Installation

```bash
mkdir build
cd build
cmake ..
make -j16
sudo make install
```

...should suffice 🙃
The library is built 3 times, for each major configuration (`Release`,
`RelWithDebInfo`, `Debug`).


# Usage

```cmake
find_package(retinanet REQUIRED)

# ...

target_link_libraries(foo PRIVATE retinanet)
```

The configuration to link against is chosen this way:

1. Check if `RETINANET_BUILD_TYPE` is defined - if so, use it, else...
2. Check if `CMAKE_BUILD_TYPE` is defined it - if so, use it, else...
3. Default to `RelWithDebInfo`


# License

The code was published under BSD by NVidia, and a copy of it should be found in
this repository.
