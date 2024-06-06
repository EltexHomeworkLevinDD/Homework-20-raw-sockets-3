#include "udp_client_module.h"

/**
 * @brief Создает RAW сокет и устанавливает опцию IP_HDRINCL.
 * 
 * @param sockfd Указатель на дескриптор сокета, который будет создан.
 * @return int Возвращает 0 в случае успеха и -1 в случае ошибки.
 */
int create_socket(int *sockfd) {
    // Создание RAW сокета
    if ((*sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        perror("socket creation failed");
        return -1;
    }

    // Установка опции IP_HDRINCL для ручного заполенния IP заголовка
    int optval = 1;
    if (setsockopt(*sockfd, IPPROTO_IP, IP_HDRINCL, &optval, 
                sizeof(optval)) != 0) {
        perror("IP_HDRINCL option set failed");
        return -1;
    }

    return 0;
}

/**
 * @brief Заполняет IP заголовок.
 * 
 * @param iph Указатель на структуру iphdr, которую необходимо заполнить.
 * @param payload_len Длина полезной нагрузки в байтах.
 */
void fill_ip_header(struct iphdr *iph, int payload_len) {
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = 
            htons(sizeof(struct iphdr) + sizeof(struct udphdr) + payload_len);
    iph->id = htons(54321); // Идентификатор пакета
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0; // Контрольная сумма будет рассчитана автоматически
    inet_pton(AF_INET, CLIENT_IP_S, (void*)&iph->saddr); // Источник
    inet_pton(AF_INET, SERVER_IP_S, (void*)&iph->daddr); // Назначение
}

/**
 * @brief Заполняет UDP заголовок.
 * 
 * @param udph Указатель на структуру udphdr, которую необходимо заполнить.
 * @param payload_len Длина полезной нагрузки в байтах.
 */
void fill_udp_header(struct udphdr *udph, int payload_len) {
    udph->source = htons(CLIENT_PORT_H);
    udph->dest = htons(SERVER_PORT_H);
    udph->len = htons(sizeof(struct udphdr) + payload_len);
    udph->check = 0; // Контрольную сумму не рассчитываем
}

/**
 * @brief Отправляет пакет на указанный сервер.
 * 
 * @param sockfd Дескриптор сокета.
 * @param data Указатель на данные, которые нужно отправить.
 * @param server_addr Указатель на структуру sockaddr_in с адресом сервера.
 * @return int Возвращает 0 в случае успеха и -1 в случае ошибки.
 */
int send_packet(int sockfd, const char *data, struct sockaddr_in *server_addr) {
    // Создать пустой буфер
    char packet[BUFFER_SIZE];
    memset(packet, 0, BUFFER_SIZE);

    // Идентифицировать заголовки IP, UDP и Payload
    struct iphdr *iph = (struct iphdr *) packet;
    struct udphdr *udph = (struct udphdr *) (packet + sizeof(struct iphdr));
    char *payload = packet + sizeof(struct iphdr) + sizeof(struct udphdr);

    int payload_len = strlen(data);

    // Заполнить заголовки и Payload
    fill_ip_header(iph, payload_len);
    fill_udp_header(udph, payload_len);
    strncpy(payload, data, payload_len+1);

    // Отправить пакет на указанный адрес
    if (sendto(sockfd, packet, 
                sizeof(struct iphdr) + sizeof(struct udphdr) + payload_len, 0,
               (struct sockaddr *) server_addr, sizeof(*server_addr)) < 0) {
        perror("sendto()");
        close(sockfd);
        return -1;
    }

    printf("Packet sent to %s:%d\n", inet_ntoa(server_addr->sin_addr), 
                ntohs(server_addr->sin_port));
    return 0;
}

/**
 * @brief Принимает пакет от сервера.
 * 
 * @param sockfd Дескриптор сокета.
 * @param server_addr Указатель на структуру sockaddr_in с адресом сервера.
 * @return int Возвращает 0 в случае успеха и -1 в случае ошибки.
 */
int receive_packet(int sockfd, struct sockaddr_in *server_addr) {
    char buffer[BUFFER_SIZE];
    // Принимать постоянно
    while (1) {
        // Обнулить буффер
        memset(buffer, 0, BUFFER_SIZE);
        socklen_t len = sizeof(*server_addr);
        // Получить пакет
        int recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                                (struct sockaddr *) server_addr, &len);
        if (recv_len < 0) {
            perror("recvfrom()");
            continue;
        }

        // Идентифицировать заголовки IP, UDP
        struct iphdr *recv_iph = (struct iphdr *) buffer;
        (void)recv_iph; // No Warnings
        struct udphdr *recv_udph = (struct udphdr *) (buffer + sizeof(struct iphdr));

        printf("Received from source port: %d dest port: %d\n",
               ntohs(recv_udph->source), ntohs(recv_udph->dest));

        // Сравнить полученные Port'ы с ожидаемыми
        if (ntohs(recv_udph->source) == SERVER_PORT_H && 
                    ntohs(recv_udph->dest) == CLIENT_PORT_H) {
            // Удентифицировать Payload
            char *recv_data = 
                        buffer + sizeof(struct iphdr) + sizeof(struct udphdr);
            // Расчитать длину Payload
            int data_len = recv_len - (sizeof(struct iphdr) + 
                        sizeof(struct udphdr));
            recv_data[data_len] = '\0';
            // Вывести
            printf("Received from server: %s\n", recv_data);
            return 0;
        }
    }
}
