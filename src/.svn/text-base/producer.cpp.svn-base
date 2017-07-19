#include <string.h>
#include "proxy.h"
#include "async_server.h"
#include "log.h"
#include "monitor_api.h"
#include "service.h"
#include "client_info.h"
#include "mempool.h"
#include "proto.h"
#include "list.h"
#include <sys/resource.h>
#include <sys/time.h>
#include "timer.h"
#include "producer.h"

#include "dao.h"


mempool_t* g_mempool_producer = NULL;
list_head_t *g_client_fd_list_producer = NULL;
int g_max_open_fd_producer = 100000;
list_head_t g_net_timer_producer;
int g_timeout_s_producer = 2;





int Producer::InitFdList()
{
    struct rlimit limit;
    int ret = 0;
    ret = getrlimit(RLIMIT_NOFILE, &limit);
    if (ret < 0 || limit.rlim_max == RLIM_INFINITY) {
        DEBUG_LOG("getrlimit failed");
        g_max_open_fd_producer = config_get_intval("max_open_fd", 100000);
    } else {
        g_max_open_fd_producer = limit.rlim_max;
    }

    DEBUG_LOG("set max_open_fd = %u", g_max_open_fd_producer);

    g_client_fd_list_producer = (list_head_t *)malloc(
            sizeof(list_head_t) * g_max_open_fd_producer);
    if (g_client_fd_list_producer == NULL) {
        ERROR_LOG("malloc fd list failed, error = %s",
                strerror(errno));
        return -1;
    }

    int i = 0;
    for (i = 0; i < g_max_open_fd_producer; i++) {
        INIT_LIST_HEAD(&g_client_fd_list_producer[i]);
    }

    return 0;
}

int Producer::Init()
{
    // 用户在客户端关闭时清理其请求
    if (InitFdList()) {
        ERROR_LOG("InitFdList failed");
        return -1;
    }

    // 初始化内存池，尽量支持10000级别的QPS
    g_mempool_producer = mempool_create(sizeof(ClientInfo), getpagesize() * 4096);
    if(!g_mempool_producer)
    {
        ERROR_LOG("create mempool failed");
        return -1;
    }
    
    // 初始化timeout列表
    g_timeout_s_producer = config_get_intval("timeout_s", 2);
    if (timer_init(&g_net_timer_producer)) {
        ERROR_LOG("timer init failed");
        return -1;
    }
    

    return 0;
}

int Producer::Uninit() 
{

    // client fd链表
    if (g_client_fd_list_producer) {
        free(g_client_fd_list_producer);
        g_client_fd_list_producer = NULL;
    }

    // 清理内存池
    if (g_mempool_producer) {
        mempool_destroy(g_mempool_producer);
        g_mempool_producer = NULL;
    }
    
    timer_uninit(&g_net_timer_producer);

    // 生产进程没有策略
	//if(strategy_handler) {
	//	strategy_handler->Uinit();
	//	delete strategy_handler;
	//}
    return 0;
}

int Producer::GetPkgLenSer(int fd, const char *buf, uint32_t len) 
{
    // 根据fd找到对应的ProtoHandler
    IAsyncProtoHandler *aph = GetAsyncProtoHandlerByFd(fd);
    if (!aph)
    {
        ERROR_LOG("Producer ProcPkgSer get NULL IAsyncProtoHandler");
        return -1;          // 会重启backend的连接
    }

    return aph->GetPkgLenSer(buf, len);
}

void Producer:: TimeEvent() 
{
	//time_t t = time(NULL);
	//bool ret;
	//if(!(t%5)) {
	//	ret = strategy_handler->Query(UpstreamProtoHandler::m_name_list, g_mempool_producer);
	//	if(ret == false) {
	//		ERROR_LOG_RP(RP_ERROR_CODE_NETWORK,"Query Startegy from center failed.");
	//	}
	//}

	//if(strategy_handler->Check(m_group)) {
    //    // 遍历map取reload
    //    map<int, AsyncProtoHandler>::iterator it = g_route_map.begin();
    //    for (; it != g_route_map.end(); ++it) {
    //        it->ReloadServiceGroup(m_group[it->first]);
    //    }
	//}

    // 重连断开的连接
    ReInitable::ProcessReInit();

//    // 处理超时
    timer_check_event(&g_net_timer_producer);
}

