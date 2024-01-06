# Use an official Go runtime as a parent image
FROM golang:latest

# Set the working directory to /app
WORKDIR /app

# Copy the Go module files and dependencies
COPY go.mod .

# Download and install Go dependencies
RUN go mod download

# Copy the Go source code into the container
COPY . .

# Build the Go application
RUN go build -o traffik ./main 

# Expose the port the application runs on
EXPOSE 9096

# Copy the data folder
COPY /data/input /app/data/input

# Run the application
CMD ["./traffik"]