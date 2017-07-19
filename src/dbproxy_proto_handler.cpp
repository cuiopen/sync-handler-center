#include "dbproxy_proto_handler.h"
#include "client_info.h"
#include "proto.h"            
#include "singleton.h"
#include "proxy.h"
#include "mempool.h"
// #include "interface_proto_handler.h"

#include "switch_proto_handler.h"
#include "dao.h"

extern mempool_t * g_mempool;
extern list_head_t *g_client_fd_list;
extern list_head_t g_net_timer;
extern int g_timeout_s;

extern GidHandlerMapSwitch g_gid_handler_map_switch;

// extern std::string table_name;



int DbproxyProtoHandler::GetPkgLenSer(const char * buf, uint32_t len)
{
    if (len < sizeof(uint32_t)) {
        return 0;
    }
    uint32_t proto_len = *(uint32_t *)buf;
    if (proto_len < sizeof(ProtoHeader))
    {
        ERROR_LOG("DbproxyProtoHandler GetPkgLenSer failed, proto_len is %u",
                                                            proto_len);
        return -1;
    }
    return proto_len;
}


bool DbproxyProtoHandler::GetSeqNum(const char * buf, uint32_t len, uint32_t &seq_num)
{
    if (len < sizeof(ProtoHeader)) 
    {
        return false;
    }

    ProtoHeader * ph = (ProtoHeader*)buf;
    if (len != ph->pkg_len)
    {
        return false;
    }
    
    seq_num = ph->seq_num;
    return true;
}


void DbproxyProtoHandler::ProcPkgSer(const char * buf, uint32_t len)
{

    ProtoHeader * header = (ProtoHeader *)buf;

    ClientInfo *ci = ClientInfo::GetClientInfo(header->seq_num);

    if (!ci) 
    {
        DEBUG_LOG("DbproxyProtoHandler ProcPkgSer get NULL ClientInfo, perhaps client closed or timeout");
        return;
    }
    if (header->status_code == DB_ERR || header->status_code == NET_ERR)
    {
        ERROR_LOG("lenght:%u][cmd_id:%u][user_id:%u][status:%u]---failed",
                header->pkg_len,
                header->cmd_id, 
                header->user_id,
                header->status_code);
        return;

    }
    INFO_LOG("lenght:%u][cmd_id:%u][user_id:%u][status:%u]---ok",
                header->pkg_len, 
                header->cmd_id, 
                header->user_id, 
                header->status_code);


    GidHandlerMapSwitch::iterator it = g_gid_handler_map_switch.begin();
    it = g_gid_handler_map_switch.find(ci->game_id);
    if (it == g_gid_handler_map_switch.end())
    {
        DEBUG_LOG("g_gid_handler_map_switch find no gamd_id");
        // return false;
        // 没有swithch的情况
    }

    for (unsigned int i = 0; i != g_gid_handler_map_switch.count(ci->game_id); i++, it++)
    {
        if (!it->second->ProcessIdentity(ci, ci->ser_seq, ci->userid))
        {
            ERROR_LOG("switch processidentity failed");
        }
    }
    
    // 提前删除会导致无法处理switch的回包，后面会一起删除
    // Proxy::DestroyClient(ci->ser_seq);
    // 删除clientinfo
    if ((UpdateOneMessage(ci->sync_id, 1)) < 0)
    {
        ERROR_LOG("UpdateOneMessage status failed");
    }
    Proxy::DestroyClient(ci->ser_seq);
}

bool DbproxyProtoHandler::ProcessIdentity(ClientInfo *ci, uint32_t seq_num, uint32_t userid)
{
    char out_buf[1024] = {0};
    ProtoHeader * out_header = (ProtoHeader*)(ci->buf);
    
    // 序列号需要改下
    out_header->seq_num = seq_num;
    out_header->user_id = userid;
    // 用ci的一个字段表示包体长度有些不合理，为了后面更好的拓展
    // 服务本身不知道包体的情况，只是提供了转发的功能
    //out_header->pkg_len = ci->pkg_len;
    // 整个包存在clientinfo中
    memcpy(out_buf, ci->buf, ci->pkg_len);
    if (!SendByInt(out_buf, ci->pkg_len, ci->userid))
    {
        ERROR_LOG("DbproxyProtoHandler DbproxyProcessIdentity failed");
        return false;
    }
    // TODO 会频发调用发送
    // 只有当第一次发送才会添加，后续不再添加
    // 解决多次添加，链表陷入死循环的bug
    if (ci->send_times == 1)
    {
        timer_add_event(&g_net_timer,
                        &ci->event,
                        time(NULL) + g_timeout_s,
                        Proxy::DoServerTimeout,
                        ci,
                       NULL);
    }
    return true;
}
//bool DbproxyProtoHandler::ProcessIdentity(ClientInfo *ci, uint32_t seq_num, uint32_t userid)
//{
//    char out_buf[1024] = {0};
//    char in_buf[1024] = {0};
//    size_t in_len;
//
//    ProtoHeader * out_header = (ProtoHeader*)(ci->buf);
//    // 序列号需要改下
//    out_header->seq_num = seq_num;
//    out_header->user_id = userid;
//    
//    // 用ci的一个字段表示包体长度有些不合理，为了后面更好的拓展
//    // 服务本身不知道包体的情况，只是提供了转发的功能
//
//    //out_header->pkg_len = ci->pkg_len;
//
//    // 整个包存在clientinfo中
//
//    memcpy(out_buf, ci->buf, ci->pkg_len);
//        
//
//    if (!SendAndRecvByInt(out_buf, 
//                          out_header->pkg_len, 
//                          in_buf, 
//                          sizeof(in_buf),
//                          in_len,
//                          ci->userid))
//    {
//        ERROR_LOG("dbproxy SendAndRecvByInt failed");
//        return false;
//    }
//
//    ProtoHeader * header = (ProtoHeader*)in_buf;
//
//    if (header->status_code == DB_ERR || header->status_code == NET_ERR)
//    {
//        ERROR_LOG("lenght:%u][cmd_id:%u][user_id:%u][status:%u]---failed",
//                header->pkg_len,
//                header->cmd_id,
//                header->user_id,
//                header->status_code);
//        return false;
//    }
//    INFO_LOG("lenght:%u][cmd_id:%u][user_id:%u][status:%u]---ok",
//                header->pkg_len,
//                header->cmd_id,
//                header->user_id,
//                header->status_code);
//    // 发送switch
//    GidHandlerMapSwitch::iterator it = g_gid_handler_map_switch.begin();
//    it = g_gid_handler_map_switch.find(ci->game_id);
//    if (it == g_gid_handler_map_switch.end())
//    {
//        DEBUG_LOG("g_gid_handler_map_switch find no gamd_id");
//        // return false;
//        // 没有swithch的情况
//    }
//
//    for (unsigned int i = 0; i != g_gid_handler_map_switch.count(ci->game_id); i++, it++)
//    {
//        if (!it->second->ProcessIdentity(ci, ci->ser_seq, ci->userid))
//        {
//            ERROR_LOG("switch processidentity failed");
//        }
//    }
//    
//    // 提前删除会导致switch拿不到发送信息
//    Proxy::DestroyClient(ci->ser_seq);
//    // 删除clientinfo
//    // Proxy::DestroyClient(ci->ser_seq);
//    if ((UpdateOneMessage(ci->sync_id, 1, table_name.c_str())) < 0)
//    {
//        ERROR_LOG("UpdateOneMessage failed, table_name %s", table_name.c_str());
//    }
//
//    return true;
//
//}

