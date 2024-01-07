#include <cstdlib>
#include <uv.h>
class tfk_servers{
    uv_loop_t *loop;

    public:
        tfk_servers(uv_loop_t *dl=nullptr);
        void run(){uv_run(loop, UV_RUN_DEFAULT);}
        void stop(){uv_stop(loop);};
};