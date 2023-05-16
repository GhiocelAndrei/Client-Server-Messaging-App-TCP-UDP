#include "Client_utils.h"
#include <iostream>
#include <math.h>

Client::Client(std::string p_id_client, std::string p_ip_server, int p_port_server)
{
    // portno = port_server    
    id_client = p_id_client;
    ip_server = p_ip_server;
    port_server = p_port_server;

    // Clear file descriptor sets
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    // Set struct fields for server
    bzero(&serv_address, sizeof(serv_address));
    serv_address.sin_family = AF_INET;
    serv_address.sin_port = htons(port_server);

    ret = inet_aton(ip_server.c_str(), &serv_address.sin_addr);
    DIE(ret == 0, "inet_aton");

    // Open socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");

    // Connect to socket
    ret = connect(sockfd, (struct sockaddr *)&serv_address, sizeof(serv_address));
    DIE(ret < 0, "connect");

    // Add stdin and server socket to set
    FD_SET(sockfd, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
}

Client::~Client()
{
    close(sockfd);
}

void Client::run()
{
    client_run = true;

    // Disable buffering
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    // Send registration message
    TCPMessage msg;
    msg.msg_type = 2; // Register client message type
    strcpy(msg.client_id, id_client.c_str());
    SendTCP_Message(sockfd, &msg);
    while (1) {
        tmp_fds = read_fds;
        ret = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
        DIE(ret < 0, "select");

        if (FD_ISSET(STDIN_FILENO, &tmp_fds)) 
        {
            HandleKeyboardInput();
        } 
        else if (FD_ISSET(sockfd, &tmp_fds)) 
        {
            HandleServerMessage();
        }

        if(!client_run)
        {
            break;
        }
    }
}

void Client::HandleKeyboardInput()
{
 // Received input from keyboard
    char buffer[100];
    bzero(buffer, 100);
    fgets(buffer, 99, stdin);

    if (strncmp(buffer, "exit", strlen("exit")) == 0) {
        client_run = false;
        return;
    }

    char topic[51];
    bool sf;
    if (sscanf(buffer, "subscribe %s %d", topic, (int *)&sf) == 2) {
        // Send subscribe message
        TCPMessage msg;
        msg.msg_type = 0; // subscribe type message
        strcpy(msg.client_id, id_client.c_str());
        strcpy(msg.topic, topic);
        msg.sf = sf;

        SendTCP_Message(sockfd, &msg);

        printf("Subscribed to topic.\n");

    } else if (sscanf(buffer, "unsubscribe %s", topic) == 1) {
        // Send unsubscribe message
        TCPMessage msg;
        msg.msg_type = 1; // unsubscribe type message
        strcpy(msg.client_id, id_client.c_str());
        strcpy(msg.topic, topic);

        SendTCP_Message(sockfd, &msg);

        printf("Unsubscribed from topic.\n");
    }
}

void Client::HandleServerMessage()
{
    // Received message from server
    TCPMessage msg;
    ReceiveTCP_Message(sockfd, &msg);

    if (msg.msg_type == 3) {
        client_run = false;
        return;
    } else if (msg.msg_type == 4) {
        PrintTCP_Message(&msg);
    }
}

void Client::SendTCP_Message(int sockfd, TCPMessage *msg) {
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

int Client::ReceiveTCP_Message(int sockfd, TCPMessage *msg) {
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

void Client::PrintTCP_Message(TCPMessage *msg)
{
    struct in_addr addr;
    addr.s_addr = msg->src_addr;
    printf("%s:%hu - %s - ", inet_ntoa(addr), msg->port, msg->topic);

    if (msg->data_type == 0) 
    {
        printf("INT - ");
        if(msg->content.i.sign == 1) 
        {
            printf("-");
        }
        printf("%u\n", msg->content.i.val);
    } 
    else if (msg->data_type == 1) 
    {
        uint16_t n = msg->content.sr.val;
        printf("SHORT_REAL - %u.%02u\n", n / 100, n % 100);
    } 
    else if (msg->data_type == 2) 
    {
        printf("FLOAT - ");
        if (msg->content.f.sign == 1) 
        {
            printf("-");
        }
        uint8_t comma_pos = msg->content.f.pos;
        uint32_t n = msg->content.f.val;

        printf("%f\n", n * pow(10, -comma_pos));
    } 
    else if (msg->data_type == 3) 
    {
        printf("STRING - %s\n", msg->content.str.txt);
    } 
    else 
    {
        printf("Invalid message\n");
    }
}