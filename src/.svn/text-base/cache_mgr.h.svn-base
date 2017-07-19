#ifndef _CACHE_MGR_H_
#define _CACHE_MGR_H_

/**
 * @brev 管理内存缓冲区，维护一个game_id的链表，提供链表的遍历，pop(), 和 push()操作
 * 里面的方法会被定时任务调度，两种处理模式，为vip模式，和兑换系统模式
 *
 */
#include <vector>
#include <stdint.h>
#include <time.h>

class CacheMgr
{
public:
    CacheMgr(int sync_num, int type) : m_sync_num(sync_num),
                                       m_type(type)
    {}
    ~CacheMgr(){}
    
    // 遍历处理 兑换系统模式
    void TraverseList(time_t now);
    // 装填
    void FillAllList();
    
    void FillFailList();
    // 取出每个头结点进行处理 vip模式
    // 两种处理模式
    void PopOneEachList(time_t now);
    
    void TraverseFailList(time_t now);

    void ClearList(int game_id);

private:
    // 初始化cache的大小，默认为100
    int m_sync_num; 
    // type为1 vip模式，type为2 交易系统模式,暂时两份代码，避免交叉影响
    int m_type;
    
    // std::vector<int> m_game_id_vec;

    std::vector<int>::iterator m_it;
};
#endif
