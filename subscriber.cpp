#include "Client_utils.h"
#include <iostream>

int main(int argc, char *argv[]) {
   std::string id_client = argv[1];
   std::string ip_server = argv[2];
   int port_server = atoi(argv[3]);

   Client *client = new Client(id_client, ip_server, port_server);
   client->run();

   delete client;
   return 0;
}