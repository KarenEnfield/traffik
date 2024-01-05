// tfk_servers/server/server.go
package server

import (
	"context"
	"fmt"
	"math/rand"
	"net/http"
	"strconv"
	"sync"
	"time"

	"github.com/KarenEnfield/traffik/go/tfk_logger/logger"
)

// Server represents a server
type Server struct {
	Name           string
	Port           int
	Type           string
	Message        string
	DataLength     int
	ErrorCode      int
	Duration       string
	TimeoutSeconds int
	logLevel       logger.LogLevel
	ActivityChan   chan bool
	stopChan       chan bool
	inactivityDur  time.Duration
	lock           sync.Mutex
	lastActivity   time.Time
}

// NewServer creates a new instance of Server
func NewServer(port int, name string, return_type string, message string, data_length int, error_code int, duration string, timeout int, level logger.LogLevel) *Server {
	new_server := Server{
		Name:           name,
		Port:           port,
		Type:           return_type,
		Message:        message,
		DataLength:     data_length,
		ErrorCode:      error_code,
		Duration:       duration,
		TimeoutSeconds: timeout,
		logLevel:       level,
		ActivityChan:   make(chan bool),
		stopChan:       make(chan bool),
		inactivityDur:  time.Duration(timeout * int(time.Second)),
		lastActivity:   time.Now(),
	}

	// Override fields with non-zero values from userConfig
	switch new_server.Type {
	case "message":
		new_server.DataLength = len(new_server.Message)
		new_server.ErrorCode = 200

	case "random":
		if new_server.DataLength < 0 {
			new_server.DataLength = 50
		}
		new_server.ErrorCode = 200
		new_server.Message = ""

	case "error":
		if new_server.ErrorCode <= 0 {
			new_server.ErrorCode = 200
		}
		new_server.DataLength = 0
		new_server.Message = ""

	default:
		new_server.Type = "message"
		new_server.DataLength = len(new_server.Message)
		new_server.ErrorCode = 200
	}

	switch new_server.Duration {
	case "continuous":
		new_server.TimeoutSeconds = -1
	case "timed":
		if new_server.TimeoutSeconds < 0 {
			new_server.TimeoutSeconds = 5
		}
	case "timeout":
		if new_server.TimeoutSeconds < 0 {
			new_server.TimeoutSeconds = 5
		}
	default:
		new_server.Duration = "continuous"
		new_server.TimeoutSeconds = -1
	}

	return &new_server
}

func NewServerInfo(port int, name string, return_type string, message string, data_length int, error_code int, duration string, timeout int) *Server {
	return NewServer(port, name, return_type, message, data_length, error_code, duration, timeout, logger.Info)

}

// Set Logging Level
func (s *Server) SetLogLevel(level logger.LogLevel) {
	s.logLevel = level
}

// Run runs the server
func (s *Server) Run() {
	// ... (server logic, e.g., handling requests)
	strName := "Server::" + strconv.Itoa(s.Port)
	log := logger.NewLogger(strName, logger.LogLevel(s.logLevel))

	// Create a new ServeMux (router) for each server
	mux := http.NewServeMux()

	// Register handlers for different routes
	mux.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {

		log.Info("request from %s", r.RemoteAddr)
		// Activity occurred, note the time
		s.lock.Lock()
		s.lastActivity = time.Now()
		s.lock.Unlock()

		// Notify about activity
		s.ActivityChan <- true

		switch s.Type {
		case "message":
			handleMessage(w, r, s.Message)
		case "random":
			handleRandom(w, r, s.DataLength)
		case "error":
			handleError(w, r, s.ErrorCode)
		default:
			http.Error(w, "default server response", http.StatusInternalServerError)
			log.Error("unhandled server type: %s from %s", s.Type, r.RemoteAddr)
		}
	})

	addr := fmt.Sprintf(":%d", s.Port)

	// Create a new http.Server
	srv := &http.Server{
		Addr:    addr,
		Handler: mux,
	}

	log.Info("Start server '%s' type: %s, datalen: %d, %s (%d)", s.Name, s.Type, s.DataLength, s.Duration, s.TimeoutSeconds)

	switch s.Duration {
	case "continuous":
		srv.ListenAndServe()
	case "timeout":
		go func() {
			srv.ListenAndServe()
			// You may add additional cleanup logic here if needed
		}()

		// Start a timer to check for inactivity
		timer := time.NewTimer(s.inactivityDur)

		for {
			select {
			case <-s.ActivityChan:
				// Reset the timer on activity
				timer.Reset(s.inactivityDur)

			case <-timer.C:
				// No activity for the specified duration, stop the server
				s.lock.Lock()
				lastActivityDuration := time.Since(s.lastActivity)
				s.lock.Unlock()

				if lastActivityDuration >= s.inactivityDur {
					log.Info("Stop server '%s' after %d seconds of inactivity", s.Name, s.TimeoutSeconds)
					s.Stop()
					return
				}

			case <-s.stopChan:
				// Stop the server if requested
				log.Info("Server '%s' stopping...", s.Name)
				ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
				defer cancel()

				if err := srv.Shutdown(ctx); err != nil {
					log.Info("Error shutting down server '%s' : %v", s.Name, err)
				}
				log.Info("Server '%s' stopped.", s.Name)
				return
			}
		}

	case "timed":
		go func() {
			for {
				select {
				case <-time.After(time.Duration(s.TimeoutSeconds) * time.Second):
					log.Info("Stop timed server '%s' after %d seconds", s.Name, s.TimeoutSeconds)
					// You may add additional cleanup logic here if needed
					return
				}
			}
		}()
	default:
		log.Info("invalid duration: %s", s.Duration)
	}

}

// Stop stops the inactivity server.
func (s *Server) Stop() {
	s.stopChan <- true
}

func handleMessage(w http.ResponseWriter, r *http.Request, message string) {
	fmt.Fprint(w, message)
}

func handleRandom(w http.ResponseWriter, r *http.Request, dataLength int) {
	if dataLength <= 0 {
		http.Error(w, "Invalid data length", http.StatusInternalServerError)
		return
	}

	randomData := generateRandomData(dataLength)
	w.Write(randomData)
}

func handleError(w http.ResponseWriter, r *http.Request, errorCode int) {
	http.Error(w, fmt.Sprintf("%d: %s", errorCode, http.StatusText(errorCode)), errorCode)
}

func generateRandomData(length int) []byte {
	const charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
	randomData := make([]byte, length)

	for i := range randomData {
		randomData[i] = charset[rand.Intn(len(charset))]
	}

	return randomData
}