void Producer::SendToClient(int fd, uint32_t seq_num, uint32_t cmd_id, 
                         uint32_t status_code, uint32_t user_id)
{
    char out_buf[1024];
    ProtoHeader *out_header = (ProtoHeader*)out_buf;

    out_header->pkg_len = sizeof(ProtoHeader);
    out_header->seq_num = seq_num;
    out_header->cmd_id = cmd_id;
    out_header->status_code = status_code;
    out_header->user_id = user_id;

    net_send_cli(fd, out_buf, out_header->pkg_len);
}

void Producer::SendToClient(ClientInfo *ci, const char *buf, 
                        int len, uint32_t status_code)
{
    char out_buf[1024];
    ProtoHeader *out_header = (ProtoHeader*)out_buf;

    out_header->pkg_len = sizeof(ProtoHeader) + len;
    out_header->seq_num = ci->cli_seq;
    out_header->cmd_id = ci->cli_cmd;
    out_header->status_code = status_code;
    out_header->user_id = ci->userid;

    net_send_cli(ci->cli_fd, out_buf, out_header->pkg_len);
    DestroyClient(ci->ser_seq);
    return;
}

// 此进程负责接收数据，不负责同步数据

void Producer::ProcPkgCli(int fd, const char *buf, uint32_t len) 
{

    //此时一定Producer::Init()已经成功了

    DEBUG_LOG("Producer ProcPkgCli start");
    ProtoHeader *in_header = (ProtoHeader*)buf;
    

    // ClientInfo *ci = new (std::nothrow) ClientInfo(); // 后续从内存池中取
    // 注意要通过mempool_free(g_mempool_producer, ci)来释放
    ClientInfo *ci = (ClientInfo*)mempool_calloc(g_mempool_producer);

    if (!ci) {
        ERROR_LOG("Producer ProcPkgCli failed for get NULL from mempool");
        SendToClient(fd, in_header->seq_num, in_header->cmd_id, 
                1001, in_header->user_id);
        return;
    }

    // event_init(&ci->event);

    ci->cli_seq = in_header->seq_num;
    ci->cli_fd = fd;
    ci->cli_cmd = in_header->cmd_id;
    ci->userid = in_header->user_id;
    

    ClientInfo::SetClientInfo(ci);

    //uint32_t backend_sn = ClientInfo::SetClientInfo(ci);
    list_add_tail(&ci->fd_list_node,  &g_client_fd_list_producer[fd]);

    int game_id = *(int*)(buf + sizeof(ProtoHeader));
    in_header->pkg_len = in_header->pkg_len - sizeof(uint32_t);
    // for debug
    DEBUG_LOG("game_id is %d cmd_id is %d userid is %d, len is %d", 
                      game_id, ci->cli_cmd, ci->userid, in_header->pkg_len);
    // 1024应该足够了
    char message[1024] = {0};
    // 整个包进行拷贝，可以后续拓展
    memcpy(message, buf, sizeof(ProtoHeader));
    // 获得message
    memcpy(message + sizeof(ProtoHeader), 
                        buf + sizeof(ProtoHeader) + sizeof(uint32_t), 
                        in_header->pkg_len - sizeof(ProtoHeader));

    // 插入成功，给客户端一个回包, 插入失败，给客户端一个回包
    uint32_t status;
    do 
    {
        if ((InsertOneMessage(game_id, message, in_header->pkg_len - sizeof(int))) < 0)
        {
            status = DB_ERR;
            break;
        }
        status = 0;
    }while(0);
    Producer::SendToClient(ci, NULL, 0, status);

}

void Producer::DestroyClient(uint32_t sn)
{
    ClientInfo *ci = ClientInfo::GetClientInfo(sn);

    if (!ci) {
        ERROR_LOG("Producer DestroyClient failed for get NULL ClientInfo by sn %u", sn);
        return;
    }

    // 从fd链表中删除
    list_del_init(&ci->fd_list_node);

    // 退回内存池
    mempool_free(g_mempool_producer, ci);

    // 从unordered_map中删除
    ClientInfo::RemoveClientInfo(sn);
    
    // timeout中删除
    // 
    // timer_del_event(&ci->event);
}

