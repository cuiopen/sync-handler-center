#ifndef _STATE_LOG_H_
#define _STATE_LOG_H_

/**
 * @brief 负责服务状态信息的一些日志，包括一些统计信息，每个小时统计发送成功失败信息，流量信息,
 * @数据成员为三个数组，每一个小时打印这三个数组的信息，需要遍历链表
 *
 */

#include <vector>

#include "proto.h"


class StateLog
{
public:
    // 打印日志信息,一小时执行一次
    static void PrintStateInfo();
    // 统计发送失败信息
    // 找打clientinfo,找到对应game_id, 添加统计项
    static void RecordFailCount(int game_id);
    // 统计流量
    static void RecordSucCount(int game_id);

    static void IncreaseCacheListSize(int game_id);

    // 清零操作 
    static void ResetCacheListSize(int game_id);
    
    static int  GetCurrentCacheListSize(int game_id);
    
private:
    
    static uint32_t fail_count_arr[MAX_GAME_ID];
    
    static uint32_t suc_count_arr[MAX_GAME_ID];

    static int cache_size_arr[MAX_GAME_ID];
};


#endif
