cmake_minimum_required(VERSION 3.10)
# set(CMAKE_C_COMPILER /usr/bin/clang)
# set(CMAKE_CXX_COMPILER /usr/bin/clang++)
# set(CMAKE_C_COMPILER /usr/bin/gcc-14)
# set(CMAKE_CXX_COMPILER /usr/bin/g++-14)

# 项目名称和版本
project(log4cpp VERSION 1.0)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 如果是多个源文件，可以使用 file(GLOB ...) 来自动添加
file(GLOB SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/log4cpp/src/*.cpp
    # 添加其他路径
)

# 添加头文件搜索路径
include_directories(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/log4cpp/include
    # 添加其他路径
)

# 创建一个名为 TemplateLib 的库
add_library(log4cppLib ${SOURCES})

# 指定生成的目标
# add_executable(main ${CMAKE_CURRENT_SOURCE_DIR}/sylar/src/main.cpp)


# 设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 添加子目录（如果有CMakeLists.txt文件的话）
# add_subdirectory(sub_director_name)

# 如果有库文件需要链接，使用以下命令
# target_link_libraries(main PRIVATE SylarLib)

# 如果需要包含外部项目或库，可能需要添加find_package或include_directories
# find_package(SomePackage REQUIRED)
# include_directories(${SomePackage_INCLUDE_DIRS})
# target_link_libraries(main PRIVATE ${SomePackage_LIBRARIES})

# 配置测试 (如果需要)
enable_testing()
add_subdirectory(tests)
