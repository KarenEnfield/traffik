// tfk_logger/logger.go
package tfk_logger

import (
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
	level LogLevel
}

// NewLogger creates a new Logger with a specific log level
func NewLogger(level LogLevel) *Logger {
	return &Logger{level: level}
}

// SetLogLevel sets the log level for the logger
func (l *Logger) SetLogLevel(level LogLevel) {
	l.level = level
}

// logMessage logs a message with a specific log level
func (l *Logger) logMessage(level LogLevel, message string) {
	if level < l.level {
		return
	}

	logPrefix := ""
	switch level {
	case Trace:
		logPrefix = "[trace]"
	case Debug:
		logPrefix = "[debug]"
	case Info:
		logPrefix = "[info]"
	case Warning:
		logPrefix = "[warning]"
	case Error:
		logPrefix = "[error]"
	case Critical:
		logPrefix = "[critical]"
	}

	log.Println(logPrefix, message)
}

// Trace logs a trace message
func (l *Logger) Trace(message string) {
	l.logMessage(Trace, message)
}

// Debug logs a debug message
func (l *Logger) Debug(message string) {
	l.logMessage(Debug, message)
}

// Info logs an info message
func (l *Logger) Info(message string) {
	l.logMessage(Info, message)
}

// Warning logs a warning message
func (l *Logger) Warning(message string) {
	l.logMessage(Warning, message)
}

// Error logs an error message
func (l *Logger) Error(message string) {
	l.logMessage(Error, message)
}

// Critical logs a critical message
func (l *Logger) Critical(message string) {
	l.logMessage(Critical, message)
}

func init() {
	// Set log format to include date and time
	log.SetFlags(log.LstdFlags)
	// Set the log output to standard error
	log.SetOutput(os.Stderr)
}
