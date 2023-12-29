
#include "tfk_logger.h"
#include <iostream>

#ifdef USE_SPDLOG
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"
//#include "spdlog/sinks/stdout_color_sinks.h"
//#include "spdlog/sinks/rotating_file_sink.h"
#endif


tfk_logger::tfk_logger(std::string console_name) {

#ifdef USE_SPDLOG    
    // Set the log level for all loggers
    spdlog::set_level(spdlog::level::info);

    // Create a console logger
    console_logger_ = spdlog::stdout_logger_mt(console_name);
       
    // Log messages with different severity levels
    console_logger_->trace("This is a trace message.");
    console_logger_->debug("This is a debug message.");
    console_logger_->info("This is an information message.");
    console_logger_->warn("This is a warning message.");
    console_logger_->error("This is an error message.");
    console_logger_->critical("This is a critical message.");
  
    // Create a file logger
    // _file_logger = spdlog::rotating_logger_mt("file_logger", "logs/mylogfile", 1048576 * 5, 3);
   
    // Set the log level for both loggers
    // spdlog::set_level(spdlog::level::debug);

    // Log messages
    //_console_logger->info("This is an information message on the console.");
    //_file_logger->warn("This is a warning message logged to the file.");

    // Use formatting
    // int value = 42;
    //_console_logger->error("An error message with a formatted value: {}", value);

    // Flush loggers (this can be important for asynchronous loggers)
    // spdlog::flush_all();
#else
    console_name_ = console_name; 
#endif   
}

#ifndef USE_SPDLOG
std::string tfk_logger::get_current_time(){
    // Get the current time point
    auto currentTimePoint = std::chrono::system_clock::now();

    // Convert the current time point to a time_t object
    std::time_t currentTime = std::chrono::system_clock::to_time_t(currentTimePoint);

    // Convert the time_t to a struct tm in local time
    struct tm* timeInfo = std::localtime(&currentTime);

    // Format and print the current local time
    char timeStr[100];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeInfo);

    //std::cout << "Current local time: " << timeStr << std::endl;
    return timeStr;
}

// Base case for the recursive template function
void tfk_logger::logMultipleVariables(std::stringstream& ss, const std::string& message) {
    ss << message;
}

#endif 