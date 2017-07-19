#ifndef _ROUTE_MGR_H_
#define _ROUTE_MGR_H_

#include <vector>
#include "proxy.h"

/**
 * @brief 管理路由表的初始化，和更新操作
 *  
 * @para  管理game_id的全局数组，strategy指针，异步拉取ip port  
 * 避免使用extern ,尽量传参
 */




class RouteMgr
{
public:
    
    RouteMgr(GidHandlerMapDbproxy & gid_handler_map_dbproxy,
             GidHandlerMapSwitch & gid_handler_map_switch,
             std::vector<int> & game_id_vec) : m_gid_handler_map_dbproxy(gid_handler_map_dbproxy),
                                               m_gid_handler_map_switch(gid_handler_map_switch),
                                               m_game_id_vec(game_id_vec)
    {}

    ~RouteMgr()
    {
        UninitRouteMap();   
    }

    // 初始化路由表
    int InitRouteMap();
    // 路由表反初始化
    int UninitRouteMap();
    // 修改路由表,异步去拉取配置，避免影响其他正常服务
    int ModifyRouteMap();
    // init protohandler
    int InitProtoHandler(int game_id);

private:

    // dbproxy的路由表
    GidHandlerMapDbproxy & m_gid_handler_map_dbproxy;
    
    // switch的路由表
    GidHandlerMapSwitch & m_gid_handler_map_switch;

    // game_id的数组
    std::vector<int> & m_game_id_vec;
 
    GidHandlerMapDbproxy::iterator m_gid_it_dbproxy;

    GidHandlerMapSwitch::iterator m_gid_it_switch;

    std::pair<GidHandlerMapSwitch::iterator, GidHandlerMapSwitch::iterator> m_ret; 


    std::vector<int> m_game_id_vec_reload;

    std::vector<int>::iterator m_reload_it;



};



#endif 
