#include "log.hpp"
#include <thread>
#include <vector>
#include <chrono>
#include <iostream>
#include <future>
#include <execution>

using namespace log4cpp;


void thread_func(Logger::ptr logger){
    LOG_INFO(logger) << "Thread id: " << std::this_thread::get_id();
}

void log_test_multithread(){
    auto logger1 = simple_init("logger1");
    // auto logger2 = simple_init("logger1");
    // auto logger3 = simple_init("logger2", "file");

    std::vector<std::jthread> threads1;
    // std::vector<std::jthread> threads2;
    // std::vector<std::jthread> threads3;
    
    for(int i=0; i<1000; i++){
        threads1.emplace_back(thread_func, logger1);
        // threads2.emplace_back(thread_func, logger2);
        // threads1.emplace_back(thread_func, logger3);
    }
    // 异步
    // std::async(std::launch::async, thread_func, logger1);
}

void log_test_parallel(){
    // auto logger1 = simple_init("logger1");
    // auto logger2 = simple_init("logger1");
    // // auto logger3 = simple_init("logger2", "file");

    std::vector<Logger::ptr> loggers;

    // std::for_each(std::execution::par, loggers.begin(), loggers.end(), thread_func);
    for(int i=0; i<1000; i++){
        auto logger = simple_init(std::string("logger")+std::to_string(i));
        loggers.push_back(logger);
    }
    std::for_each(std::execution::par, loggers.begin(), loggers.end(), thread_func);
}

int main(){
    auto start1 = std::chrono::high_resolution_clock::now();
    log_test_multithread();
    auto end1 = std::chrono::high_resolution_clock::now();
    double duration1 = std::chrono::duration_cast<std::chrono::duration<double>>(end1 - start1).count();
    std::cout << "Time taken by log_test_multithread() function: " << duration1 << " s" << std::endl;


    // auto start2 = std::chrono::high_resolution_clock::now();
    // log_test_parallel();
    // auto end2 = std::chrono::high_resolution_clock::now();
    // auto duration2 = std::chrono::duration_cast<std::chrono::duration<double>>(end2 - start2).count();
    // std::cout << "Time taken by log_test_parallel() function: " << duration2 << " s" << std::endl;
    return 0;
}