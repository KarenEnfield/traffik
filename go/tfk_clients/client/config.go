// tfk_clients/client/config.go
package client

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

// ClientConfig represents client configuration
type ClientConfig struct {
	Name     string `json:"name"`
	URL      string `json:"url"`
	Rate     int    `json:"rate"`
	Message  string `json:"message"`
	MaxSends int    `json:"max_sends"`
}

// ClientsConfig represents top-level client configuration
type ClientsConfig struct {
	Clients []ClientConfig `json:"clients"`
}

// ReadConfig reads the configuration from the given JSON file
func ReadConfig(filename string) (*ClientsConfig, error) {
	data, err := os.ReadFile(filename)
	if err != nil {
		return nil, err
	}
	var config ClientsConfig

	if err := json.Unmarshal(data, &config); err != nil {
		return nil, err
	}

	return &config, nil

}

// GetConfigFilePath returns the JSON file path from the environment variable or a default value
func GetConfigFilePath() string {
	// Check if the environment variable is set
	envVar := os.Getenv("TFK_CONFIG_FILE_PATH")
	if envVar != "" {
		return envVar
	}

	defaultFileRelPath := "data/input/tfk_clients_config.json"
	// If not set, use a default value
	projectRoot, err := os.Getwd()
	if err != nil {
		fmt.Println("Error getting working directory:", err)
		os.Exit(1)
	}

	// Find the first occurrence of "traffik" in the current directory path
	index := strings.Index(projectRoot, "traffik")

	// If "traffik" is found, truncate the string to keep only the part before it
	if index != -1 {
		projectRoot = projectRoot[:index+len("traffik")]
		projectRoot += "/go"
	}

	// Construct the absolute path to the default input file
	defaultFilePath := filepath.Join(projectRoot, defaultFileRelPath)

	return defaultFilePath

}
