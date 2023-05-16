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

// Class used for storing informations about the TCP clients
class TCP_Client {
public:
    TCP_Client() = default;
    TCP_Client(const TCP_Client& other) : sockfd(other.sockfd), active(other.active), sf_topics(other.sf_topics), 
    all_topics(other.all_topics), client_id(other.client_id)
    {
        // Make a deep copy of the pending messages
        for (auto msg : other.pending_msgs) {
            pending_msgs.push_back(new TCPMessage(*msg));
        }
    }
    ~TCP_Client()
    {
        // Delete the pending messages
        for (auto msg : pending_msgs) {
            delete msg;
        }
    }

    struct Hash {
        size_t operator()(const TCP_Client& client) const {
            return std::hash<std::string>()(client.client_id);
        }
    };

    bool operator==(const TCP_Client& other) const {
    return client_id == other.client_id;
    }
    
public:
    mutable int sockfd;
    mutable bool active;
    mutable std::unordered_set<std::string> sf_topics;  // store and forward subscriptions
    mutable std::unordered_set<std::string> all_topics;  // all subscriptions
    mutable std::vector<TCPMessage *> pending_msgs;     // list of pending messages (if sf = 1)

    std::string client_id;
};

class Server {

    int tcp_fd;
    int udp_fd;
    int client_socket;
    int portno;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int ret, flag;
    socklen_t client_address_len;
    int fdmax;

    // Set used for reading using select()
    fd_set read_fds;
    // Temporary set 
    fd_set tmp_fds;   

    int server_run;

    // Map for storing file descriptors and IDs
    std::unordered_map<int, std::string> socket_map;

    // List for storing clients;
    std::unordered_set<TCP_Client, TCP_Client::Hash> client_list;

public:
    Server(int port);
    ~Server();
    void run();

protected:
    void HandleNewTCP_Client();
    void HandleUDP_Message(int fd);
    void HandleInputFromKeyboard();
    void HandleTCP_Message(int fd);

    void ReceiveUDP_Message(int sockfd, char *buffer, struct sockaddr *src_addr);
    int ReceiveTCP_Message(int sockfd, TCPMessage *msg);
    int CreateTCP_Message(char *buffer, TCPMessage *msg, struct sockaddr_in *src_addr);
    void SendTCP_Message(int sockfd, TCPMessage *msg);

private:
    void reverse_char_array(char *res, char *src, int n);
};


