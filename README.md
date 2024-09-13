# Multithreaded Socket Server in C

This project implements a simple multithreaded chatroom using C sockets. The client connects to a server, allowing multiple users to communicate in real-time. The server handles each connection in a separate thread, enabling simultaneous communication between multiple clients.

## Features

- **Multithreaded Server:** Each client connection is handled in a separate thread.
- **Real-time Messaging:** Clients can send and receive messages concurrently.
- **Private Messaging:** Clients can send private messages to specific users.
- **Graceful Disconnection:** Clients can exit the chatroom, and the server handles disconnections cleanly.
- **Simple Command System:** Type `exit` to leave the chatroom and `send [name]` for private messaging.

## How it Works

- **Client (`client.c`)**: 
  - Creates a socket and connects to the server.
  - Sends the client's name to the server.
  - Uses multithreading to allow the client to both send and receive messages simultaneously.
  - The client can type messages to send to the server or other clients.
  
- **Server (`server.c`)**:
  - Listens for incoming client connections on a specified port.
  - For each connected client, creates a new thread to handle sending and receiving data.
  - Broadcasts received messages to all connected clients, except the sender.
  - Supports private messaging between clients using a specific command syntax.

## Code Overview

### Client (`client.c`)

1. **Create Socket:** The client first creates a socket using the `socket()` function, specifying the IP and port of the server to connect to.
2. **Connect to Server:** After setting up the connection details, the client connects to the server and sends its username.
3. **Send and Receive Messages:** The client enters a loop where it can send messages and simultaneously receive messages from other clients. A separate thread listens for incoming messages.
4. **Exit Handling:** If the client types `exit`, the connection is closed gracefully.

### Server (`server.c`)

1. **Create Socket:** The server creates a listening socket to accept client connections on a specified port.
2. **Accept Clients:** It accepts up to 10 client connections. Each accepted client is managed in a new thread to allow for simultaneous communication.
3. **Message Handling:** Incoming messages from one client are broadcast to all other clients. Private messages can be sent using the command `send [client_name]`.
4. **Disconnection:** When a client disconnects, its socket is closed, and the server stops sending messages to that client.

## Installation and Usage

### Prerequisites

- Linux system with GCC installed.
- Basic knowledge of working with C and networking.

### Build Instructions

To build and run the project:

1. **Clone the repository:**
   ```bash
   git clone https://github.com/your-username/multithreaded-socket-server.git
   cd multithreaded-socket-server
   
2. **Compile the Server:**
   ```bash
   gcc -o server server.c -lpthread
   
3. **Compile the Client:**
   ```bash
   gcc -o client client.c -lpthread

5. **Running the Server:**
   ```bash
   ./server
   
5. **Running the Client:**
   Now in a seperate terminal
   ```bash
   ./client
   
7. **Enter your name when prompted:**
   After running the client, you will be prompted to enter your name.

8. **Start typing your messages:**
   You can now type your messages to send to the server or other clients. To exit the chat, type `exit`.

9. **Private Messaging:**
   To send a private message to another user, use the following syntax:
   ```bash
   send [name] [your_message]



