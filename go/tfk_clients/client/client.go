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

	log.Printf("[%s] sending to %s:%d\n", c.Name, c.IPAddress, c.Port)

	interval := time.Second / time.Duration(c.Rate)

	// Simulating client activity
	for i := 1; c.MaxSends == -1 || i <= c.MaxSends; i++ {
		log.Printf("[%s] %d sends to  %s:%d", c.Name, i, c.IPAddress, c.Port)

		// Your client logic goes here (send the message, etc.)
		result, err := c.sendMessage()
		if err != nil {
			log.Printf("[%s] error sending message %d %v", c.Name, i, err)
		} else {
			log.Printf("[%s] received response %d from %s:%d %.15s ...\n", c.Name, i, c.IPAddress, c.Port, result)
		}

		// Simulate some processing time
		time.Sleep(interval)
	}

	log.Printf("[%s] completed maximum sends (%d) to %s:%d\n", c.Name, c.MaxSends, c.IPAddress, c.Port)

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
