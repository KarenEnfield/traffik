// main.go
package main

import (
	"log"
	"sync"

	"github.com/KarenEnfield/traffik/go/tfk_clients/client"
)

func main() {
	configFilePath := client.GetConfigFilePath()

	// Read the configuration from the JSON "../data/input/tfk_clients_config.json")
	config, err := client.ReadConfig(configFilePath)
	if err != nil {
		log.Fatal(err)
	}

	// Create and run clients concurrently
	var wg sync.WaitGroup
	for _, clientConfig := range config.Clients {

		wg.Add(1)
		client := client.NewClientInfo(
			clientConfig.Name,
			clientConfig.URL,
			clientConfig.Rate,
			clientConfig.Message,
			clientConfig.MaxSends,
		)

		wg.Add(1)
		go client.Run(&wg)
	}

	// Wait for all clients to finish
	wg.Wait()
}
