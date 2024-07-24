#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  
#include <stdbool.h>
#include <pthread.h>

// Quick Rundown of how the server side works
/*
Section 1 : First create the socket which you communicate through for the client
Then you configure the servers address which we set to any, meaning listen to any incoming
Section 2 : Now you are gonna listen for incoming connection requests to the server socket
you can accept up to 10
when you have connections, the program will accept the connection and store the connecting
clientSocketFD. These socketFD is used to communicate which is why its important
*/

#define NAME_SIZE 1024



struct AcceptedSocket {
    int clientFD;
    char name[50];
    struct sockaddr_in address;
    int error;
    bool acceptedSuccessfully;
};

struct AcceptedSocket acceptedSocketList[10];
int acceptedSocketCount = 0;

struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD);
void* handleIncomingConnectionData(void* clientSocketStruct);
void printIncomingData(struct AcceptedSocket *clientSocket);
void sendReceivedMessageToOtherClients(char* buffer, int clientSocketFDSayingMessage);
void sendMessageToSpecifiedClient(char* messageReceieved, const char* nameToSendTo);

int main() {
    int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFD < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(2000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Bind failed");
        close(serverSocketFD);
        return 1;
    }

    if (listen(serverSocketFD, 10) < 0) {
        perror("Listen failed");
        close(serverSocketFD);
        return 1;
    }

    printf("Server socket was bound successfully!!\n");

    while (1) {
        struct AcceptedSocket* clientSocketStruct = acceptIncomingConnection(serverSocketFD);
        if (clientSocketStruct->acceptedSuccessfully) {
            acceptedSocketList[acceptedSocketCount++] = *clientSocketStruct;
            pthread_t id;
            pthread_create(&id, NULL, handleIncomingConnectionData, clientSocketStruct);
        } else {
            free(clientSocketStruct);
        }
    }

    close(serverSocketFD);
    return 0;
}

struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressSize = sizeof(clientAddress);
    int clientSocketFD = accept(serverSocketFD, (struct sockaddr*)&clientAddress, &clientAddressSize);

    struct AcceptedSocket* acceptedSocket = malloc(sizeof(struct AcceptedSocket));
    acceptedSocket->address = clientAddress;
    acceptedSocket->clientFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = (clientSocketFD >= 0);
    if (clientSocketFD < 0) {
        acceptedSocket->error = clientSocketFD;
        perror("Accept failed");
    } else {
        char name[50];
        ssize_t nameLength = recv(clientSocketFD, name, NAME_SIZE - 1, 0);
        if (nameLength >= 0) {
            name[nameLength - 1] = '\0';
            strncpy(acceptedSocket->name, name, sizeof(acceptedSocket->name) - 1);
        } else {
            perror("Name reception failed");
            acceptedSocket->acceptedSuccessfully = false;
        }
    }

    return acceptedSocket;
}

void* handleIncomingConnectionData(void* clientSocketStruct) {
    struct AcceptedSocket* clientSocket = (struct AcceptedSocket*)clientSocketStruct;
    printIncomingData(clientSocket);
    free(clientSocket);
    return NULL;
}

void printIncomingData(struct AcceptedSocket *clientSocket) {
    char messageReceieved[1024];
    char messageWithName[1024];
    while (1) {
        ssize_t amountReceived = recv(clientSocket->clientFD, messageReceieved, sizeof(messageReceieved) - 1, 0);
        if (amountReceived > 0) {
            messageReceieved[amountReceived] = '\0'; // Null-terminate the received data
            
            if (strncmp(messageReceieved, "send ", 5) == 0) {
                // Define the delimiter
                const char delim[] = " ";
                char *token = strtok(messageReceieved, delim);
                token = strtok(NULL, delim);   

                if (token != NULL) {
                    char nameToSendTo[10];
                    strncpy(nameToSendTo, token, sizeof(nameToSendTo) - 1);
                    nameToSendTo[sizeof(nameToSendTo) - 1] = '\0'; // Ensure null-termination

                    sprintf(messageWithName, "[PRIVATE] %s: %s", clientSocket->name, messageReceieved + 5 + strlen(nameToSendTo) + 1);   
                    sendMessageToSpecifiedClient(messageWithName, nameToSendTo);
                }
            } else {
                sprintf(messageWithName, "%s: %s", clientSocket->name, messageReceieved);
                printf("%s\n", messageWithName);
                sendReceivedMessageToOtherClients(messageWithName, clientSocket->clientFD);
            }
        } else if (amountReceived < 0) {
            perror("Receiving data failed");
        } else {
            // Client disconnected
            break;
        }
    }

    close(clientSocket->clientFD);
}


void sendReceivedMessageToOtherClients(char* buffer, int clientSocketFDSayingMessage) {
    for (int i = 0; i < acceptedSocketCount; i++) {
        if (acceptedSocketList[i].clientFD != clientSocketFDSayingMessage) {
            send(acceptedSocketList[i].clientFD, buffer, strlen(buffer), 0);
        }
    }
}

void sendMessageToSpecifiedClient(char* messageReceieved, const char* nameToSendTo) {
    for (int i = 0; i < acceptedSocketCount; i++) {
        if (strcmp(acceptedSocketList[i].name, nameToSendTo) == 0) {
            send(acceptedSocketList[i].clientFD, messageReceieved, strlen(messageReceieved), 0);
            break;
        }
    }
}

