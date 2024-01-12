

#include "tfk_logger.h"


tfk_logger::tfk_logger(const char * console_name, const char *logLevel): console_name_(console_name){
        
    // Set the log level for all loggers
 
    // Get log level from environment variable, default to info if not set
    const char* logLevelStr = std::getenv("TFK_LOG_LEVEL");
    // a log level override was specified
    if (logLevel!=nullptr) {
        stringToLogLevel(logLevel);
    }
    else if (logLevelStr!=nullptr) // if there is a level specified
    {   std::string str;
        str.assign(logLevelStr);
        stringToLogLevel(str);
    }
    else
        stringToLogLevel(log_level_str_);
        
#ifdef USE_SPDLOG    

    // Create a console logger
    console_logger_ = spdlog::stdout_logger_mt(console_name_);
       
    // Log messages with different severity levels
    logTrace("This is an spdlog trace message.");
    logDebug("This is an spdlog debug message.");
    logInfo("This is an spdlog information message.");
    logWarn("This is an spdlog warning message.");
    logError("This is an spdlog error message.");
    logCritical("This is an spdlog critical message.");
  
    // Create a file logger
    // _file_logger = spdlog::rotating_logger_mt("file_logger", "logs/mylogfile", 1048576 * 5, 3);
   
    // Log messages
    //_console_logger->info("This is an information message on the console.");
    //_file_logger->warn("This is a warning message logged to the file.");

    // Use formatting
    // int value = 42;
    //_console_logger->error("An error message with a formatted value: {}", value);

    // Flush loggers (this can be important for asynchronous loggers)
    // spdlog::flush_all();
#else    
    // Log messages with different severity levels
    logTrace("This is a trace message.");
    logDebug("This is a debug message.");
    logInfo("This is an information message.");
    logWarn("This is a warning message.");
    logError("This is an error message.");
    logCritical("This is a critical message.");
 
#endif

  
}


// Helper function to convert a string to kCustomLogLevel
void tfk_logger::stringToLogLevel(std::string levelStr)
{
    // change the internal logging level if valid one is defined
    const auto& a = mpCustomLogLevel.find(levelStr) ;
    if (a!=mpCustomLogLevel.end()){
        log_level_str_ = levelStr;
        log_level_ = a->second;
    }
    
#ifdef USE_SPDLOG
    // set spdlog level
    if (log_level_ == kCustomLogLevel::trace) spdlog::set_level(spdlog::level::trace);
    else if (log_level_ == kCustomLogLevel::debug)  spdlog::set_level(spdlog::level::debug);
    else if (log_level_ == kCustomLogLevel::info)  spdlog::set_level(spdlog::level::info);
    else if (log_level_ == kCustomLogLevel::warn)  spdlog::set_level(spdlog::level::warn);
    else if (log_level_ == kCustomLogLevel::err)  spdlog::set_level(spdlog::level::err);
    else if (log_level_ == kCustomLogLevel::critical)  spdlog::set_level(spdlog::level::critical);
    else if (log_level_ == kCustomLogLevel::off)  spdlog::set_level(spdlog::level::off);
    // Default to info if the input is not recognized
    else {  
        // should not get here
        spdlog::set_level(spdlog::level::info);
        log_level_ = kCustomLogLevel::info;
        log_level_str_ = "info";
    }

#endif
    


}
