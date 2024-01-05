// main.go
package main

import (
	"sync"

	"github.com/KarenEnfield/traffik/go/tfk_clients/client"
	"github.com/KarenEnfield/traffik/go/tfk_logger/logger"
	"github.com/KarenEnfield/traffik/go/tfk_servers/server"
)

func main() {
	logLevel := logger.Info

	log := logger.NewLogger("traffik", logLevel)

	// Create servers
	configFilePath := server.GetConfigFilePath()

	// Read the configuration from the JSON "../data/input/tfk_clients_config.json")
	serversConfig, err := server.ReadConfig(configFilePath)
	if err != nil {
		log.Critical(err.Error())
		return
	}

	// Create and start server modules
	for _, serverConfig := range serversConfig.Servers {

		serverModule := server.NewServer(
			serverConfig.Port,
			serverConfig.Name,
			serverConfig.Type,
			serverConfig.Message,
			serverConfig.DataLength,
			serverConfig.ErrorCode,
			serverConfig.Duration,
			serverConfig.TimeoutSeconds,
			logLevel,
		)

		go serverModule.Run()
	}

	// Create clients
	configFilePath = client.GetConfigFilePath()

	// Read the configuration from the JSON "../data/input/tfk_clients_config.json")
	clientsConfig, err := client.ReadConfig(configFilePath)
	if err != nil {
		log.Critical(err.Error())
		return
	}

	// Create and run clients concurrently
	var wg sync.WaitGroup
	for _, clientConfig := range clientsConfig.Clients {

		wg.Add(1)
		client := client.NewClient(
			clientConfig.Name,
			clientConfig.URL,
			clientConfig.Rate,
			clientConfig.Message,
			clientConfig.MaxSends,
			logLevel,
		)

		log.Info("Start client [%s] to %s:%d,%d,%d,%s", client.Name, client.IPAddress, client.Port, client.MaxSends, client.Rate, client.Message)

		wg.Add(1)
		go client.Run(&wg)
	}

	// Wait for servers and clients to finish
	wg.Wait()

	// Keep the main program running
	select {}

	log.Info("All servers and clients have finished.")
}

func init() {
	// Seed the random number generator to ensure different results in each run
	// (this is useful if you are using random data in your clients or servers)

}
