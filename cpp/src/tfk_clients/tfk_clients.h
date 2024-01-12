
#include <cstdlib>
#include <uv.h>
#include <string>


// list of clients
class tfk_clients{
    uv_loop_t *loop;

    public:
        tfk_clients(uv_loop_t *dl = nullptr, const char *configData=nullptr, const char *logLevel=nullptr);
        void Run() {uv_run(loop, UV_RUN_DEFAULT);}
        void Stop(){uv_stop(loop);};

    static std::string CreateConfigString(std::string name, std::string url, std::string message, int rate, int maxSends);

};