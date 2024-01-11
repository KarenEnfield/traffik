// tfk_clients/client/client.go
package client

import (
	"fmt"
	"io"
	"net"
	"net/http"
	"net/url"
	"strconv"
	"strings"
	"sync"
	"time"

	"github.com/KarenEnfield/traffik/go/tfk_logger/logger"
)

// Client represents a client
type Client struct {
	Name      string
	IPAddress string
	Port      int
	Rate      int
	Message   string
	MaxSends  int
	logLevel  logger.LogLevel
}

// NewClient creates a new instance of Client
func NewClient(name string, url string, rate int, message string, max_sends int, level logger.LogLevel) *Client {
	new_client := Client{
		Name:      name,
		IPAddress: url,
		Port:      80,
		Rate:      rate,
		Message:   message,
		MaxSends:  max_sends,
		logLevel:  level,
	}

	host, portStr, err := net.SplitHostPort(url)
	if err == nil {
		new_client.IPAddress = host
		new_client.Port, err = strconv.Atoi(portStr)
		if err != nil {
			// Error converting port to integer for client %s: %v", name, err)
			if host == "localhost" {
				new_client.Port = 8080
			} else if host == "127.0.0.1" {
				new_client.Port = 8080
			} else {
				new_client.Port = 80
			}
		}
	}

	return &new_client
}

// NewClient creates a new instance of Client
func NewClientInfo(name string, url string, rate int, message string, max_sends int) *Client {
	return NewClient(name, url, rate, message, max_sends, logger.Info)
}

// Set Logging Level
func (c *Client) SetLogLevel(level logger.LogLevel) {
	c.logLevel = level
}

// Set Logging Level
func (c *Client) SetLogLevelTrace() {
	c.logLevel = logger.Trace
}

// Set Logging Level
func (c *Client) SetLogLevelDebug() {
	c.logLevel = logger.Debug
}

// Set Logging Level
func (c *Client) SetLogLevelInfo() {
	c.logLevel = logger.Info
}

// Set Logging Level
func (c *Client) SetLogLevelWarning() {
	c.logLevel = logger.Warning
}

// Set Logging Level
func (c *Client) SetLogLevelError() {
	c.logLevel = logger.Error
}

// Set Logging Level
func (c *Client) SetLogLevelCritical() {
	c.logLevel = logger.Critical
}

// Run runs the client
func (c *Client) Run(wg *sync.WaitGroup) {

	defer wg.Done()
	log := logger.NewLogger(c.Name, c.logLevel)
	if c.MaxSends >= 0 {
		log.Info("Client begin sending %d messages to %s:%d", c.MaxSends, c.IPAddress, c.Port)
	} else {
		log.Info("Client begin sending continuous messages to %s:%d", c.IPAddress, c.Port)

	}

	interval := time.Second / time.Duration(c.Rate)

	// Simulating client activity
	for i := 1; c.MaxSends == -1 || i <= c.MaxSends; i++ {
		log.Info("client send (%d)", i)

		// Your client logic goes here (send the message, etc.)
		result, err := c.sendMessage()
		if err != nil {
			if urlErr, ok := err.(*url.Error); ok {
				// Check if the embedded error is of type *net.OpError
				if opErr, ok := urlErr.Err.(*net.OpError); ok {
					// Extract the HTTP status code and message from the embedded error
					log.Info("client neterr send #%d to %s:%d %s (%s)", i, c.IPAddress, c.Port, opErr.Err, opErr.Op)

				} else {
					// If the embedded error is not of type *net.OpError, print the original error
					log.Info("client urlerr send #%d to %s:%d %s", i, c.IPAddress, c.Port, err)
				}
			} else {
				// If the error is not of type *url.Error, print the original error
				log.Info("client tcperr send #%d to %s:%d %s", i, c.IPAddress, c.Port, err)
			}

		} else {
			log.Debug("client resp #%d from %s:%d %.15s ...", i, c.IPAddress, c.Port, result)
		}

		// Simulate some processing time
		time.Sleep(interval)
	}

	log.Info("Client stopped after reaching maximum sends (%d)", c.MaxSends)

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
