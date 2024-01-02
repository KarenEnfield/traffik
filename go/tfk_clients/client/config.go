// tfk_clients/client/config.go
package client

import (
	"encoding/json"
	"os"
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
	envVar := os.Getenv("CONFIG_FILE_PATH")
	if envVar != "" {
		return envVar
	}

	// If not set, use a default value
	return "data/input/tfk_clients_config.json"

}
