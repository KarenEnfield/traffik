package main

import (
	"fmt"
	"log"
	"net"
	"strconv"
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
		client := &client.Client{
			Name:     clientConfig.Name,
			Rate:     clientConfig.Rate,
			Message:  clientConfig.Message,
			MaxSends: clientConfig.MaxSends,
		}

		host, portStr, err := net.SplitHostPort(clientConfig.URL)
		if err != nil {
			// If SplitHostPort fails, use the whole host as the IP address
			client.IPAddress = clientConfig.URL
			// Default port to 80 if not specified
			client.Port = 80
			//continue
		} else {
			client.IPAddress = host
			client.Port, err = strconv.Atoi(portStr)
			if err != nil {
				log.Printf("Error converting port to integer for client %s: %v", client.Name, err)
				continue
			}
		}

		fmt.Printf("%s,%s,%d,%d,%d,%s\n", client.Name, client.IPAddress, client.Port, client.MaxSends, client.Rate, client.Message)

		wg.Add(1)
		go client.Run(&wg)
	}

	// Wait for all clients to finish
	wg.Wait()
}
