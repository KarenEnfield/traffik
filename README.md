# traffik
Network traffic generator

Traffik allows the creation of clients that send requests to servers, and the creation of servers that respond to clients.
Clients and Servers are created from reading a json file.
Clients may run a number of times, or indefinitely.  
Servers may respond with an error code or an html page of a message or random bytes of a particular length
Servers may remain running indefinitely, or until an inactivity timeout has been reached

Dependencies
C++ compilation:
  libuv must be installed for compilation: brew install libuv
  optional: spdlog can be installed for optimal logging: brew install spdlog
  If no spdlog is prefered, in tfk_logging.h comment out line //#ifdef USE_SPDLOG and recompile

