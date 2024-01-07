

#include "tfk_clients/tfk_clients.h"
#include "tfk_servers/tfk_servers.h"

#include <uv.h>
#include <iostream>
int main(){

try{
    uv_loop_t *lp = uv_default_loop();
    tfk_clients c(lp);
    tfk_servers s(lp);
    
    //c.run();
    //s.run();
    uv_run(lp, UV_RUN_DEFAULT);
}
catch(const std::exception& e)
{
    std::cerr<< "caught in main" <<e.what()<<std::endl;
}
    
}
