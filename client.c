#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>  
#include <pthread.h>


// Quick Rundown of how the client side works 
/*
Section 1 : You first create a socket which is basically a file descriptor used to pass in data
Then you specify the address details you are trying to connect to in a struct named address

Section 2 : Now you are trying to connect to the address you specified and handle the result
*/

void createNewPrintThread(int socketFileDescriptor);
void startListeningAndPrinting(int socketFileDescriptor);


int main() {

    // SECTION 1

    //create socket
    int socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor < 0) {
        perror("Socket creation failed");
        return 1;
    }
    // address config
    char* ip = "127.0.0.1";
    struct sockaddr_in *address = malloc(sizeof(struct sockaddr_in));
    if (address == NULL) {
        perror("Memory allocation failed");
        close(socketFileDescriptor);
        return 1;
    }
    memset(address, 0, sizeof(struct sockaddr_in)); 
    address->sin_family = AF_INET;
    address->sin_port = htons(2000);
    if (inet_pton(AF_INET, ip, &address->sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        free(address);
        close(socketFileDescriptor);
        return 1;
    }


    // SECTION 2 

    //try connection
    int result = connect(socketFileDescriptor, address, sizeof(*address));
    if (result == 0) {
        printf("[SERVER] You've Connected to the Chatroom!\n");
        printf("[SERVER] Enter your name: ");

        char *name = NULL;
        size_t nameSize = 0;
        ssize_t nameCharCount = getline(&name, &nameSize, stdin);

        send(socketFileDescriptor, name, nameCharCount, 0);

        free(name);  // Free the allocated memory for name
    } else {
        perror("[SERVER] Connection failed!\n");
        return 1;
    }


    //sending message through your socket for server to accept and read
    char *message = NULL;
    size_t messageSize = 0;
    printf("[SERVER] Type the messages you would like to send\n");

    createNewPrintThread(socketFileDescriptor);
    
    while(1) {
        //getting the lines
        printf("You - ");
        ssize_t charCount = getline(&message, &messageSize, stdin);
        //check for exit command
        if (charCount > 0 && strcmp(message, "exit\n") == 0) {
            break;
        }
        //sending
        ssize_t amountSent = send(socketFileDescriptor, message, charCount, 0);
    }

    close(socketFileDescriptor);

}


void createNewPrintThread(int socketFileDescriptor){
    pthread_t id;
    pthread_create(&id, NULL, startListeningAndPrinting, socketFileDescriptor);
    return;
}

void startListeningAndPrinting(int socketFileDescriptor) {
    char buffer[1024];
    while (1) {
        ssize_t amountReceived = recv(socketFileDescriptor, buffer, sizeof(buffer) - 1, 0);
        if (amountReceived > 0) {
            buffer[amountReceived] = '\0';

            // Clear the current line and move the cursor to the beginning
            printf("\r\033[K"); 

            // Print the received message
            printf("%s\n", buffer);

            // Reprint the prompt for the user
            printf("You - ");
            fflush(stdout);  // Ensure that the prompt is shown immediately
        } else if (amountReceived < 0) {
            perror("Receiving data failed");
        } else {
            // Server disconnected
            break;
        }
    }

    close(socketFileDescriptor);
}