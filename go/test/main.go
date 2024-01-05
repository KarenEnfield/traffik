// main.go
package main

import (
	"sync"

	"github.com/KarenEnfield/traffik/go/tfk_clients/client"
	"github.com/KarenEnfield/traffik/go/tfk_logger/logger"
	"github.com/KarenEnfield/traffik/go/tfk_servers/server"
)

func main() {
	// Start servers
	level := logger.Info

	log := logger.NewLogger("traffik_test", level)

	server1 := server.NewServer(8081, "Server1", "message", "Hello_from_Server1", 0, 0, "timeout", 5, level)
	server2 := server.NewServer(8082, "Server2", "random", "", 100, 0, "timeout", 5, level)

	go server1.Run()
	go server2.Run()

	// Start clients
	client1 := client.NewClient(
		"Client1",
		"127.0.0.1:8081",
		1,
		"Hello_World!",
		5,
		level,
	)

	client2 := client.NewClient(
		"Client2",
		"127.0.0.1:8082",
		1,
		"Hello_World!",
		5,
		level,
	)

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

	// Keep the main program running
	select {}

	// log.Info("All servers and clients have finished.")
}

func init() {
	// Seed the random number generator to ensure different results in each run
	// (this is useful if you are using random data in your clients or servers)

}
