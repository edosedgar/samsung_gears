#include <net_connection.h>
#include "net.h"

int init_connection(const char *host, conn_info_h conn_ih, appdata_h ad)
{
    int error_code;
    struct hostent *hostaddr;
    struct sockaddr_in servaddr;
    int sockfd = -1;
    char msg[MSG_LEN] = {};
    memset(&servaddr, 0x00, sizeof(struct sockaddr_in));
    memset(&msg, 0x00, MSG_LEN);
    struct timeval ct;

    conn_ih->state = 0;
    ad->conn_ih = (void *)conn_ih;
    error_code = connection_create(&(conn_ih->connection));
    if (error_code != CONNECTION_ERROR_NONE)
        return error_code;

    error_code = connection_get_type(conn_ih->connection,
                                     &(conn_ih->net_state));
    if (error_code == CONNECTION_ERROR_NONE)
        dlog_print(DLOG_INFO, LOG_TAG, "Network connection type: %d",
                   conn_ih->net_state);

    error_code = connection_get_ip_address(conn_ih->connection,
                     CONNECTION_ADDRESS_FAMILY_IPV4, &(conn_ih->ip_addr));
    if (error_code != CONNECTION_ERROR_NONE) {
        dlog_print(DLOG_INFO, LOG_TAG, "IP address: %s",  conn_ih->ip_addr);
        free(conn_ih->ip_addr);
        return error_code;
    }

    /* Create socket for current connection */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        dlog_print(DLOG_INFO, LOG_TAG, "socket error\n");
        return 1;
    }
    conn_ih->sockfd = sockfd;

    /* Connecting to server */
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    hostaddr = gethostbyname(host);
    memcpy(&servaddr.sin_addr, hostaddr->h_addr, hostaddr->h_length);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0) {
        dlog_print(DLOG_INFO, LOG_TAG, "connect() error: %s\n", strerror(errno));
        close(sockfd);
        connection_destroy(conn_ih->connection);
        return 1;
    }

    /* Send handshake to initiate connection */
    char *user_msg = "dbebf164ffefe13";
    int count;
    if ((count = write(sockfd, user_msg, strlen(user_msg))) < 0) {
        close(sockfd);
        return 1;
    }
    conn_ih->state = 1;
    elm_object_text_set(ad->l_conn, "<color=#FFFF20FF><align=center>Server: Active</align></color>");

    /* Send out current time */
    gettimeofday(&ct, NULL);
    sprintf(msg, FRMT_TIM, ct.tv_sec * 1000 + ct.tv_usec / 1000);
    write(sockfd, msg, MSG_LEN);
    return 0;
}

int check_connection(const char *host, appdata_h ad)
{
    conn_info_h conn_ih = ad->conn_ih;
    int error = 0;
    char msg[MSG_LEN] = "";

    if (!(conn_ih->state)) {
        error = init_connection(host, conn_ih, ad);
        goto done;
    }

    /* Check if server is alive and affect conn_ih->state*/
    sprintf(msg, FRMT_SRV, "ping");
    error = write(conn_ih->sockfd, msg, MSG_LEN);
    if (error < 0) {
        close(conn_ih->sockfd);
        connection_destroy(conn_ih->connection);
        conn_ih->state = 0;
    }

done:
    if (conn_ih->state)
        elm_object_text_set(ad->l_conn, "<color=#FFFF20FF><align=center>Server: Active</align></color>");
    else
        elm_object_text_set(ad->l_conn, "<color=#FFFF20FF><align=center>Server: Inactive</align></color>");
    return error;
}

int conn_sent(appdata_h ad, char *msg)
{
    conn_info_h conn_ih = ad->conn_ih;

    if (conn_ih->state) {
        write(conn_ih->sockfd, msg, MSG_LEN);
        return 0;
    }

    return -1;
}
