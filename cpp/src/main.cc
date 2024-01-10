

#include "tfk_clients/tfk_clients.h"
#include "tfk_servers/tfk_servers.h"

#include <uv.h>
#include <iostream>

int main(){

    
    #ifdef SPECIFY_UV_LOOP
    // specifying a uv loop is optional, 
    uv_loop_t *lp = uv_default_loop();

    tfk_clients c(lp);
    tfk_servers s(lp);
    uv_run(lp, UV_RUN_DEFAULT); // loop run can be explicity called

    #else
    // Uses the default uv loop, if not specified
    tfk_clients c;
    tfk_servers s;
    
    c.Run();
    s.Run();

    #endif
    
}
