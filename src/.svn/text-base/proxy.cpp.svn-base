#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include "proxy.h"
#include "async_server.h"
#include "log.h"
#include "monitor_api.h"
#include "service.h"
#include "client_info.h"
#include "mempool.h"
#include "proto.h"
#include "list.h"
#include "timer.h"
#include "startegy_proto_handler.h"
#include "center_proto.h"
#include "server.h"
#include "i_mysql_iface.h"
#include "dbproxy_proto_handler.h"
#include "switch_proto_handler.h"
#include "dao.h"
#include "route_mgr.h"
#include "cache_mgr.h"
#include "state_log.h"

mempool_t* g_mempool = NULL;
list_head_t *g_game_id_list = NULL;
int g_max_game_id = MAX_GAME_ID;
list_head_t g_net_timer;
// 默认八天后删除
int g_timeout_s = 8 * 24 * 3600;
int g_sync_num = 200;

map<string, ServiceGroupInfo> m_group;
vector<string> g_name_list;
vector<int> g_game_id_vec;
StrategyProtoHandler *strategy_handler = NULL;
// 路由处理
RouteMgr * g_route_mgr = NULL;
// 缓存管理
CacheMgr * g_cache_mgr = NULL;
GidHandlerMapDbproxy g_gid_handler_map_dbproxy;
GidHandlerMapSwitch g_gid_handler_map_switch;
// GidHandlerMap g_gid_handler_map_switch;
extern std::string table_name;
// 全局数组, 统计上次处理最大的sync_id(类似滑动窗口最左端)
uint32_t g_last_sync_id_arr[MAX_GAME_ID];
// 全局数组，统计上次处理最大的sync_id(类似滑动窗口最右端)
uint32_t g_last_sync_id_range_arr[MAX_GAME_ID];
// game_id 的list, 可以处理最大为1000的game_id,可调整


int Proxy::InitGameIdList()
{
    g_game_id_list = (list_head_t *)malloc(
            sizeof(list_head_t) * g_max_game_id);
    if (g_game_id_list == NULL) {
        ERROR_LOG("malloc game id list failed, error =  %s",
                strerror(errno));
        return -1;
    }
    int i = 0;
    for (i = 0; i < g_max_game_id; i++) {
        INIT_LIST_HEAD(&g_game_id_list[i]);
    }
    return 0;
}

int Proxy::Init()
{
	//初始化Strategy拉取策略	
	//name_list.push_back(vip_sync);
	strategy_handler = new StrategyProtoHandler("service-config.taomee.com", 19155);
    if (!strategy_handler) {
        ERROR_LOG("new StrategyProtoHandler failed");
        return -1;
    }

    if (InitGameIdList()) {
        ERROR_LOG("InitFdList failed");
        return -1;
    }
    
    // 初始化内存池，尽量支持10000级别的QPS
    g_mempool = mempool_create(sizeof(ClientInfo), getpagesize() * 4096);
    if(!g_mempool)
    {
        ERROR_LOG("create mempool failed");
        return -1;
    }
    
    // 初始化timeout列表
    g_timeout_s = config_get_intval("timeout_s", 8 * 24 * 3600);
    if (timer_init(&g_net_timer)) {
        ERROR_LOG("timer init failed");
        return -1;
    }
    // 初始化路由表
    g_route_mgr = new (std::nothrow) RouteMgr(g_gid_handler_map_dbproxy,
                                                       g_gid_handler_map_switch,
                                                       g_game_id_vec);
    if (g_route_mgr == NULL)
    {
        ERROR_LOG("route_mgr Init failed");
        return -1;
    }

    // TODO 最好用单例模式
    if (g_route_mgr->InitRouteMap() < 0)
    {
        ERROR_LOG("InitRouteMap failed");
        return -1;
    }
    
    g_cache_mgr = new (std::nothrow) CacheMgr(g_sync_num, 1);

    if (g_cache_mgr == NULL)
    {
        ERROR_LOG("InitCacheMgr failed");
        return -1;
    }
    

    // 初始化lasy_sync_id表
    std::vector<int>::iterator it = g_game_id_vec.begin();
    uint32_t last_sync_id = 0;
    for (it = g_game_id_vec.begin(); it != g_game_id_vec.end(); ++it)
    {

        if (SelectLastSyncId(*it, last_sync_id) < 0)
        {
            ERROR_LOG("get_last_sync_id failed");
            return -1;
        }
        g_last_sync_id_arr[*it] = last_sync_id;
        g_last_sync_id_range_arr[*it] = last_sync_id;
    }
 
    // 初始化发送失败表
    // g_game_id_list[0] 失败链表头结点
    // 意外重启的时候也不会丢失数据
    // 定时调用处理
    // 从db取出适当数据
    
    // g_game_id_list[0];
    // g_cache_mgr->FillFailList();

    return 0;
}

