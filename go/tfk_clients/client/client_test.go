package client

import (
	"fmt"
	"net"
	"net/http"
	"net/http/httptest"
	"sync"
	"testing"
)

func TestClientRun(t *testing.T) {
	// Set up a mock server for testing
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		// Simulate server response
		fmt.Fprint(w, "Server response")
	}))
	defer server.Close()

	// Create a test client
	testClient := &Client{
		Name:      "TestClient",
		IPAddress: "localhost",
		Port:      server.Listener.Addr().(*net.TCPAddr).Port,
		Rate:      1,
		Message:   "TestMessage",
		MaxSends:  2,
	}

	var wg sync.WaitGroup
	wg.Add(1)

	// Run the client
	go testClient.Run(&wg)

	// Wait for the client to finish
	wg.Wait()

	// Assert any expected results here based on the behavior of your client
}

func TestSendMessage(t *testing.T) {
	// Set up a mock server for testing
	server := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		// Simulate server response
		fmt.Fprint(w, "Server response")
	}))
	defer server.Close()

	// Create a test client
	testClient := &Client{
		Name:      "TestClient",
		IPAddress: "localhost",
		Port:      server.Listener.Addr().(*net.TCPAddr).Port,
		Rate:      1,
		Message:   "TestMessage",
		MaxSends:  2,
	}

	// Call the sendMessage function
	result, err := testClient.sendMessage()

	// Assert any expected results here based on the behavior of your sendMessage function
	if err != nil {
		t.Fatalf("Expected no error, but got: %v", err)
	}

	expectedResult := "Server response"
	if result != expectedResult {
		t.Fatalf("Expected result '%s', but got '%s'", expectedResult, result)
	}
}
