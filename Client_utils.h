#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <list>
#include <string>
#include <unordered_set>
#include <vector>
#include <unordered_map>
#include <memory>

#include "helpers.h"

class Client {
    std::string id_client;
    std::string ip_server;
    int port_server;
    int sockfd, ret;
    struct sockaddr_in serv_address;
    
    bool client_run;
    
    fd_set read_fds;  // Set used for reading using select()
    fd_set tmp_fds;   // Temporary set

public:
    Client(std::string p_id_client, std::string p_ip_server, int p_port_server);
    ~Client();
    void run();

protected:
    void HandleKeyboardInput();
    void HandleServerMessage();
    int ReceiveTCP_Message(int sockfd, TCPMessage *msg);
    void PrintTCP_Message(TCPMessage *msg);
    void SendTCP_Message(int sockfd, TCPMessage *msg);
};