// tfk_servers/server/config.go
package server

import (
	"encoding/json"
	"os"
	"strconv"
)

// ServerConfig represents server configuration
type ServerConfig struct {
	Name           string `json:"name"`
	Port           int    `json:"port"`
	Type           string `json:"type"`
	Message        string `json:"message"`
	DataLength     int    `json:"data_length"`
	ErrorCode      int    `json:"error_code"`
	Duration       string `json:"duration"`
	TimeoutSeconds int    `json:"timeout_seconds"`
}

// ServersConfig represents top-level server configuration
type ServersConfig struct {
	Servers []ServerConfig `json:"servers"`
}

// NewDefaultServerConfig creates a new instance of ServerConfig with default values
func NewDefaultServerConfig(port int) *ServerConfig {
	return &ServerConfig{
		Name:           "localhost:" + strconv.Itoa(port),
		Port:           port,
		Type:           "message", // Set your default values here
		Message:        "localhost:" + strconv.Itoa(port),
		DataLength:     50,
		ErrorCode:      200,
		Duration:       "continuous",
		TimeoutSeconds: -1,
	}
}

// ReadConfig reads the configuration from the given JSON file
func ReadConfig(filename string) (*ServersConfig, error) {
	// ... (same as before)
	data, err := os.ReadFile(filename)
	if err != nil {
		return nil, err
	}
	var config ServersConfig

	if err := json.Unmarshal(data, &config); err != nil {
		return nil, err
	}

	return &config, nil
}

// GetConfigFilePath returns the JSON file path from the environment variable or a default value
func GetConfigFilePath() string {
	// Check if the environment variable is set
	envVar := os.Getenv("CONFIG_FILE_PATH")
	if envVar != "" {
		return envVar
	}

	// If not set, use a default value
	return "data/input/tfk_servers_config.json"

}
