#include <cstdlib>
#include <uv.h>
#include <string>

class tfk_servers{
    uv_loop_t *loop;

    public:
        tfk_servers(uv_loop_t *dl=nullptr, const char *configString=nullptr, const char *logLevel=nullptr);
        void Run(){uv_run(loop, UV_RUN_DEFAULT);}
        void Stop(){uv_stop(loop);};
    
    static std::string CreateConfigString(std::string name, int port, std::string serverType, std::string message, int dataLength, int errorCode, std::string duration, int timeout);

};