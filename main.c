#ifdef WIN32
    #include <windows.h>
    #include <winsock.h>
    #include <ws2tcpip.h>
#else
    #define closesocket close
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"

/*!------------------------------------------------------------------------
 * Program:   server
 *
 * Purpose:   allocate a socket and then repeatedly execute the following:
 *              (1) wait for the next connection from a client
 *              (2) send a short message to the client
 *              (3) close the connection
 *              (4) go back to step (1)
 *
 * Syntax:    server < port >
 *
 *               port  - protocol port number to use
 * *
 *------------------------------------------------------------------------
 */

int createConnect(int port) {
    struct  protoent *ptrp;  /* pointer to a protocol table entry   */
    struct  sockaddr_in sad; /* structure to hold server's address  */
    int     sd;              /* socket descriptors                  */
    int optval = 1;          /* options set by setsockopt           */

#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(0x0101, &wsaData);
#endif
    memset((char *)&sad, 0, sizeof(sad)); /* clear sockaddr structure */
    sad.sin_family = AF_INET;             /* set family to Internet     */
    sad.sin_addr.s_addr = INADDR_ANY;     /* set the local IP address   */
    /* Check command-line argument for protocol port and extract
       port number if one is specified.*/
    if (port > PUBLIC_PORT)         /* test for illegal value       */
        sad.sin_port = htons((u_short)port);
    else {                          /* print error message and exit */
        printf("Bad port number %d\n",port);
        exit(EXIT_FAILURE);
    }
    /* Map TCP transport protocol name to protocol number */
    if ( ((int)(ptrp = getprotobyname("tcp"))) == 0) {
        printf("cannot map \"tcp\" to protocol number");
        exit(EXIT_FAILURE);
    }
    /* Create a socket */
    sd = socket (AF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sd < 0) {
        printf("Socket creation failed\n");
        exit(EXIT_FAILURE);
    }
    /* Eliminate "Address already in use" error from bind. */
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int)) < 0) {
        printf("Set socket option failed\n");
        exit(EXIT_FAILURE);
    }
    /* Bind a local address to the sockexit(EXIT_FAILURE);et */
    if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        printf("Bind failed\n");
        exit(EXIT_FAILURE);
    }
    /* Specify size of request queue */
    if (listen(sd, QLEN) < 0) {
        printf("Listen failed\n");
        exit(EXIT_FAILURE);
    }
    return sd;
}

void usage(char *prg) {
    printf("usage:%s <port>",prg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    socklen_t alen;            /* length of address                   */
    int     clientSocket,listenSocket; /* socket descriptors                  */
    struct  sockaddr_in cad; /* structure to hold client's address  */
    char    buf[BUFFER_SIZE];/* buffer for string the server sends  */
    int     n;               /* number of characters received       */

    if (argc !=2)
        usage(argv[0]);

    /* Main server loop - accept and handle requests */
    listenSocket = createConnect(atoi(argv[1]));
    printf("Server is starting at port:%s\n",argv[1]);
    while (1) {
        alen = sizeof(cad);
        if ((clientSocket = accept(listenSocket, (struct sockaddr *)&cad, &alen)) < 0) {
            printf("Accept failed\n");
            exit(EXIT_FAILURE);
        }
        n = recv(clientSocket, buf, BUFFER_SIZE-1, 0);
        buf[n] = '\0';
        send(clientSocket, buf, n, 0);
        closesocket(clientSocket);
        printf(buf);
    }
    closesocket(listenSocket);
#ifdef WIN32
    WSACleanup();
#endif
    return EXIT_SUCCESS;
}