int Proxy::Uninit() 
{
    if (g_game_id_list) {
        free(g_game_id_list);
        g_game_id_list = NULL;
    }
    // 清理内存池
    if (g_mempool) {
        mempool_destroy(g_mempool);
        g_mempool = NULL;
    }

    timer_uninit(&g_net_timer);

	if(strategy_handler) {
		strategy_handler->Uinit();
		delete strategy_handler;
	}
    // 遍历处理函数
    GidHandlerMapDbproxy::iterator it = g_gid_handler_map_dbproxy.begin();
    for (; it != g_gid_handler_map_dbproxy.end(); ++it)
    {
        //it->second->Uninit();
        delete it->second;
        it->second = NULL;
    }
    // 析构路由表
    if (g_route_mgr) {
        delete g_route_mgr;
        g_route_mgr = NULL;
    }
    
    // 析构cache类
    if (g_cache_mgr) {
        delete g_cache_mgr;
        g_cache_mgr = NULL;
    }
    

    return 0;
}

int Proxy::GetPkgLenSer(int fd, const char *buf, uint32_t len) 
{
    // 根据fd找到对应的ProtoHandler
    IAsyncProtoHandler *aph = GetAsyncProtoHandlerByFd(fd);
    if (!aph)
    {
        ERROR_LOG("Proxy ProcPkgSer get NULL IAsyncProtoHandler");
        return -1;          // 会重启backend的连接
    }
    return aph->GetPkgLenSer(buf, len);
}

void Proxy:: TimeEvent() 
{
    // 定时重连
    ReInitable::ProcessReInit();
    
    // 可以执行过期数据的删除,任务相对较轻，考虑处理其他任务
    // 同步可以不做处理
    // 
    timer_check_event(&g_net_timer);
    // 更新内存的配置
    strategy_handler->Query(g_name_list, g_mempool);
    // 定期任务
    // 处理同步逻辑
    // 此为同步服务主要业务逻辑
    ServerCron();
}

