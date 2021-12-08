#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)
#include <ctype.h>

int flag = 2;

int main() {
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(0, "8080", &hints, &bind_address);


    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
            bind_address->ai_socktype, bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }


    printf("Binding socket to local address...\n");
    if (bind(socket_listen,
                bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(bind_address);


    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    SOCKET max_socket = socket_listen;

    printf("Waiting for connections...\n");

    int ox[20];
    int a = 0;
    int b = 0;
    memset(ox,0,sizeof(ox));
    while(1) {
        fd_set reads;
        reads = master;
        if (select(max_socket+1, &reads, 0, 0, 0) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
            return 1;
        }

        SOCKET i;
        for(i = 1; i <= max_socket; ++i) {
            if (FD_ISSET(i, &reads)) {

                if (i == socket_listen) {
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    SOCKET socket_client = accept(socket_listen,
                            (struct sockaddr*) &client_address,
                            &client_len);
                    if (!ISVALIDSOCKET(socket_client)) {
                        fprintf(stderr, "accept() failed. (%d)\n",
                                GETSOCKETERRNO());
                        return 1;
                    }

                    FD_SET(socket_client, &master);
                    if (socket_client > max_socket)
                        max_socket = socket_client;

                    char address_buffer[100];
                    getnameinfo((struct sockaddr*)&client_address,
                            client_len,
                            address_buffer, sizeof(address_buffer), 0, 0,
                            NI_NUMERICHOST);
                    printf("New connection from %s\n", address_buffer);

                } else {
                    char read[1024];
                    int bytes_received = recv(i, read, 1024, 0);
                    if (bytes_received < 1) {
                        FD_CLR(i, &master);
                        CLOSESOCKET(i);
                        continue;
                    }
                  printf("\ni = %d\n", i);
                  int temp;
                  temp = atoi(read);
                  if(temp == 0)
                    {
                      memset(ox, 0, sizeof(ox));
                      a = b = 0;
                      flag = 2;
                    }
                  else if(temp == 10 && a == 0)
                    a = i;
                  else if (temp == 10 && b == 0 && i != a)
                    b = i;
                  if(temp > 0 && temp < 10){
                  if(ox[temp] == 0 && i == a && flag == 2)
                    {ox[temp] = 1; flag = 1;}
                  else if(ox[temp] == 0 && i == b && flag == 1)
                    {ox[temp] = 2; flag = 2;}
                  }
                  if(((ox[1] && ox[2] && ox[3]) || (ox[4] && ox[5] && ox[6]) ||
                    (ox[7] && ox[8] && ox[9]) || (ox[1] && ox[4] && ox[7]) ||
                    (ox[2] && ox[5] && ox[8]) || (ox[3] && ox[6] && ox[9]) ||
                    (ox[1] && ox[5] && ox[9]) || (ox[3] && ox[5] && ox[9])) == 1)
                      {ox[10] == 1; ox[11] == 1;printf("O win\n");}
                  else if(((ox[1] && ox[2] && ox[3]) || (ox[4] && ox[5] && ox[6]) ||
                    (ox[7] && ox[8] && ox[9]) || (ox[1] && ox[4] && ox[7]) ||
                    (ox[2] && ox[5] && ox[8]) || (ox[3] && ox[6] && ox[9]) ||
                    (ox[1] && ox[5] && ox[9]) || (ox[3] && ox[5] && ox[9])) == 2)
                      {ox[10] == 2; ox[11] == 1;printf("X win\n");}
                  }
                  printf("\n");
                  int k,l, temp;
                  char buf[2048];
                  memset(buf, 0, sizeof(buf));
                  for(k = 1; k < 10; k++)
                    {l = ox[k]; sprintf(buf,"%d",l);}
                      for(k = 1; k <= 9; k++)
                        {
                          if(ox[k] == 1)
                            printf("O\t");
                          else if(ox[k] == 2)
                            printf("X\t");
                          else
                            printf("%d\t", k);
                          if(k == 3 || k == 6)
                            printf("\n");
                          else if (k == 9)
                            printf("\n\n");
                          }
                  printf("\ni = %d, a = %d, b = %d\n", i, a ,b);
                    SOCKET j;
                    for (j = 1; j <= max_socket; ++j) {
                        if (FD_ISSET(j, &master)) {
                            if (j == socket_listen || j == i)
                                continue;
                            else
                                send(j, buf, sizeof(buf), 0);
                        }
                }
            } //if FD_ISSET
        } //for i to max_socket
    } //while(1)

    printf("Closing listening socket...\n");
    CLOSESOCKET(socket_listen);


    printf("Finished.\n");

    return 0;
}
