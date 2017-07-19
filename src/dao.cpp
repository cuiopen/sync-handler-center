#include "dao.h"
// #include "i_mysql_iface.h"
#include "async_server.h"
#include "log.h"
#include "mempool.h"


#include <string.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <time.h>


#define DATA_LEN 254


extern i_mysql_iface * g_mysql_handler;
extern mempool * g_mempool;
extern uint32_t g_last_sync_id_arr[MAX_GAME_ID];
extern uint32_t g_last_sync_id_range_arr[MAX_GAME_ID];

// db_err 为数据库出错1002
// table_name 基本固定
extern const char * g_sync_table_name;
extern const char * g_last_sync_table_name;



int InsertOneMessage(int game_id, const char * message, uint32_t message_len)
{
    int rv = 0;
    //char data[1024] = {0};
    char sql[1024] = {0};
    MYSQL * sync_db = NULL;
    char * end = sql;
    if (!(sync_db = g_mysql_handler->get_conn()))
    {
        ERROR_LOG("mysql_iface:get_conn() failed, err: %s", g_mysql_handler->get_last_errstr());
        return -1;
    }
    //if (!(strncasecmp(table_name, "vip_sync_table", 14))) 
    //    sprintf(sql, "INSERT INTO %s (game_id, message) VALUES (%d, ", table_name, game_id);
    //else 
    //    sprintf(sql, "INSERT INTO %s (sync_id, game_id, message) VALUES (%d, %d, ", table_name, sync_id, game_id);
    
    uint32_t size = sprintf(sql, "INSERT INTO %s (game_id, message) VALUES (%d, ", g_sync_table_name, game_id);
    DEBUG_LOG("size %d", size);
    end = sql + strlen(sql);
    *end++ = '\'';
    end += mysql_real_escape_string(sync_db, sql + strlen(sql), message, message_len);
    // DEBUG_LOG("end %u", mysql_real_escape_string(sync_db, sql + strlen(sql), message, message_len));
    *end++ = '\'';
    *end++ = ')';
    if ((rv = mysql_real_query(sync_db, sql, uint32_t(end - sql))) < 0)
    {
        //ERROR_LOG("mysql_iface:execsql() failed, err: %s", g_mysql_handler->get_last_errstr());
        return -1;
    }
    return 0;
}

int SelectOneMessageWithGameId(uint32_t last_sync_id, char * message, uint32_t & sync_id, int game_id)
{
    int rv = 0;
    int ret = 0;
    MYSQL_ROW row = NULL;
    //unsigned long message_len;
    //unsigned long * row_len;
    //std::vector<std::pair<uint32_t, char[255]> > 
    do
    {
        if ((rv = g_mysql_handler->select_first_row(&row, 
                            "SELECT sync_id, message FROM %s WHERE game_id = %d AND status = 0 AND sync_id > %d ORDER BY sync_id limit 100", 
                            g_sync_table_name, game_id, last_sync_id)) < 0)
        {
            ERROR_LOG("mysql_iface:select_first_row() failed, err: %s", g_mysql_handler->get_last_errstr());
            // 不需要回复状态
            ret = -1;
            break;
        } 
        else 
        {
            DEBUG_LOG(" game_id %d has been dealed", game_id);
        }
        if (!row)
        {
            ERROR_LOG("mysql_iface:row = NULL");
            ret = -1;
            break;
        }
        
        for (int i = 0; i < rv; ++i)
        {
            if (row[0]) sync_id = atoi(row[0]);
            if (row[1]) memcpy(message, row[1], 255);
            row = g_mysql_handler->select_next_row(false);
        }
        ret = 0;
    }while(0);  
    return ret;
}

int SelectLastSyncId(int game_id, uint32_t & last_sync_id)
{
    int rv = 0;
    int ret = 0;
    MYSQL_ROW row = NULL;
    do
    {
        if ((rv = g_mysql_handler->select_first_row(&row, 
                            "SELECT last_sync_id FROM %s WHERE game_id = %d", g_last_sync_table_name,  game_id)) < 0)
        {
            ERROR_LOG("mysql_iface:select_first_row() failed, err: %s", g_mysql_handler->get_last_errstr());
            // 不需要回复状态
            ret = -1;
            break;
        } 
        else 
        {
            // do nothing
        }
        if (!row)
        {
            ERROR_LOG("mysql_iface:row = NULL");
            ret = -1;
            break;
        }
        if (row[0]) last_sync_id = atoi(row[0]);
        
    }while(0);  
    return ret;
}


