// main.go
package main

import (
	"fmt"
	"sync"

	"github.com/yourusername/traffik/clients/client"
	"github.com/yourusername/traffik/servers/server"
)

func main() {
	// Start servers
	server1 := server.NewServer(8081, "Server1", "message", "Hello_from_Server1", 0, 0, "timeout", 10)
	server2 := server.NewServer(8082, "Server2", "random", "", 100, 0, "timeout", 10)

	var wg sync.WaitGroup
	wg.Add(2)

	go func() {
		defer wg.Done()
		server1.Run()
	}()

	go func() {
		defer wg.Done()
		server2.Run()
	}()

	// Start clients
	client1 := client.NewClient("Client1", "http://localhost:8081", 1, "Hello_World!", -1)
	client2 := client.NewClient("Client2", "http://localhost:8082", 1, "Hello_World!", 30)

	go func() {
		client1.Run()
	}()

	go func() {
		client2.Run()
	}()

	// Wait for servers and clients to finish
	wg.Wait()

	fmt.Println("All servers and clients have finished.")
}

func init() {
	// Seed the random number generator to ensure different results in each run
	// (this is useful if you are using random data in your clients or servers)

}
