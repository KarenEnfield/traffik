# Use an official C++ runtime as a parent image
FROM gcc:latest

# Set the working directory to /app
WORKDIR /app

# Copy the C++ source code into the container
COPY src/ .

# Install libuv inside the Docker image
RUN apt-get update && \
    apt-get install -y libuv1-dev g++ cmake

# Clone nlohmann/json from GitHub
RUN git clone --branch=v3.10.2 --depth=1 https://github.com/nlohmann/json.git

# Build the C++ application
RUN g++ -o traffik \
    -Ijson/include \
    -luv \
    -std=c++17 \
    main.cc \
    tfk_clients/tfk_clients.cc \
    tfk_servers/tfk_servers.cc \
    tfk_logger/tfk_logger.cc 

# Expose the port the application runs on
EXPOSE 9095

# Copy the data folder
COPY ./data/input /data/input

# Run the application
CMD ["./traffik"]