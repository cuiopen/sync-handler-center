#ifndef _PROTO_H
#define _PROTO_H

#include <stdint.h>

struct ProtoHeader
{
    uint32_t pkg_len;
    uint32_t seq_num;
    uint16_t cmd_id;
    uint32_t status_code;
    uint32_t user_id;
}__attribute__((packed));

struct ProtoBody
{
    char     identity[64];
}__attribute__((packed));

struct Content
{
    char     identity1[64];
    char     identity2[64];
}__attribute__((packed));



#define MAX_LEN 1024
#define MAX_MESSAGE_LEN 255
#define MAX_SEND_TIMES  500
#define MAX_KEY_LEN     255
#define MAX_GAME_ID     1000


// 单个游戏每次最多select的个数
//#define MAX_COUNT 100

#define REPEATED_TRANS_ID 1004
#define NET_ERR           1003
#define DB_ERR            1002



#endif
