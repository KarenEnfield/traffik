
#include <cstdlib>
#include <uv.h>

// list of clients
class tfk_clients{
    uv_loop_t *loop;

    public:
        tfk_clients(uv_loop_t *dl = nullptr);
        void Run() {uv_run(loop, UV_RUN_DEFAULT);}
        void Stop(){uv_stop(loop);};
};