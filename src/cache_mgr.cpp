#include "cache_mgr.h"
#include "proxy.h"
#include "log.h"
#include "list.h"
#include "dao.h"
#include "client_info.h"
#include "state_log.h"

#include "monitor_api.h"

// 商品兑换模式
// 遍历处理每一个clientinfo
extern std::vector<int> g_game_id_vec;
extern list_head_t * g_game_id_list;
// extern int g_max_cache_size;
extern uint32_t g_last_sync_id_arr[MAX_GAME_ID];
extern uint32_t g_last_sync_id_range_arr[MAX_GAME_ID];


void CacheMgr::FillAllList()
{
    m_it = g_game_id_vec.begin();
    for (; m_it != g_game_id_vec.end(); ++m_it)  
    {
        
        // 如果链表不为空，表示有剩余没有发送成功的
        if (!list_empty(&g_game_id_list[*m_it]))
        {
            ERROR_LOG_RP(RP_ERROR_CODE_NETWORK, "product sync failed game_id is %d", *m_it);
            // 清空链表
            ClearList(*m_it);
        } else {
            // 链表为空，表示已经处理完这一批次，可进入下一个批次
            // 对last_sync_id进行更新
            g_last_sync_id_arr[*m_it] = g_last_sync_id_range_arr[*m_it];
            // 更新lasy_sync_id
            if ((UpdateLastSyncId(*m_it, g_last_sync_id_arr[*m_it])) < 0)
            {
                ERROR_LOG("UpdateLastSyncId failed, game_id %d", *m_it);
                return;
            }
        }
        
        FillMessage(*m_it, &g_game_id_list[*m_it], m_sync_num);
    }
}

void CacheMgr::FillFailList()
{
    FillFailMessage(&g_game_id_list[0]);
}

void CacheMgr::ClearList(int game_id)
{
    ClientInfo *entry = NULL;
    ClientInfo *next = NULL;
    list_for_each_entry_safe(entry, next, &g_game_id_list[game_id], game_id_list_node, ClientInfo)
    {
        Proxy::DestroyClient(entry->ser_seq);
    }
}


void CacheMgr::TraverseList(time_t now)
{
    m_it = g_game_id_vec.begin();
    for (; m_it != g_game_id_vec.end(); ++m_it)
    {
        ClientInfo *entry = NULL;
        ClientInfo *next = NULL;
        // StateLog::ResetCacheListSize(*m_it);
        list_for_each_entry_safe(entry, next, &g_game_id_list[*m_it], game_id_list_node, ClientInfo) 
        {
            // 记录一些统计数据
            // StateLog::IncreaseCacheListSize(*m_it);
            entry->DealMessage(now);
        }
    }
}

void CacheMgr::TraverseFailList(time_t now)
{
    ClientInfo *entry = NULL;
    ClientInfo *next = NULL;
    // StateLog::ResetCacheListSize(0);
    list_for_each_entry_safe(entry, next, &g_game_id_list[0], game_id_list_node, ClientInfo) 
    {
        // 记录一些统计数据, 定时报警
       //  StateLog::IncreaseCacheListSize(0);
        entry->DealMessage(now);
    }
}

void CacheMgr::PopOneEachList(time_t now)
{
    m_it = g_game_id_vec.begin();
    for (; m_it != g_game_id_vec.end(); ++m_it)
    {
        ClientInfo * entry = NULL;
        list_head_t * node = NULL;

        //entry = list_entry(&g_game_id_list[*m_it], ClientInfo, game_id_list_node);
        while((node = (&g_game_id_list[*m_it])->next) != &g_game_id_list[*m_it])
        {
            entry = list_entry(node, ClientInfo, game_id_list_node);
            if (entry->DealMessage(now) < 0) 
            {
                ERROR_LOG("entry DealMessage failed");
                break;
            }
            // 发送swithch, 这里switch也用同步可能会导致阻塞，switch必须异步
        }
    }
}

