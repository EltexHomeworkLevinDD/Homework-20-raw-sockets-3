#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "udp_client_module.h"

int main() {
    int sockfd;
    struct sockaddr_in server_addr;

    create_socket(&sockfd);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT_H);
    inet_pton(AF_INET, SERVER_IP_S, &(server_addr.sin_addr));

    const char text[] = "Hello";
    send_packet(sockfd, text, &server_addr);
    receive_packet(sockfd, &server_addr);

    close(sockfd);
    return 0;
}