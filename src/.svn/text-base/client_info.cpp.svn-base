#include "client_info.h"

#include "proxy.h"
#include "state_log.h"

extern GidHandlerMapDbproxy g_gid_handler_map_dbproxy;

ClientInfo::ClientInfoMap ClientInfo::client_info_map;
uint32_t ClientInfo::serialized_no = 0;

/**
 * @获取超时时间,第一个100次，5S，第二个100次， 60S, 以上一小时
 * para send_times, 发送次数
 * para type        策略类型,暂时没用
 * 设置5s为了避免同一个消息被重复处理的情况,后期从配置文件
 */


// 通过配置文件
uint32_t ClientInfo::GetNextInterval(uint32_t sync_time, time_t now) {
    if (now - sync_time < 500)
        return 5;
    else if(now - sync_time >= 500 && now - sync_time < 5000)
        return 10;
    else
        return 60;
}

/**
 * @初始化clientinfo,用message和sync_id初始化
 * @para 链表头结点和game_id和last_sync_id
 * @直接调用mysql的api进行操作，后续会调整
 */
int ClientInfo::Init(int game_id, uint32_t sync_id, const char * message, time_t timestamp, list_head_t * game_id_list)
{
    // 每个clientinfo有固定的生存周期
    event_init(&this->event);
    this->sync_id = sync_id;
    ProtoHeader * header = (ProtoHeader *)message;
    memcpy(this->buf, message, header->pkg_len);
    // for debug
    DEBUG_LOG("len %u, userid %u game_id %d", header->pkg_len, header->user_id, game_id);
    this->userid = header->user_id;
    this->game_id = game_id;
    this->send_times = 0;
    this->pkg_len = header->pkg_len;// len是包含game_id长度的，此处需要去掉这个长度
    // 初始化为0
    this->last_send_time = 0;
    this->sync_time = timestamp;       
    uint32_t backend_sn = SetClientInfo(this);
    DEBUG_LOG("backen_sn is %u", backend_sn);
    list_add_tail(&this->game_id_list_node, game_id_list);
    return 0;
}

/**
 * @遍历message的链表处理业务逻辑
 * @para game_id_handler_map
 */
int ClientInfo::DealMessage(time_t now) {
    if (now < GetNextInterval(sync_time, now) + last_send_time)
    {
        // 第一次发送
        DEBUG_LOG("interval %u, last_send_time %u", GetNextInterval(sync_time, now), last_send_time);
        // DEBUG_LOG("the message is during the interval");
        return -1;
    }
    // 这里必定可以找到, 后面加上一些定时更新的逻辑 
    // 需要相加的业务逻辑

    last_send_time = now;
    GidHandlerMapDbproxy::iterator it = g_gid_handler_map_dbproxy.begin();
    it = g_gid_handler_map_dbproxy.find(game_id);
    if (it == g_gid_handler_map_dbproxy.end())
    {
        ERROR_LOG("gid_handler_map find no gamd_id, can not be here");
        return -1;
    }
    // 发送dbproxy
    if (!it->second->ProcessIdentity(this, this->ser_seq, userid))
    {
        ERROR_LOG("Dbproxy processidentity failed");
        return -1;
    }
    // 记录发送成功次数
    // StateLog::RecordSucCount(game_id);
    return 0;
}


uint32_t ClientInfo::GetSerializeNo() {
    return serialized_no++;
}

ClientInfo* ClientInfo::GetClientInfo(uint32_t sn) {
    if (client_info_map.find(sn) != client_info_map.end()){
        return client_info_map[sn];
    }
    return NULL;
}

uint32_t ClientInfo::SetClientInfo(ClientInfo *ci) {
    uint32_t sn = GetSerializeNo();
    ci->ser_seq = sn;
    client_info_map[sn] = ci;
    return sn;
}

void ClientInfo::RemoveClientInfo(uint32_t sn) {
    client_info_map.erase(sn);
}




