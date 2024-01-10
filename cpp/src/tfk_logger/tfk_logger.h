#pragma once 
#include <cstdlib>

// To enable/disable, see the definition in CMakeLists.txt
#ifdef USE_SPDLOG
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"

#else
#include <iostream>
#include <sstream>
#include <chrono>
#endif

    
class tfk_logger{

    enum class CustomLogLevel
    {
        trace,
        debug,
        info,
        warn,
        err,
        critical,
        off
    };

    std::string console_name_; 
    // Custom level same levels as found in spdlog::level::level_enum
    CustomLogLevel log_level_{CustomLogLevel::info};
    std::string log_level_str_{"info"};
#ifdef USE_SPDLOG    
    std::shared_ptr<spdlog::logger> console_logger_;
    //std::shared_ptr<spdlog::logger> file_logger_;
#endif

    // Set the log level for all loggers
    void stringToLogLevel(std::string levelStr) ;

public:
    tfk_logger(std::string console_name);

    

    // Log messages with various variables
    void logDebug(const std::string& message) {
 #ifdef USE_SPDLOG       
        console_logger_->debug(message);
 #else
    if (log_level_ <= CustomLogLevel::info)
    {
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "][debug] "<< message << std::endl;
    }
 #endif    

    }

    template <typename Arg1, typename... Args>
    void logDebug(const std::string& message, const Arg1& arg1, const Args&... args) {
        
#ifdef USE_SPDLOG
        console_logger_->info(message.c_str(), arg1, args...);
#else
    if (log_level_ <= CustomLogLevel::info)
    {
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "][debug] " << formattedMessage.str() << std::endl;
    }
#endif
    }


    void logInfo(const std::string& message) {
 #ifdef USE_SPDLOG       
        console_logger_->info(message);
 #else
    if (log_level_ <= CustomLogLevel::info)
    {
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "][info]  "<< message << std::endl;
    }
 #endif    

    }

    template <typename Arg1, typename... Args>
    void logInfo(const std::string& message, const Arg1& arg1, const Args&... args) {
        
#ifdef USE_SPDLOG
        console_logger_->info(message.c_str(), arg1, args...);
        // console_logger_->info(message.c_str(), args...);
#else
    if (log_level_ <= CustomLogLevel::info)
    {
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "][info]  " << formattedMessage.str() << std::endl;
    }
#endif
    }

    void logWarn(const std::string& message) {
 #ifdef USE_SPDLOG           
        console_logger_->warn(message);
 #else
    if (log_level_ <= CustomLogLevel::info)
    {
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "][warn]  "<< message << std::endl;      
    }
 #endif       
    }

    template <typename Arg1, typename... Args>
    void logWarn(const std::string& message, const Arg1& arg1, const Args&... args) {
#ifdef USE_SPDLOG 
        console_logger_->warn(message.c_str(), arg1, args...);
#else
    if (log_level_ <= CustomLogLevel::info)
    {
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "][warn]  " << formattedMessage.str() << std::endl;
    }
#endif
    }

    void logError(const std::string& message) {
#ifdef USE_SPDLOG    
        console_logger_->error(message);
#else
    if (log_level_ <= CustomLogLevel::info)
    {
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "][ERROR] "<< message << std::endl;  
    }
#endif     
    }

    template <typename Arg1, typename... Args>
    void logError(const std::string& message, const Arg1& arg1, const Args&... args) {
#ifdef USE_SPDLOG    
        console_logger_->error(message.c_str(), arg1, args...);
#else
    if (log_level_ <= CustomLogLevel::info)
    {
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "][ERROR] " << formattedMessage.str() << std::endl;
    }
#endif    
    }

    void logCritical(const std::string& message) {
#ifdef USE_SPDLOG    
        console_logger_->critical(message);
#else
    if (log_level_ <= CustomLogLevel::info)
    {
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "][CRITICAL] "<< message << std::endl;
    }
#endif    
    }

    template <typename Arg1, typename... Args>
    void logCritical(const std::string& message, const Arg1& arg1, const Args&... args) {
#ifdef USE_SPDLOG    
        console_logger_->critical(message.c_str(), arg1, args...);
#else
    if (log_level_ <= CustomLogLevel::info)
    {
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "][CRITICAL] " << formattedMessage.str() << std::endl;
    }
#endif
    }
    
    std::string getLogLevel()
    {
        return log_level_str_;
    }

#ifndef USE_SPDLOG    
    std::string get_current_time(){
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
    void logMultipleVariables(std::stringstream& ss, const std::string& message){
        ss << message;
    }

    // Helper function for formatting the message
    template <typename T, typename... Args>
    void logMultipleVariables(std::stringstream& ss, const std::string& message, const T& arg, const Args&... args){
        size_t pos = message.find("{}");
        if (pos != std::string::npos) {
            ss << message.substr(0, pos) << arg;
            logMultipleVariables(ss, message.substr(pos + 2), args...);
        } else {
            ss << message;
            logMultipleVariables(ss, "", args...);
        }
    }
#endif
};