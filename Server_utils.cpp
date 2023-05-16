#include "Server_utils.h"
#include <iostream>

Server::Server(int port) 
{
    // Disable buffering
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;
    // Clear file descriptor sets
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    // Open TCP listening and UDP sockets
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(tcp_fd < 0, "socket tcp");
    DIE(udp_fd < 0, "socket udp");

    // Bind server address to TCP and UDP descriptors
    ret = bind(tcp_fd, (struct sockaddr *)&server_address, sizeof(struct sockaddr));
    DIE(ret < 0, "bind tcp");
    ret = bind(udp_fd, (struct sockaddr *)&server_address, sizeof(struct sockaddr));
    DIE(ret < 0, "bind udp");

    // Call listen for TCP descriptor
    ret = listen(tcp_fd, 10);
    DIE(ret < 0, "listen");

    client_address_len = sizeof(client_address);

    // Add TCP and UDP sockets to set
    FD_SET(tcp_fd, &read_fds);
    FD_SET(udp_fd, &read_fds);
    fdmax = std::max(tcp_fd, udp_fd);

    // Add keyboard input to set
    FD_SET(STDIN_FILENO, &read_fds);
}

Server::~Server() 
{
    close(tcp_fd);
    close(udp_fd);

    TCPMessage msg;
    msg.msg_type = 3;
    for (auto client : client_list) {
        SendTCP_Message(client.sockfd, &msg);
    }
}

void Server::run() 
{
    server_run = true;
    while (1) 
    {
        tmp_fds = read_fds;  // Save reading set (select will modify it)

        ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "select");
        for (int fd = 0; fd <= fdmax; fd++) 
        {
            if (FD_ISSET(fd, &tmp_fds)) 
            {
                if (fd == tcp_fd) {
                    HandleNewTCP_Client();
                } else if (fd == udp_fd) {
                    HandleUDP_Message(fd);
                } else if (fd == STDIN_FILENO){
                    HandleInputFromKeyboard();
                } else {
                    HandleTCP_Message(fd);
                }
            }
        }

        if(!server_run)
        {
            break;
        }
    }
}

void Server::HandleNewTCP_Client()
{
    // New TCP client attempting to connect
    client_socket = accept(tcp_fd, (struct sockaddr *)&client_address, &client_address_len);
    DIE(client_socket < 0, "accept");

    // Diasble Nagle's algorithm
    flag = 1;
    ret = setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
    DIE(ret < 0, "setsockopt");

    // Add new socket to reading set
    FD_SET(client_socket, &read_fds);
    if (client_socket > fdmax) 
    {
        fdmax = client_socket;
    }
}

void Server::HandleUDP_Message(int fd)
{
    
    // Received message from UDP client
    char buffer[1551];
    bzero(buffer, 1551);
    TCPMessage msg;
    ReceiveUDP_Message(fd, buffer, (struct sockaddr *)&client_address);

    // Process UDP message
    ret = CreateTCP_Message(buffer, &msg, &client_address);
    if (ret == -1) 
    {
        return ; // Corrupt UDP message
    }

    // Send message to all subscribed TCP clients
    std::unordered_set<TCP_Client, TCP_Client::Hash> pending_list;
    for (auto& client : client_list) 
    {
        if(client.all_topics.find(msg.topic) == client.all_topics.end())
        {
            continue;
        }

        if(client.active)
        {
            SendTCP_Message(client.sockfd, &msg);
        }
        else 
        {
            if(client.sf_topics.find(msg.topic) == client.sf_topics.end())
            {
                continue;
            }

            // Allocate memory on heap for pending message
            TCPMessage *pending_msg = new TCPMessage(msg);

            client.pending_msgs.push_back(pending_msg);
        }
    }
    
}

void Server::HandleInputFromKeyboard()
{
    // Received input from keyboard
    char buffer[100];
    bzero(buffer, 100);
    fgets(buffer, 99, stdin);

    if (strncmp(buffer, "exit", strlen("exit")) == 0)
    {
        server_run = false;
    }
}

