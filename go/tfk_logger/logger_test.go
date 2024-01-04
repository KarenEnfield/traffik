// logger/logger_test.go
package logger

import (
	"bytes"
	"log"
	"os"
	"strings"
	"testing"
)

func TestLogger(t *testing.T) {
	// Create a buffer to capture log output
	var buf bytes.Buffer

	// Redirect log output to the buffer
	log.SetOutput(&buf)

	// Create a logger with the INFO log level
	logger := NewLogger(Info)

	// Log messages at different levels
	logger.Trace("This is a trace message.")
	logger.Debug("This is a debug message.")
	logger.Info("This is an info message.")
	logger.Warning("This is a warning message.")
	logger.Error("This is an error message.")
	logger.Critical("This is a critical message.")

	// Reset the log output to the default (standard error)
	log.SetOutput(os.Stderr)

	// Check the log messages in the buffer
	got := buf.String()

	// Assert that the expected log messages are present
	assertLogContains(t, got, "[info] This is an info message.")
	assertLogContains(t, got, "[warning] This is a warning message.")
	assertLogContains(t, got, "[error] This is an error message.")
	assertLogContains(t, got, "[critical] This is a critical message.")
	// Assert that the unexpected log messages are not present
	assertLogNotContains(t, got, "[trace] This is a trace message.")
	assertLogNotContains(t, got, "[debug] This is a debug message.")
}

func assertLogContains(t *testing.T, logOutput, expected string) {
	t.Helper()
	if !strings.Contains(logOutput, expected) {
		t.Errorf("Log output does not contain expected message: %s", expected)
	}
}

func assertLogNotContains(t *testing.T, logOutput, unexpected string) {
	t.Helper()
	if strings.Contains(logOutput, unexpected) {
		t.Errorf("Log output contains unexpected message: %s", unexpected)
	}
}
