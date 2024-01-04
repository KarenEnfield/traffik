// servers/server/server_test.go
package server

import (
	"net/http"
	"net/http/httptest"
	"testing"
)

func TestHandleMessage(t *testing.T) {
	// Create a request
	req, err := http.NewRequest("GET", "/", nil)
	if err != nil {
		t.Fatal(err)
	}

	// Create a ResponseRecorder
	rr := httptest.NewRecorder()

	// Call the handler function
	handleMessage(rr, req, "Hello_World!")

	// Check the response status code
	if status := rr.Code; status != http.StatusOK {
		t.Errorf("handler returned wrong status code: got %v want %v", status, http.StatusOK)
	}

	// Check the response body
	expected := "Hello_World!"
	if rr.Body.String() != expected {
		t.Errorf("handler returned unexpected body: got %v want %v", rr.Body.String(), expected)
	}
}

func TestHandleRandom(t *testing.T) {
	// Create a request
	req, err := http.NewRequest("GET", "/", nil)
	if err != nil {
		t.Fatal(err)
	}

	// Create a ResponseRecorder
	rr := httptest.NewRecorder()

	// Call the handler function
	handleRandom(rr, req, 10)

	// Check the response status code
	if status := rr.Code; status != http.StatusOK {
		t.Errorf("handler returned wrong status code: got %v want %v", status, http.StatusOK)
	}

	// Check the response body length
	expectedLength := 10
	if len(rr.Body.Bytes()) != expectedLength {
		t.Errorf("handler returned unexpected body length: got %v want %v", len(rr.Body.Bytes()), expectedLength)
	}
}

func TestHandleError(t *testing.T) {
	// Create a request
	req, err := http.NewRequest("GET", "/", nil)
	if err != nil {
		t.Fatal(err)
	}

	// Create a ResponseRecorder
	rr := httptest.NewRecorder()

	// Call the handler function
	handleError(rr, req, 404)

	// Check the response status code
	if status := rr.Code; status != 404 {
		t.Errorf("handler returned wrong status code: got %v want %v", status, 404)
	}
}
