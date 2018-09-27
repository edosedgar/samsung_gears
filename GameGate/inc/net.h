/*
 * net.h
 *
 *  Created on: Sep 23, 2018
 *      Author: maxwell
 */

#ifndef NET_H_
#define NET_H_

#include <net_connection.h>
#include "biomonitorc.h"
#include "struct.h"
#include "sensors_part.h"
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>

#define HOST_ADDR "172.20.10.6"
#define SERVER_PORT 14342
#define MSG_LEN 96

#define FRMT_HRM "{ \"time\": %lu, \"heart_rate\": %.0f }"
#define FRMT_ACC "{ \"time\": %lu, \"accX\": %.3f, \"accY\": %.3f, \"accZ\": %.3f }"
#define FRMT_SRV "{ \"service\": \"%s\" }"
#define FRMT_TIM "{ \"init_time\": %lu }"

/* Description of current app internet connection */
struct conn_info {
	connection_h connection;
	connection_type_e net_state;
	char *ip_addr;
	int sockfd;
	int state;
};
typedef struct conn_info *conn_info_h;

int init_connection(const char *host, conn_info_h conn_ih, appdata_h ad);

int check_connection(const char *host, appdata_h ad);

int conn_sent(appdata_h ad, char *msg);

#endif /* NET_H_ */
