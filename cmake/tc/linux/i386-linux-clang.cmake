# Building in 32-bit mode seems to fail due to a 64-bit Vulkan library:
#       /usr/bin/ld: /lib/libvulkan.so: error adding symbols: file in wrong format
# If anyone compiles this in 32-bit mode and they DON'T get this error, please file an issue at https://github.com/kosude/thallium.

set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_FLAGS "-m32")
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "-m32")
