#pragma once 
#include <cstdlib>
#include <map>

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

    enum class kCustomLogLevel
    {
        trace,
        debug,
        info,
        warn,
        err,
        critical,
        off
    };
    const std::map<std::string,kCustomLogLevel> mpCustomLogLevel
    {
        {"trace",kCustomLogLevel::trace},
        {"debug",kCustomLogLevel::debug},
        {"info", kCustomLogLevel::info},
        {"warning",kCustomLogLevel::info},
        {"error",kCustomLogLevel::err},
        {"critical",kCustomLogLevel::critical},
        {"off",kCustomLogLevel::off}
    };

    
    std::string console_name_; 

    // Custom level same levels as found in spdlog::level::level_enum
    
#ifdef TFK_LOG_ERROR    
    std::string log_level_str_{"error"};
    kCustomLogLevel log_level_{kCustomLogLevel::err};
#else
    std::string log_level_str_{"info"};
    kCustomLogLevel log_level_{kCustomLogLevel::info};
#endif

#ifdef USE_SPDLOG    
    std::shared_ptr<spdlog::logger> console_logger_;
    //std::shared_ptr<spdlog::logger> file_logger_;
#endif

    // Set the log level for all loggers
    void stringToLogLevel(std::string levelStr) ;

public:
    tfk_logger(const char * console_name, const char * logLevel=nullptr);

    // Log messages with various variables
   
    void logTrace(const std::string& message) {
 #ifdef USE_SPDLOG       
        console_logger_->trace(message);
 #else
    if (log_level_ <= kCustomLogLevel::trace)
    {
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "] [trace] "<< message << std::endl;
    }
 #endif    

    }

    template <typename Arg1, typename... Args>
    void logTrace(const std::string& message, const Arg1& arg1, const Args&... args) {
        
#ifdef USE_SPDLOG
        console_logger_->trace(message.c_str(), arg1, args...);
#else
    if (log_level_ <= kCustomLogLevel::trace)
    {
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "] [trace] " << formattedMessage.str() << std::endl;
    }
#endif
    }


    void logDebug(const std::string& message) {
 #ifdef USE_SPDLOG       
        console_logger_->debug(message);
 #else
    if (log_level_ <= kCustomLogLevel::debug)
    {
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "] [debug] "<< message << std::endl;
    }
 #endif    

    }

    template <typename Arg1, typename... Args>
    void logDebug(const std::string& message, const Arg1& arg1, const Args&... args) {
        
#ifdef USE_SPDLOG
        console_logger_->debug(message.c_str(), arg1, args...);
#else
    if (log_level_ <= kCustomLogLevel::debug)
    {
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "] [debug] " << formattedMessage.str() << std::endl;
    }
#endif
    }


    void logInfo(const std::string& message) {
 #ifdef USE_SPDLOG       
        console_logger_->info(message);
 #else
    if (log_level_ <= kCustomLogLevel::info)
    {
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "] [info]  "<< message << std::endl;
    }
 #endif    

    }

    template <typename Arg1, typename... Args>
    void logInfo(const std::string& message, const Arg1& arg1, const Args&... args) {
        
#ifdef USE_SPDLOG
        console_logger_->info(message.c_str(), arg1, args...);
        
#else
    if (log_level_ <= kCustomLogLevel::info)
    {
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "] [info]  " << formattedMessage.str() << std::endl;
    }
#endif
    }

    void logWarn(const std::string& message) {
 #ifdef USE_SPDLOG           
        console_logger_->warn(message);
 #else
    if (log_level_ <= kCustomLogLevel::warn)
    {
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "] [warn]  "<< message << std::endl;      
    }
 #endif       
    }

    template <typename Arg1, typename... Args>
    void logWarn(const std::string& message, const Arg1& arg1, const Args&... args) {
#ifdef USE_SPDLOG 
        console_logger_->warn(message.c_str(), arg1, args...);
#else
    if (log_level_ <= kCustomLogLevel::warn)
    {
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "] [warn]  " << formattedMessage.str() << std::endl;
    }
#endif
    }

    void logError(const std::string& message) {
#ifdef USE_SPDLOG    
        console_logger_->error(message);
#else
    if (log_level_ <= kCustomLogLevel::err)
    {
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "] [error] "<< message << std::endl;  
    }
#endif     
    }

    template <typename Arg1, typename... Args>
    void logError(const std::string& message, const Arg1& arg1, const Args&... args) {
#ifdef USE_SPDLOG    
        console_logger_->error(message.c_str(), arg1, args...);
#else
    if (log_level_ <= kCustomLogLevel::err)
    {
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "] [error] " << formattedMessage.str() << std::endl;
    }
#endif    
    }

    void logCritical(const std::string& message) {
#ifdef USE_SPDLOG    
        console_logger_->critical(message);
#else
    if (log_level_ <= kCustomLogLevel::critical)
    {
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "] [critical] "<< message << std::endl;
    }
#endif    
    }

    template <typename Arg1, typename... Args>
    void logCritical(const std::string& message, const Arg1& arg1, const Args&... args) {
#ifdef USE_SPDLOG    
        console_logger_->critical(message.c_str(), arg1, args...);
#else
    if (log_level_ <= kCustomLogLevel::critical)
    {
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "] [critical] " << formattedMessage.str() << std::endl;
    }
#endif
    }
    
    const char * getLogLevel()
    {
        return log_level_str_.c_str();
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