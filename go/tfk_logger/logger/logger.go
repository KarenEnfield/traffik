// tfk_logger/logger.go
package logger

import (
	"fmt"
	"log"
	"os"
)

// LogLevel represents log levels
type LogLevel int

const (
	Trace LogLevel = iota
	Debug
	Info
	Warning
	Error
	Critical
)

// Logger represents a logger instance
type Logger struct {
	name  string
	level LogLevel
}

// NewLogger creates a new Logger with a specific log level
func NewLogger(name string, level LogLevel) *Logger {
	return &Logger{name: name, level: level}
}

// SetLogLevel sets the log level for the logger
func (l *Logger) SetLogLevel(level LogLevel) {
	l.level = level
}

// logMessage logs a message with a specific log level
func (l *Logger) logMessage(level LogLevel, format string, a ...interface{}) {
	if level < l.level {
		return
	}

	logPrefix := ""
	switch level {
	case Trace:
		logPrefix = "trace"
	case Debug:
		logPrefix = "debug"
	case Info:
		logPrefix = "info"
	case Warning:
		logPrefix = "warning"
	case Error:
		logPrefix = "error"
	case Critical:
		logPrefix = "critical"
	}
	message := fmt.Sprintf(format, a...)
	fullMessage := fmt.Sprintf(" [%s] [%s] %s", l.name, logPrefix, message)

	log.Println(fullMessage)
}

// Trace logs a trace message
func (l *Logger) Trace(format string, a ...interface{}) {
	l.logMessage(Trace, format, a...)
}

// Debug logs a debug message
func (l *Logger) Debug(format string, a ...interface{}) {
	l.logMessage(Debug, format, a...)
}

// Info logs an info message
func (l *Logger) Info(format string, a ...interface{}) {
	l.logMessage(Info, format, a...)
}

// Warning logs a warning message
func (l *Logger) Warning(format string, a ...interface{}) {
	l.logMessage(Warning, format, a...)
}

// Error logs an error message
func (l *Logger) Error(format string, a ...interface{}) {
	l.logMessage(Error, format, a...)
}

// Critical logs a critical message
func (l *Logger) Critical(format string, a ...interface{}) {
	l.logMessage(Critical, format, a...)
}

// GetConfigFilePath returns the JSON file path from the environment variable or a default value
func GetLogLevel() LogLevel {
	// Check if the environment variable is set
	envVar := os.Getenv("TFK_LOG_LEVEL")

	switch envVar {
	case "trace":
		return Trace
	case "debug":
		return Debug
	case "info":
		return Info
	case "warning":
		return Warning
	case "error":
		return Error
	case "critical":
		return Critical
	}
	return Info
}

func init() {
	// Set log format to include date and time
	log.SetFlags(log.LstdFlags)
	// log.SetFlags(log.Ldate | log.Ltime | log.Lmicroseconds)
	// Set the log output to standard error
	log.SetOutput(os.Stderr)
}
