// main.go
package main

import (
	"log"

	"github.com/KarenEnfield/traffik/go/tfk_logger/logger"
	"github.com/KarenEnfield/traffik/go/tfk_servers/server"
)

func main() {

	level := logger.Info

	configFilePath := server.GetConfigFilePath()

	// Read the configuration from the JSON "../data/input/tfk_servers_config.json")
	config, err := server.ReadConfig(configFilePath)
	if err != nil {
		log.Fatal(err)
	}

	// Create and start server modules
	for _, serverConfig := range config.Servers {

		serverModule := server.NewServer(
			serverConfig.Port,
			serverConfig.Name,
			serverConfig.Type,
			serverConfig.Message,
			serverConfig.DataLength,
			serverConfig.ErrorCode,
			serverConfig.Duration,
			serverConfig.TimeoutSeconds,
			level,
		)

		go serverModule.Run()
	}

	// Keep the main program running
	select {}
}