void Server::HandleTCP_Message(int fd)
{
    // Received message from TCP client
    TCPMessage msg;

    int bytes_received = ReceiveTCP_Message(fd, &msg);
    
    // Check closed connection
    if (bytes_received == 0) 
    {
        // Mark client as inactive
        TCP_Client temp_client_for_search;
        temp_client_for_search.client_id = socket_map[fd];

        auto clientIter = client_list.find(temp_client_for_search);
        if(clientIter != client_list.end()) 
        {
            clientIter->active = false;
        }

        // Remove socket from reading list, socket map and close socket
        FD_CLR(fd, &read_fds);
        socket_map.erase(fd);
        close(fd);

        // Print details
        printf("Client %s disconnected.\n", (clientIter->client_id).c_str());
        return;
    }
    
    // Process TCP message
    std::string id = msg.client_id;
    if (msg.msg_type == 2) 
    {
        // Check if ID was previously registered
        TCP_Client temp_client_for_search;
        temp_client_for_search.client_id = id;

        auto clientIter = client_list.find(temp_client_for_search);
        if(clientIter != client_list.end())
        {
            // Check if client with same ID is connected
            if (clientIter->active) 
            {
                // Send close request as TCP message
                TCPMessage reply_msg;
                reply_msg.msg_type = 3;
                SendTCP_Message(fd, &reply_msg);

                // Print details
                printf("Client %s already connected.\n", id.c_str());

                // Remove socket from reading list
                FD_CLR(fd, &read_fds);
                return;
            } 
            else 
            {
                // Client was previously connected

                // Send pending messages
                for (auto &msg : clientIter->pending_msgs) 
                {
                    SendTCP_Message(fd, msg);
                }

                // Clear pending message list
                clientIter->pending_msgs.clear();
            }

            // Update client 
            clientIter->active = true;
            clientIter->sockfd = fd;
        } 
        else 
        {
            // Add new client to the client list
            TCP_Client newClient;
            newClient.client_id = id;
            newClient.active = true;
            newClient.sockfd = fd;
            client_list.insert(newClient);
        }

        // Add to socket map
        socket_map[fd] = id;

        // Print details
        ret = getpeername(fd, (struct sockaddr *)&client_address, &client_address_len);
        DIE(ret < 0, "getpeername");

        printf("New client %s connected from %s:%hu.\n",
                msg.client_id, inet_ntoa(client_address.sin_addr), client_address.sin_port);
    } 
    else if (msg.msg_type == 0) 
    {
        TCP_Client temp_client_for_search;
        temp_client_for_search.client_id = id;

        auto clientIter = client_list.find(temp_client_for_search);

        clientIter->all_topics.insert(msg.topic);

        if (msg.sf) {
            clientIter->sf_topics.insert(msg.topic);
        }
    } 
    else if (msg.msg_type == 1) 
    {
        TCP_Client temp_client_for_search;
        temp_client_for_search.client_id = id;

        auto clientIter = client_list.find(temp_client_for_search);

        if (msg.sf) {
            clientIter->sf_topics.erase(msg.topic);
        }
    } 
    else 
    {
        // Invalid message
        return; 
    }
}

void Server::ReceiveUDP_Message(int sockfd, char *buffer, struct sockaddr *src_addr)
{
    unsigned int nr_of_bytes_read = 0;

    socklen_t len;
    nr_of_bytes_read = recvfrom(sockfd, buffer, 1551, MSG_DONTWAIT, src_addr, &len);
    DIE(nr_of_bytes_read < 0, "recvfrom");
}

int Server::CreateTCP_Message(char *buffer, TCPMessage *msg, struct sockaddr_in *src_addr) {
    // Fill struct fields
    msg->msg_type = 4;
    msg->src_addr = src_addr->sin_addr.s_addr;
    msg->port = src_addr->sin_port;

    // Parse topic
    sscanf(buffer, "%s", msg->topic);

    // Parse data_type
    msg->data_type = buffer[50];

    
    if (msg->data_type == 0) 
    { 
        // Type INT
        char sign = buffer[51];
        if (sign != 0 && sign != 1) 
        {
            return -1;
        }
        msg->content.i.sign = sign;

        uint32_t val;
        memcpy(&val, buffer + 52, sizeof(uint32_t));
        val = ntohl(val);

        msg->content.i.val = val;
    }
    else if (msg->data_type == 1) 
    { 
        // Type SHORT_REAL
        uint16_t val;
        memcpy(&val, buffer + 51, sizeof(uint16_t));
        val = ntohs(val);

        msg->content.sr.val = val;
    }
    else if (msg->data_type == 2) 
    { 
        // Type FLOAT
        char sign = buffer[51];
        if (sign != 0 && sign != 1) 
        {
            return -1;
        }
        msg->content.f.sign = sign;

        uint32_t val;
        memcpy(&val, buffer + 52, sizeof(uint32_t));
        val = ntohl(val);

        msg->content.f.val = val;

        msg->content.f.pos = buffer[56];
    } 
    else if (msg->data_type == 3) 
    { 
        // Type STRING
        sscanf(buffer + 51, "%[^\n]s", msg->content.str.txt);
    } 
    else 
    {
        // Corrupt UDP message
        return -1;  
    }

    return 0;
}

void Server::SendTCP_Message(int sockfd, TCPMessage *msg) {
    unsigned int bytes_sent = 0;
    unsigned int bytes_remaining = sizeof(TCPMessage);
    char buffer[sizeof(TCPMessage)];
    unsigned int n;

    // Copy message into buffer
    memcpy(buffer, msg, sizeof(TCPMessage));

    while (bytes_sent < sizeof(TCPMessage)) {
        n = send(sockfd, buffer + bytes_sent, bytes_remaining, 0);
        DIE(n < 0, "send");

        bytes_sent += n;
        bytes_remaining -= n;
    }
}

int Server::ReceiveTCP_Message(int sockfd, TCPMessage *msg) {
    unsigned int bytes_received = 0;
    unsigned int bytes_remaining = sizeof(TCPMessage);
    char buffer[sizeof(TCPMessage)];
    unsigned int n;

    while (bytes_received < sizeof(TCPMessage)) {
        n = recv(sockfd, buffer + bytes_received, bytes_remaining, 0);
        DIE(n < 0, "recv");

        if (n == 0) {
            return 0;  // Connection closed
        }

        bytes_received += n;
        bytes_remaining -= n;
    }

    // Copy buffer to struct (ensured packing)
    memcpy(msg, buffer, sizeof(TCPMessage));

    return bytes_received;
}