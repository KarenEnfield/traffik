

#include "tfk_clients/tfk_clients.h"
#include "tfk_servers/tfk_servers.h"

#include <uv.h>

int main(){

    tfk_clients c;
    tfk_servers s;
    // run something after the clients and servers are set up
    // uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    c.run();
    s.run();
    
}