void Proxy::SendToClient(int fd, uint32_t seq_num, uint32_t cmd_id, 
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

void Proxy::SendToClient(ClientInfo *ci, const char *buf, 
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
    // 不应该删除,只有当成功确认后，再执行删除
    // DestroyClient(ci->ser_seq);
    return;
}

void Proxy::ProcPkgCli(int fd, const char *buf, uint32_t len) 
{
    // 非协议驱动
}

void Proxy::DestroyClient(uint32_t sn)
{
    ClientInfo *ci = ClientInfo::GetClientInfo(sn);

    if (!ci) {
        ERROR_LOG("Proxy DestroyClient failed for get NULL ClientInfo by sn %u", sn);
        return;
    }
    // 将buf重新置为0
    memset(ci->buf, 0, 1024);
    // 从fd链表中删除
    // list_del_init(&ci->fd_list_node);
    // 从game_id链表中删除
    list_del_init(&ci->game_id_list_node);
    // 退回内存池
    mempool_free(g_mempool, ci);
    // 从unordered_map中删除, 序列号和clientinfo
    ClientInfo::RemoveClientInfo(sn);
    // 从timeout中删除
    timer_del_event(&ci->event);
}

IAsyncProtoHandler *Proxy::GetAsyncProtoHandlerByFd(int fd)
{
    AsyncService *as = (AsyncService*)AsyncService::GetServiceByFd(fd);
    if (!as) {
        ERROR_LOG("Proxy GetAsyncProtoHandlerByFd failed for GetServiceByFd failed, fd is %d", fd);
        return NULL;
    }

    IAsyncServiceGroup *asg = (IAsyncServiceGroup*)(as->GetParent()->GetParent());
    if (!as) {
        ERROR_LOG("Proxy GetAsyncProtoHandlerByFd failed for get NULL IAsyncServiceGroup");
        return NULL;
    }

    IAsyncProtoHandler *aph = (IAsyncProtoHandler*)(asg->GetProtoHandler());
    if (!aph) {
        ERROR_LOG("Proxy GetAsyncProtoHandlerByFd failed for get NULL ProtoHandler");
        return NULL;
    }
    
    return aph;
}

// 同步的service不会访问到这个函数，一定是异步的
void Proxy::ProcPkgSer(int fd, const char *buf, uint32_t len) 
{
    // 根据fd找到对应的ProtoHandler
    IAsyncProtoHandler *aph = GetAsyncProtoHandlerByFd(fd);
    if (!aph)
    {
        ERROR_LOG("Proxy ProcPkgSer get NULL IAsyncProtoHandler");
        return;
    }

    uint32_t seq_num = 0;
    if (!aph->GetSeqNum(buf, len, seq_num)) {
        ERROR_LOG("Proxy GetSeqNum failed");
        return;
    }

    ClientInfo *ci = ClientInfo::GetClientInfo(seq_num);

    if (!ci) {
        DEBUG_LOG("Proxy ProcPkgSer get NULL ClientInfo, perhaps client closed ");
        return;
    }

    if (ci->fd_list_node.prev == &ci->fd_list_node &&
        ci->fd_list_node.next == &ci->fd_list_node) {
       	// 当fd的链表不存在，或者客户端断开，会进入这里
        if (ci->cli_fd == 0) {
       	    ERROR_LOG("Proxy ProcPkgSer get ClientInfo not in fd list");
       	    return;
        }
    }

    aph->ProcPkgSer(buf, len);
    return;
}

void Proxy::LinkUpCli(int fd, uint32_t ip) 
{
}

void Proxy::LinkUpSer(int fd, uint32_t ip, uint16_t port) 
{
    DEBUG_LOG("Proxy LinkUpSer fd is %d, ip is %u, port is %d", fd, ip, port);
    AsyncService::SetServiceConnected(fd);
}

void Proxy::LinkDownCli(int fd) 
{
    // 非协议驱动
}
void Proxy::LinkDownSer(int fd) 
{
    DEBUG_LOG("Proxy LinkDownSer fd is %d", fd);
    AsyncService::SetServiceDisconnected(fd);
}
int Proxy::DoServerTimeout(void *owner, void *arg)
{
    DEBUG_LOG("handler_timeout");
    ClientInfo *ci = (ClientInfo*)owner;
    if(ci->cli_cmd == QUERY_STRATEGY_CMD) {
		ERROR_LOG_RP(RP_ERROR_CODE_NETWORK,"Query Startegy from center timeout.");
        strategy_handler->TimeOutEvent(ci);
        return 0;
    }
    // 把一个消息状态置为2
    // 表示未必处理，让滑动窗口跳过此消息
    DEBUG_LOG("the message has not been  dealed for 8 days, set status = 2, game_id is %d", ci->game_id);
    if ((UpdateOneMessage(ci->sync_id, 2)) < 0)
    {
        ERROR_LOG("UpdateOneMessage status failed");
    }
    return 0;
}
void Proxy::ServerCron()
{
    time_t now = time(NULL);
    std::vector<int>::iterator it = g_game_id_vec.begin();  
    // 5s执行一次,查看是否需要reload
    if (!(now%5))
    {
        // 检查是否需要reload,通过reload_flag
        // check and reload
        g_route_mgr->ModifyRouteMap();
    }
    // 3秒执行一次，装填
    // 此处理会清空上次的包
    if (!(now%3))
    {
        g_cache_mgr->FillAllList();
    }
    // 3s执行一次
    if (!(now%3))
    {
        // g_cache_mgr->PopOneEachList(now);
        g_cache_mgr->TraverseList(now);
    }

    //2h执行一次
    //if (!(now%3600))
    //{
    //   // 打印信息
    //   StateLog::PrintStateInfo();
    //}
    //半小时一次,搜集状态信息
    //成功次数，发送次数，流量信息
}