IAsyncProtoHandler *Producer::GetAsyncProtoHandlerByFd(int fd)
{
    AsyncService *as = (AsyncService*)AsyncService::GetServiceByFd(fd);
    if (!as) {
        ERROR_LOG("Producer GetAsyncProtoHandlerByFd failed for GetServiceByFd failed, fd is %d", fd);
        return NULL;
    }

    IAsyncServiceGroup *asg = (IAsyncServiceGroup*)(as->GetParent()->GetParent());
    if (!as) {
        ERROR_LOG("Producer GetAsyncProtoHandlerByFd failed for get NULL IAsyncServiceGroup");
        return NULL;
    }

    IAsyncProtoHandler *aph = (IAsyncProtoHandler*)(asg->GetProtoHandler());
    if (!aph) {
        ERROR_LOG("Producer GetAsyncProtoHandlerByFd failed for get NULL ProtoHandler");
        return NULL;
    }
    
    return aph;
}

// 同步的service不会访问到这个函数，一定是异步的
void Producer:: ProcPkgSer(int fd, const char *buf, uint32_t len) 
{
    // 根据fd找到对应的ProtoHandler
    IAsyncProtoHandler *aph = GetAsyncProtoHandlerByFd(fd);
    if (!aph)
    {
        ERROR_LOG("Producer ProcPkgSer get NULL IAsyncProtoHandler");
        return;
    }

    uint32_t seq_num = 0;
    if (!aph->GetSeqNum(buf, len, seq_num)) {
        ERROR_LOG("Producer GetSeqNum failed");
        return;
    }

    ClientInfo *ci = ClientInfo::GetClientInfo(seq_num);

    if (!ci) {
        DEBUG_LOG("Producer ProcPkgSer get NULL ClientInfo, perhaps client closed ");
        return;
    }

    if (ci->fd_list_node.prev == &ci->fd_list_node &&
        ci->fd_list_node.next == &ci->fd_list_node) {
       	// 不应到此, 不涉及拉取策略
        //if (ci->cli_fd == 0) {
       	//    ERROR_LOG("Producer ProcPkgSer get ClientInfo not in fd list");
       	//    return;
        //}
    }

    aph->ProcPkgSer(buf, len);
    return;
}

void Producer:: LinkUpCli(int fd, uint32_t ip) 
{
    
}

void Producer:: LinkUpSer(int fd, uint32_t ip, uint16_t port) 
{
    DEBUG_LOG("Producer LinkUpSer fd is %d, ip is %u, port is %d", fd, ip, port);
    AsyncService::SetServiceConnected(fd);
}

void Producer:: LinkDownCli(int fd) 
{
    if (fd < 0 || fd >= g_max_open_fd_producer) {
        ERROR_LOG("Producer LinkDownCli recv invalid fd %d", fd);
        return;
    }

    ClientInfo *entry = NULL;
    ClientInfo *next = NULL;
    list_for_each_entry_safe(entry, next, &g_client_fd_list_producer[fd], fd_list_node, ClientInfo) {
        DEBUG_LOG("client close fd %d, cmd 0x%04X, userid %u", 
                  fd, entry->cli_cmd, entry->userid);
        DEBUG_LOG("start destroy");
        DestroyClient(entry->cli_seq);
    }
}

void Producer:: LinkDownSer(int fd) 
{
    DEBUG_LOG("Producer LinkDownSer fd is %d", fd);
    AsyncService::SetServiceDisconnected(fd);
}

int Producer::DoServerTimeout(void *owner, void *arg)
{
    //ClientInfo *ci = (ClientInfo*)owner;
    //if(ci->cli_cmd == QUERY_STRATEGY_CMD) {
	//	ERROR_LOG_RP(RP_ERROR_CODE_NETWORK,"Query Startegy from center timeout.");
    //    strategy_handler->TimeOutEvent(ci);
    //    return 0;
    //}

//	BdbAsyncProtoHandler *p = Singleton<BdbAsyncProtoHandler>::GetInstance();
//	if(p) {
//	    ERROR_LOG_RP(RP_ERROR_CODE_NETWORK,"Sync vip to game time out.");
//		p->TimeOutEvent(ci);
//	}
    return 0;
}












