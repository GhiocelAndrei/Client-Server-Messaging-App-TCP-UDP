# Client-Server-Messaging-App-TCP-UDP

## Description
The Client-Server Messaging App (TCP/UDP) is a network-based messaging application that enables real-time communication between multiple clients and a server. It leverages the TCP (Transmission Control Protocol) and UDP (User Datagram Protocol) protocols to facilitate reliable and efficient communication.

The application consists of two main components: the server and the client. The server acts as a centralized entity that receives messages from clients and relays them to the appropriate recipients. Each client connects to the server to send and receive messages.
The server relays UDP messages received to the TCP clients.

## Features

#### TCP Server-Client Communication
The application supports communication between the server and clients using the TCP protocol. TCP ensures reliable and ordered message delivery, making it suitable for scenarios where message integrity is crucial.

#### UDP Server-Client Communication
Additionally, the application provides support for server-client communication using the UDP protocol. UDP offers faster transmission speeds and is ideal for scenarios where real-time messaging is a priority over reliability.

#### Subscription and Store-Forward Features
TCP Clients have the option to subscribe to different topics and receive news/messages from the UDP Clients through the Server. Clients can subscribe with a store-forward options which will save their messages when they are offline.

#### User-Friendly Interface
The Client-Server Messaging App features a user-friendly command-line interface, allowing users to interact with the application seamlessly.

## Usage
To use the Client-Server Messaging App, follow these steps:

#### 1. Clone the repository to your local machine using the command:
   git clone https://github.com/GhiocelAndrei/Client-Server-Messaging-App-TCP-UDP.git
#### 2. Compile the server and client source files using the command:
   make
#### 3. Run the server and the client(s) in separate terminal windows:
  ./server [PORT]
  Server commands :<br>
  exit -> Inchide instanta <br>
  
  ./subscriber [client_id] [ip_server] [server_port]<br>
  Subscribers commands :<br>
  subscribe <topic> <sf> - sf can be 0 or 1 -> 1 if store-forward is on, 0 if it's off<br>
  unsubscribe <topic><br>
  exit -> Inchide instanta 
  
#### 4. Simulate receiving UDP Messages to the server using the command:
  ./python3 pcom_hw2_udp_client/udp_client.py
 
## Testing all features :
  Run : ./python3 test.py 

#### The script runs the following tests:

1. compile --> compiles the two executables
2. server_start --> starts the server (the default port is 12345, but it can be changed by modifying the port variable on line 15 of the script)
3. c1_start --> starts a subscriber C1; if the test fails, skip to step 21
4. data_unsubscribed --> generates a message on each topic using the UDP test client; the TCP subscriber should not receive anything; the UDP test client is expected to be in a directory named pcom_hw2_udp_client, but this can be changed by modifying the udp_client_path variable on line 21 of the script
5. c1_subscribe_all --> subscribes C1 to all topics with SF=0
6. data_subscribed --> generates a message on each topic that C1 should receive and display correctly
7. c1_stop --> disconnects C1 from the server; if the test fails, skip to step 20
8. c1_restart --> generates a message and then reconnects C1; if the test fails, skip to step 20
9. data_no_clients --> verifies that C1 does not receive the message generated while it was disconnected
10.same_id --> attempts to start and connect a second subscriber with the same ID
11. c2_start --> starts a subscriber C2; if the test fails, skip to step 20
12. c2_subscribe --> subscribes C2 to a topic with SF=0
13. c2_subscribe_sf --> subscribes C2 to a topic with SF=1
14. data_no_sf --> generates a message on a topic to which both C1 and C2 are subscribed (with SF=0)
15. data_sf --> generates a message on a topic to which C1 (with SF=0) and C2 (with SF=1) are subscribed
16. c2_stop --> disconnects C2 from the server; if the test fails, skip to step 20
17. data_no_sf2 --> generates a message on a topic to which both C1 and C2 are subscribed (with SF=0)
18. data_sf2 --> generates three messages on a topic to which C1 (with SF=0) and C2 (with SF=1) are subscribed
19. c2_restart_sf --> reconnects C2 and verifies the receipt of the three messages on the topic with SF=1
20. quick_flow --> generates all message types 30 times and sends them in rapid succession, verifying that they all reach C1 correctly
21. server_stop --> stops the server.

 
 
 
