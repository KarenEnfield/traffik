// tfk_servers/server/server.go
package server

import (
	"fmt"
	"log"
	"math/rand"
	"net/http"
	"time"
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
}

// NewServer creates a new instance of Server
func NewServer(port int, name string, return_type string, message string, data_length int, error_code int, duration string, timeout int) *Server {
	new_server := Server{
		Name:           name,
		Port:           port,
		Type:           return_type,
		Message:        message,
		DataLength:     data_length,
		ErrorCode:      error_code,
		Duration:       duration,
		TimeoutSeconds: timeout,
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
		new_server.TimeoutSeconds = 5
	case "timeout":
		new_server.TimeoutSeconds = 5
	default:
		new_server.Duration = "continuous"
		new_server.TimeoutSeconds = -1
	}

	return &new_server
}

// Run runs the server
func (s *Server) Run() {
	// ... (server logic, e.g., handling requests)

	// Create a new ServeMux (router) for each server
	mux := http.NewServeMux()

	// Register handlers for different routes
	mux.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		log.Printf("[server:%d] received from %s", s.Port, r.RemoteAddr)
		switch s.Type {
		case "message":
			handleMessage(w, r, s.Message)
		case "random":
			handleRandom(w, r, s.DataLength)
		case "error":
			handleError(w, r, s.ErrorCode)
		default:
			http.Error(w, "Invalid server type", http.StatusInternalServerError)
		}
	})

	addr := fmt.Sprintf(":%d", s.Port)
	log.Printf("[%s] port: %d,  type: %s, Started: %s (%d)\n", s.Name, s.Port, s.Type, s.Duration, s.TimeoutSeconds)
	// Create a new http.Server
	srv := &http.Server{
		Addr:    addr,
		Handler: mux,
	}

	switch s.Duration {
	case "continuous":
		srv.ListenAndServe()
	case "timed":
		go func() {
			srv.ListenAndServe()
			time.Sleep(time.Duration(s.TimeoutSeconds) * time.Second)
			log.Printf("[%s] port: %d Stopped after %d seconds.\n", s.Name, s.Port, s.TimeoutSeconds)
			// You may add additional cleanup logic here if needed
		}()
	case "timeout":
		go func() {
			for {
				select {
				case <-time.After(time.Duration(s.TimeoutSeconds) * time.Second):
					log.Printf("[%s] port: %d Stopped after %d seconds of inactivity.\n", s.Name, s.Port, s.TimeoutSeconds)
					// You may add additional cleanup logic here if needed
					return
				}
			}
		}()
	default:
		log.Printf("[%s] port: %d Invalid duration: %s\n", s.Name, s.Port, s.Duration)
	}

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
	http.Error(w, "Error response", errorCode)
}

func generateRandomData(length int) []byte {
	const charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
	randomData := make([]byte, length)

	for i := range randomData {
		randomData[i] = charset[rand.Intn(len(charset))]
	}

	return randomData
}
