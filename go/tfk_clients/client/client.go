// tfk_clients/client/client.go
package client

import (
	"fmt"
	"io"
	"log"
	"net/http"
	"strings"
	"sync"
	"time"
)

// Client represents a client
type Client struct {
	Name      string
	IPAddress string
	Port      int
	Rate      int
	Message   string
	MaxSends  int
}

// Run runs the client
func (c *Client) Run(wg *sync.WaitGroup) {

	defer wg.Done()
	//fmt
	log.Printf("Client %s is running at %s:%d\n", c.Name, c.IPAddress, c.Port)

	interval := time.Second / time.Duration(c.Rate)

	// Simulating client activity
	for i := 1; c.MaxSends == -1 || i <= c.MaxSends; i++ {
		message := fmt.Sprintf("Sending message %d from client %s: %s", i, c.Name, c.Message)
		//fmt
		log.Println(message)

		// Your client logic goes here (send the message, etc.)
		result, err := c.sendMessage()
		if err != nil {
			log.Printf("Error sending message %d from client %s: %v", i, c.Name, err)
		} else {
			fmt.Printf("Received result for message %d from client %s: %.10s ...\n", i, c.Name, result)
		}

		// Simulate some processing time
		time.Sleep(interval)
	}

	fmt.Printf("Client %s completed its task of %d sends.\n", c.Name, c.MaxSends)

}

// sendMessage sends a message to the server and returns the result
func (c *Client) sendMessage() (string, error) {
	// Assuming the server is at the configured URL
	serverURL := fmt.Sprintf("http://%s:%d", c.IPAddress, c.Port)

	// Prepare the request
	req, err := http.NewRequest("POST", serverURL, strings.NewReader(c.Message))
	if err != nil {
		return "", err
	}

	// Set request headers or add payload if needed
	req.Header.Set("Content-Type", "application/json")

	// req.Body = ...

	// Send the request
	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()

	// Read the response
	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return "", err
	}

	return string(body), nil

}
