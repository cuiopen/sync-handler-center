#include "startegy_async_handler.h"
#include "center_proto.h"
#include "client_info.h"
#include "proxy.h"
#include "singleton.h"
#include "async_server.h"
#include "mempool.h"
//#include "upstream_proto_handler.h"

extern mempool_t* g_mempool;

extern map<string, ServiceGroupInfo> m_group;;
extern list_head_t g_net_timer;
extern int g_timeout_s;



int StrategyAsyncProtoHandler::GetPkgLenSer(const char *buf, uint32_t len)
{
    if (len < sizeof(uint32_t)) {
        return 0;
    }

    // return *(uint32_t*)buf;

    // 对长度要做一定的检查
    uint32_t proto_len = *(uint32_t*)buf;
    if (proto_len < sizeof(proto_header)) {
        ERROR_LOG("StrategyAsyncProtoHandler GetPkgLenSer failed, proto_len is %u", 
                  proto_len);
        return -1;
    }

    return proto_len;
}

bool StrategyAsyncProtoHandler::GetSeqNum(const char *buf, uint32_t len, 
                                           uint32_t &seq_num)
{
    if (len < sizeof(proto_header)) {
        // TODO
        return false;
    }

    proto_header *ph = (proto_header*)buf;
    if (len != ph->pkg_len) {
        // TODO
        return false;
    }

    seq_num = ph->seq_num;
    return true;
}

void StrategyAsyncProtoHandler::ProcPkgSer(const char *buf, uint32_t len)
{
    proto_header *header = (proto_header*)buf;   
    ClientInfo *ci = ClientInfo::GetClientInfo(header->seq_num);

	map<string, ServiceGroupInfo> g;
	if(!AckPack(buf, len, g)) { 
		ERROR_LOG("Get Startegy failed.");
		return;
	}
    
	if(g != m_group) {
        m_group.clear();
        m_group = g;
	}

//	Proxy::DestroyClient(ci->ser_seq);
	mempool_free(g_mempool, ci);
	ClientInfo::RemoveClientInfo(ci->ser_seq);
	timer_del_event(&ci->event);	
	return;
}

bool StrategyAsyncProtoHandler::AsyncGetProcessIdentity(ClientInfo *ci,
													vector<string> &name_list, 
													uint32_t seq_num)
{
	char out_buf[4096];
	memset(out_buf, 0, sizeof(proto_header));
	proto_header *out_header = (proto_header*)out_buf;
 	ReqPack(out_buf, sizeof(out_buf), name_list);
	out_header->seq_num = seq_num; 

    // 先发送到asg1
    if (!SendByInt(out_buf, out_header->pkg_len, 1)) {
        // 无序删除ClientInfo，删除以及返回客户端的工作交由外层处理
        ERROR_LOG("StrategyAsyncProtoHandler PGetProcessIdentity failed");
		return false;
	}

	timer_add_event(&g_net_timer,
					&ci->event,
					time(NULL) + g_timeout_s,
					Proxy::DoServerTimeout,
					ci,
					NULL);
    return true;
}

