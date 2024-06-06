#ifndef UDP_CLIENT_MODULE_H
#define UDP_CLIENT_MODULE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#define SERVER_PORT_H 12345
#define SERVER_IP_S "127.0.0.1"
#define CLIENT_IP_S SERVER_IP_S
#define CLIENT_PORT_H 54321
#define BUFFER_SIZE 1024

int create_socket(int *sockfd);
void fill_ip_header(struct iphdr *iph, int payload_len);
void fill_udp_header(struct udphdr *udph, int payload_len);
int send_packet(int sockfd, const char *data, struct sockaddr_in *server_addr);
int receive_packet(int sockfd, struct sockaddr_in *server_addr);

#endif