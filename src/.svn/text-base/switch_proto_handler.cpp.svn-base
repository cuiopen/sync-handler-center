#include "switch_proto_handler.h"
#include "client_info.h"
#include "proto.h"            
#include "singleton.h"
#include "proxy.h"
#include "mempool.h"


extern mempool_t * g_mempool;
extern list_head_t *g_client_fd_list;
extern list_head_t g_net_timer;
extern int g_timeout_s;



int SwitchProtoHandler::GetPkgLenSer(const char * buf, uint32_t len)
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


bool SwitchProtoHandler::GetSeqNum(const char * buf, uint32_t len, uint32_t &seq_num)
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


void SwitchProtoHandler::ProcPkgSer(const char * buf, uint32_t len)
{
    // 只打一个log,并不会影响数据库操作
    ProtoHeader * header = (ProtoHeader *)buf;

    ClientInfo * ci = ClientInfo::GetClientInfo(header->seq_num);

    if (!ci)
    {
        DEBUG_LOG("SwitchProtoHandler ProcPkgSer get NULL ClientInfo, perhaps client closed or timeout");
        return;
    }
    
    // 不需要此log
    //DEBUG_LOG("lenght:%u][cmd_id:%u][user_id:%u][status:%u]---switch--ok",
    //            header->pkg_len,
    //            header->cmd_id,
    //            header->user_id,
    //            header->status_code);
}


bool SwitchProtoHandler::ProcessIdentity(ClientInfo *ci, uint32_t seq_num, uint32_t userid)
{
    
    char out_buf[1024] = {0};
    ProtoHeader * out_header = (ProtoHeader*)ci->buf;

    out_header->seq_num = seq_num;
    out_header->user_id = userid;
    
    // 用ci的一个字段表示包体长度有些不合理，为了后面更好的拓展
    // 服务本身不知道包体的情况，只是提供了转发的功能

    //out_header->pkg_len = sizeof(ProtoHeader) + ci->pkg_body_len;

    memcpy(out_buf, ci->buf, ci->pkg_len);
        
    if (!SendByInt(out_buf, ci->pkg_len, ci->userid))
    {
        ERROR_LOG("DbproxyProtoHandler DbproxyProcessIdentity failed");
        return false;
    }
    
    // 无需关心switch的返回包
    //timer_add_event(&g_net_timer,
    //                &ci->event,
    //                time(NULL) + g_timeout_s,
    //                Proxy::DoServerTimeout,
    //                ci,
    //                NULL);
    return true;
}

