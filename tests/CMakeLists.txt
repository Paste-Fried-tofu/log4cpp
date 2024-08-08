# 收集所有的测试源文件
file(GLOB TEST_SOURCES "*.cpp")

# 创建测试可执行文件
add_executable(Tests ${TEST_SOURCES})

# 链接测试可执行文件与项目库
target_link_libraries(Tests PRIVATE log4cppLib)  # 链接主项目库

# 添加测试命令
add_test(NAME TestsRun COMMAND Tests)
