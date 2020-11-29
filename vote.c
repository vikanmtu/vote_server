
#include "server.h"

int main(int argc, char* argv[])
{
        short i;

        i=srv_init(argc, argv); //initialize server
        tor_run();

        if(i) srv_process();  //server's infinite task

        return 0;
}