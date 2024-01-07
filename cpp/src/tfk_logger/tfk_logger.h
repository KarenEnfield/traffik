#include <cstdlib>

//#define USE_SPDLOG

#ifdef USE_SPDLOG
#include "spdlog/spdlog.h"
#else
#include <iostream>
#include <sstream>
#endif

class tfk_logger{
    private:
#ifdef USE_SPDLOG    
        std::shared_ptr<spdlog::logger> console_logger_;
        std::shared_ptr<spdlog::logger> file_logger_;
#else
        std::string console_name_;        
#endif        
    public:
        tfk_logger(std::string console_name);
        // Log messages with various variables
    void logDebug(const std::string& message) {
 #ifdef USE_SPDLOG       
        console_logger_->debug(message);
 #else
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "][debug] "<< message << std::endl;
 #endif    

    }

    template <typename Arg1, typename... Args>
    void logDebug(const std::string& message, const Arg1& arg1, const Args&... args) {
        
#ifdef USE_SPDLOG
        console_logger_->info(message.c_str(), arg1, args...);
        // If USE_SPDLOG is defined, use spdlog
        // console_logger_->info(message.c_str(), args...);
#else
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "][debug] " << formattedMessage.str() << std::endl;
#endif
    }


    void logInfo(const std::string& message) {
 #ifdef USE_SPDLOG       
        console_logger_->info(message);
 #else
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "][info]  "<< message << std::endl;
 #endif    

    }

    template <typename Arg1, typename... Args>
    void logInfo(const std::string& message, const Arg1& arg1, const Args&... args) {
        
#ifdef USE_SPDLOG
        console_logger_->info(message.c_str(), arg1, args...);
        // If USE_SPDLOG is defined, use spdlog
        // console_logger_->info(message.c_str(), args...);
#else
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "][info]  " << formattedMessage.str() << std::endl;
#endif
    }

    void logWarn(const std::string& message) {
 #ifdef USE_SPDLOG           
        console_logger_->warn(message);
 #else
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "][warn]  "<< message << std::endl;      
 #endif       
    }

    template <typename Arg1, typename... Args>
    void logWarn(const std::string& message, const Arg1& arg1, const Args&... args) {
    #ifdef USE_SPDLOG 
        console_logger_->warn(message.c_str(), arg1, args...);
    #else
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "][warn]  " << formattedMessage.str() << std::endl;

    #endif
    }

    void logError(const std::string& message) {
    #ifdef USE_SPDLOG    
        console_logger_->error(message);
    #else
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "][ERROR] "<< message << std::endl;  
    #endif     
    }

    template <typename Arg1, typename... Args>
    void logError(const std::string& message, const Arg1& arg1, const Args&... args) {
    #ifdef USE_SPDLOG    
        console_logger_->error(message.c_str(), arg1, args...);
    #else
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "][ERROR] " << formattedMessage.str() << std::endl;
    #endif    
    }

    void logCritical(const std::string& message) {
    #ifdef USE_SPDLOG    
        console_logger_->critical(message);
    #else
        std::cerr <<"[" << get_current_time() << "] [" << console_name_ << "][CRITICAL] "<< message << std::endl;
    #endif    
    }

    template <typename Arg1, typename... Args>
    void logCritical(const std::string& message, const Arg1& arg1, const Args&... args) {
    #ifdef USE_SPDLOG    
        console_logger_->critical(message.c_str(), arg1, args...);
    #else
        // If not defined, use std::cerr
        std::stringstream formattedMessage;
        logMultipleVariables(formattedMessage, message, arg1, args...);
        std::cerr << "[" << get_current_time() << "] [" << console_name_ << "][CRITICAL] " << formattedMessage.str() << std::endl;

    #endif
    }
    

    private:
    #ifndef USE_SPDLOG
    // Get the current time as a string
    std::string get_current_time();

    // Base case for the recursive template function
    void logMultipleVariables(std::stringstream& ss, const std::string& message);
    
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