//int SelectOneMessageWithStatus(int status, uint32_t & sync_id, char * message, int & game_id)
//{
//    
//    int rv = 0;
//    int ret = 0;
//    MYSQL_ROW row = NULL;
//    // unsigned long message_len;
//    char sql[1024] = {0};
//    if (!strncasecmp(table_name, "sync_table", 10))
//    {
//        sprintf(sql, "SELECT game_id, sync_id, message FROM %s WHERE status = %d ORDER BY sync_id LIMIT 1", g_sync_table_name, status);
//    } else {
//        sprintf(sql, "SELECT game_id, sync_id, message FROM %s ORDER BY sync_id LIMIT 1", g_sync_table_name);
//    }
//    do
//    {
//        if ((rv = g_mysql_handler->select_first_row(&row, sql)) < 0)
//        {
//            ERROR_LOG("mysql_iface:select_first_row() failed, err: %s", g_mysql_handler->get_last_errstr());
//            ret = -1;
//            break;
//        } else if (rv != 1)
//        {
//            ERROR_LOG("mysql_iface:select_first_row() failed, affected_row = %d", ret);
//            ret = -1;
//            break;
//        } else {
//            // do nothing
//        }
//        if (!row)
//        {
//            ERROR_LOG("mysql_iface:row = NULL");
//            ret = -1;
//            break;
//        }
//        if (row[0]) game_id = atoi(row[0]);
//        if (row[1]) sync_id = atoi(row[1]);
//        if (row[2]) memcpy(message, row[2], sizeof(message));
//        ret = 0;
//    }while(0);   
//
//    return ret;
//}   

std::vector<int> GetGameIdVecWithFlag()
{
    int rv = 0;
    std::vector<int> game_id_vec;
    MYSQL_ROW row = NULL;
    do
    {
        if ((rv = g_mysql_handler->select_first_row(&row, 
                            "SELECT game_id from %s where reload_flag = 1", g_last_sync_table_name)) < 0)
        {
            ERROR_LOG("mysql_iface:select_first_row() failed, err: %s", g_mysql_handler->get_last_errstr());
            // 不需要回复状态
            break;
        } 
        else 
        {
        }
        if (!row)
        {
            // DEBUG_LOG("mysql_iface:row = NULL");
            break;
        }
        for (int i = 0; i < rv; ++i)
        {
            if (row[0]) game_id_vec.push_back(atoi(row[0]));
            row = g_mysql_handler->select_next_row(false);
        }
    }while(0);  
    return game_id_vec;
}



std::vector<int> GetGameIdVec()
{
    int rv = 0;
    std::vector<int> game_id_vec;
    MYSQL_ROW row = NULL;
    do
    {
        if ((rv = g_mysql_handler->select_first_row(&row, 
                            "SELECT game_id FROM %s", g_last_sync_table_name)) < 0)
        {
            ERROR_LOG("mysql_iface:select_first_row() failed, err: %s", g_mysql_handler->get_last_errstr());
            // 不需要回复状态
            break;
        }
        else 
        {
        }
        if (!row)
        {
            ERROR_LOG("mysql_iface:row = NULL");
            break;
        }
        for (int i = 0; i < rv; ++i)
        {
            if (row[0]) game_id_vec.push_back(atoi(row[0]));
            row = g_mysql_handler->select_next_row(false);
        }
    }while(0);  
    return game_id_vec;
}

int UpdateOneMessage(uint32_t sync_id, int status)
{
    int rv = 0;
    int ret = 0;
    time_t now = time(NULL);
    char update_time[26] = {0};
    tm * p_now = localtime(&now);
    strftime(update_time, 26, "\'%Y-%m-%d %H:%M:%S\'", p_now);
    do 
    {
        if ((rv = g_mysql_handler->execsql("UPDATE %s SET status = %d,\
                                            finish_time = %s\
                                            WHERE sync_id = %d", 
                                            g_sync_table_name, 
                                            status, 
                                            update_time, 
                                            sync_id)) < 0)
        {
            ERROR_LOG("mysql_iface:execsql() failed, err: %s", g_mysql_handler->get_last_errstr());
            ret = -1;
            break;
        }
    }while(0);
    return ret;
}


