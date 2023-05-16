
#include "Server_utils.h"
#include <iostream>
#include <stdio.h>

int main(int argc, char *argv[]) {
   Server server = Server(atoi(argv[1]));
   server.run();
   return 0;
}