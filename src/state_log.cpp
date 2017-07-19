#include "state_log.h"
#include "log.h"

uint32_t StateLog::fail_count_arr[MAX_GAME_ID];

uint32_t StateLog::suc_count_arr[MAX_GAME_ID];

int StateLog::cache_size_arr[MAX_GAME_ID];

extern std::vector<int> g_game_id_vec;

// 打印失败信息和流量信息
void StateLog::PrintStateInfo()
{
    std::vector<int>::iterator it = g_game_id_vec.begin();
    NOTI_LOG("----------------------failed--------------------------");
    for (; it != g_game_id_vec.end(); ++it)
    {
        NOTI_LOG("game_id : %d, failed_count : %d", *it, fail_count_arr[*it]);
        fail_count_arr[*it] = 0;
    }
    it = g_game_id_vec.begin();
    NOTI_LOG("----------------------success--------------------------");
    for (; it != g_game_id_vec.end(); ++it)
    {
        NOTI_LOG("game_id : %d, success_count : %d", *it, suc_count_arr[*it]);
        
        suc_count_arr[*it] = 0;

    }

}

// 记录一次失败次数
void StateLog::RecordFailCount(int game_id)
{
    fail_count_arr[game_id] ++;
}

// 记录一次成功次数
void StateLog::RecordSucCount(int game_id)
{
    suc_count_arr[game_id] ++;
}

// 记录队列长度
// 在每次遍历处理的时候进行统计
void StateLog::IncreaseCacheListSize(int game_id)
{
    // 遍历操作
    cache_size_arr[game_id] ++;
}

// TODO 增加对game_id范围的限制, 刚开始进行限制
int StateLog::GetCurrentCacheListSize(int game_id)
{
    return cache_size_arr[game_id];
}

void StateLog::ResetCacheListSize(int game_id)
{
    cache_size_arr[game_id] = 0;
}








