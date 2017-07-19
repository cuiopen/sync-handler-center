#ifndef _CENTER_PROTO_H
#define _CENTER_PROTO_H
#include <stdint.h>
#define MAX_BUF_LEN (1024*1024)

typedef struct {
    uint32_t pkg_len;
    uint32_t seq_num;
    uint16_t cmd_id;
    uint32_t status_code;
    uint32_t user_id;
} __attribute__((packed)) proto_header;

typedef struct {
    uint32_t type;
    uint32_t value_len;
    char key[64];
    char value[0];
}__attribute__((packed)) config_info;
//=============================================
//#define QUERY_STRATEGY_CMD	0x2000
#define QUERY_STRATEGY_CMD	0x2100
typedef struct {
	char name[64];
}__attribute__((packed)) name_info;

typedef struct {
	uint32_t version_id;
	char ip[32];
	char path[1024];
	uint32_t name_num;
	name_info name_list[0];
}__attribute__((packed)) serv_query_group_req;

typedef struct {
	char ip[32];
	uint32_t port;
	uint32_t timeout_ms;
	uint8_t forbidden;
}__attribute__((packed)) host_info;

typedef struct{
	uint8_t replicas_strategy;
	uint8_t forbidden;
	uint32_t num;
	host_info list[0];
}__attribute__((packed)) service_replicas_info;

typedef struct{
	char name[64];
	uint8_t group_strategy;
	uint8_t forbidden;
	uint32_t num;
	service_replicas_info list[0];
}__attribute__((packed)) service_group_info;

typedef struct {
	uint32_t version_id;
	uint32_t num;
	service_group_info list[0];
}__attribute__((packed)) serv_query_group_ack_1;

typedef struct {
	uint32_t num;
	config_info list[0];
}__attribute__((packed)) serv_query_group_ack_2;

int atoi_safe(const char *nptr);
#endif
