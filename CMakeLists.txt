cmake_minimum_required(VERSION 3.12)
project(j3dconv)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)

add_subdirectory(lib/tinygltf)
add_subdirectory(lib/glm)
add_subdirectory(lib/TriStripper)

file(GLOB J3DCONV_SRC
    # J3DConv
    "src/*.cpp"
    "include/*.hpp"
    
    # bStream
    "lib/bStream/*.cpp"
)

add_library(j3dconv ${J3DCONV_SRC})
target_include_directories(j3dconv PUBLIC include lib/bStream lib/tinygltf lib/glm lib/TriStripper/include ${ZLIB_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})
target_link_libraries(j3dconv PUBLIC tinygltf glm TriStripper ZLIB::ZLIB PNG::PNG)

option(J3DCONV_BUILD_TESTER "Builds the tester program" ON)
if (J3DCONV_BUILD_TESTER)
  add_executable(tester src/tester.cpp)
  target_include_directories(tester PUBLIC include lib/bStream lib/tinygltf)
  target_link_libraries(tester PUBLIC j3dconv tinygltf)
endif (J3DCONV_BUILD_TESTER)