int UpdateReloadFlag(int game_id, int flag)
{
    int rv = 0;
    int ret = 0;
    do 
    {
        if ((rv = g_mysql_handler->execsql("UPDATE %s SET reload_flag = %d WHERE game_id = %d", g_last_sync_table_name, flag, game_id)) < 0)
        {
            ERROR_LOG("mysql_iface:execsql() failed, err: %s", g_mysql_handler->get_last_errstr());
            ret = -1;
            break;
        }
    }while(0);
    return ret;
}



int UpdateLastSyncId(int game_id, uint32_t last_sync_id)
{
    int rv = 0;
    int ret = 0;
    do 
    {
        if ((rv = g_mysql_handler->execsql("UPDATE %s SET last_sync_id = %d WHERE game_id = %d", 
                                                            g_last_sync_table_name, last_sync_id, game_id)) < 0)
        {
            ERROR_LOG("mysql_iface:execsql() failed, err: %s", g_mysql_handler->get_last_errstr());
            ret = -1;
            break;
        }
    }while(0);
    return ret;
}


int DeleteOneMessage(uint32_t sync_id, const char * table_name)
{
    int rv = 0;
    int ret = 0;
    do 
    {
        if ((rv = g_mysql_handler->execsql("DELETE FROM %s where sync_id = %d", table_name, sync_id)) < 0)
        {
            ERROR_LOG("mysql_iface:execsql() failed, err: %s", g_mysql_handler->get_last_errstr());
            ret = -1;
            break;
        }
    }while(0);
    return ret;
}

void FillMessage(int game_id, list_head_t * game_id_list, int num)
{
    MYSQL_ROW row;
    int ret = 0;
    ret = g_mysql_handler->select_first_row(&row,
                             "SELECT sync_id, message, UNIX_TIMESTAMP(sync_time) FROM %s\
                              WHERE game_id = %d AND status = 0\
                              AND sync_id > %d ORDER BY sync_id limit %d",
                             g_sync_table_name, game_id, g_last_sync_id_arr[game_id], num);
    if (ret < 0) {
        ERROR_LOG("mysql_iface:select_first_row() failed, err: %s", 
                         g_mysql_handler->get_last_errstr());
        return;
    } else if (ret == 0) {
        DEBUG_LOG("game_id %d is empty in database", game_id);
        // 处理后面的游戏
        return;
    }
    int message_num = ret;
    for (int i = 0; i < message_num; ++i)
    {
          ClientInfo * ci = (ClientInfo*)mempool_calloc(g_mempool);
          if (!ci) {
              ERROR_LOG("Producer ProcPkgCli failed for get NULL from mempool");
              return;
          }
          ci->Init(game_id, atoi(row[0]), row[1], atoi(row[2]), game_id_list);
          // 置一下last_sync_id的值
          g_last_sync_id_range_arr[game_id] = atoi(row[0]);
          DEBUG_LOG("last_sync_id_range %d", g_last_sync_id_range_arr[game_id]);     
          row = g_mysql_handler->select_next_row(false);
    }
}


void FillFailMessage(list_head_t * game_id_list)
{
    MYSQL_ROW row;
    int ret = 0;
    ret = g_mysql_handler->select_first_row(&row,
                             "SELECT sync_id, game_id, message, UNIX_TIMESTAMP(sync_time) FROM sync_table\
                              WHERE status = 0");
    if (ret < 0) {
        ERROR_LOG("mysql_iface:select_first_row() failed, err: %s", 
                         g_mysql_handler->get_last_errstr());
        return;
    } else if (ret == 0) {
        DEBUG_LOG("no message failed last time");
        // 处理后面的游戏
        return;
    }
    int message_num = ret;
    for (int i = 0; i < message_num; ++i)
    {
          ClientInfo * ci = (ClientInfo*)mempool_calloc(g_mempool);
          if (!ci) {
              ERROR_LOG("Producer ProcPkgCli failed for get NULL from mempool");
              return;
          }
          // 
          ci->Init(atoi(row[1]), atoi(row[0]), row[2], atoi(row[3]), game_id_list);
          // 置一下last_sync_id的值
          // g_last_sync_id_arr[game_id] = atoi(row[0]);
          // DEBUG_LOG("last_sync_id %d", g_last_sync_id_arr[game_id]);     
          row = g_mysql_handler->select_next_row(false);
    }
}


