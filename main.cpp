#include <unistd.h>
#include "server/webserver.h"

int main() 
{
    WebServer server;
    server.loadConfigFile();
    server.start();
    return 0;
} 