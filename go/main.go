// main.go
package main

import (
	"log"
	"sync"

	"github.com/KarenEnfield/traffik/go/tfk_clients/client"
	"github.com/KarenEnfield/traffik/go/tfk_servers/server"
)

func main() {
	// Start servers
	server1 := server.NewServer(8081, "Server1", "message", "Hello_from_Server1", 0, 0, "continuous", 10)
	server2 := server.NewServer(8082, "Server2", "random", "", 100, 0, "continuous", 10)
	go server1.Run()
	go server2.Run()

	// Start clients
	client1 := client.Client{
		Name:      "Client1",
		IPAddress: "127.0.0.1",
		Port:      8081,
		Rate:      1,
		Message:   "Hello_World!",
		MaxSends:  -1,
	}
	client2 := client.Client{
		Name:      "Client2",
		IPAddress: "127.0.0.1",
		Port:      8082,
		Rate:      1,
		Message:   "Hello_World!",
		MaxSends:  30,
	}

	var wg1 sync.WaitGroup
	wg1.Add(1)
	go func() {
		client1.Run(&wg1)
	}()

	wg1.Add(1)
	go func() {
		client2.Run(&wg1)
	}()

	// Wait for servers and clients to finish
	wg1.Wait()

	log.Println("All servers and clients have finished.")
}

func init() {
	// Seed the random number generator to ensure different results in each run
	// (this is useful if you are using random data in your clients or servers)

}
