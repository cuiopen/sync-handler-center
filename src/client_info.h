#ifndef _CLIENTINFO_H
#define _CLIENTINFO_H

#include <unordered_map>
#include <map>
#include <stdint.h>
#include "list.h"
#include "timer.h"
#include "proto.h"

#include "proto_handler.h"
// #include "interface_proto_handler.h"
#include "dbproxy_proto_handler.h"

//#include "proxy.h"

class ClientInfo
{
public:
    static uint32_t GetSerializeNo();
    static ClientInfo* GetClientInfo(uint32_t sn);
    static uint32_t SetClientInfo(ClientInfo *ci);
    static void RemoveClientInfo(uint32_t sn);

public:
    typedef  std::map<int, DbproxyProtoHandler *> GidHandlerMapDbproxy;

    int      DealMessage(time_t now);
    
    int      Init(int game_id, uint32_t sync_id, const char * message, time_t sync_time, list_head_t * game_id_list);
    // 和重发策略相关

private:
    // 类内部调用，获取下次发送的时间间隔
    uint32_t GetNextInterval(uint32_t send_times, time_t now);


public:
    int              cli_fd;
    uint32_t         cli_seq;
    uint16_t         cli_cmd;  // for debug
    uint32_t         userid;   // for debug
    uint32_t         ser_seq;
    
    // game_id 的链表
    // 依据game_id 存储对应的message
    list_head_t      game_id_list_node;

    // fd 链表
    list_head_t      fd_list_node;
    // 定时器
    timer_event_t    event;
	uint32_t		 send_ser_times;
    // 包长
    uint32_t         pkg_len;
    // 来包的game_id
    int              game_id;   
    // 此条记录在数据库中的id,快速对数据库操作
    uint32_t         sync_id;
    // 发送次数
    uint8_t          send_times; 
    // 和GetNextInterval一起判断是否需要发送，和重发策略相关
    uint32_t         last_send_time;
    // 处理标志位, 暂时不用
    uint32_t         deal_flag;
    // 缓冲区，存放message主体
  
    uint32_t         sync_time;
    
    char buf[1024];

    typedef std::unordered_map<uint32_t, ClientInfo*> ClientInfoMap;
    typedef std::unordered_map<uint32_t, ClientInfo*>::iterator ClientInfoMapIter;
    static ClientInfoMap client_info_map;
    static uint32_t serialized_no;
};

#endif
