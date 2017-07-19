#ifndef _DAO_H
#define _DAO_H

#include <stdint.h>
#include <vector>
#include "i_mysql_iface.h"

#include "client_info.h"
// 进一步封装数据库
// 返回一套记录
// 依据id取一条记录

#define MAX_MESSAGE_LEN 255


int InsertOneMessage(int game_id, const char * message, uint32_t len);

int SelectOneMessageWithGameId(uint32_t last_sync_id, char * message, uint32_t & sync_id, int game_id);

// int SelectOneMessageWithStatus(int status, uint32_t & sync_id, char * message, int & game_id);

int SelectLastSyncId(int game_id, uint32_t & last_sync_id);

std::vector<int> GetGameIdVecWithFlag();

std::vector<int> GetGameIdVec();

int DeleteOneMessage(uint32_t sync_id, const char * table_name);

// 将数据从一个表移动到另一个表
//int MoveOneMessage(uint32_t sync_id, const char * table_name);

// 依据id更新一个表的状态，主要是status相关
int UpdateOneMessage(uint32_t sync_id, int status);

int UpdateReloadFlag(int game_id, int flag);

int UpdateLastSyncId(int game_id, uint32_t last_sync_id);

void FillMessage(int game_id, list_head_t * game_id_list, int num);

// 全量取失败的message
void FillFailMessage(list_head_t * game_id_list);

//int DeleteAndInsertOneMessage(uint32_t sync_id, int game_id, const char * message);

//int UpdateAndDeleteOneMessage(uint32_t sync_id, const char * message);

//int UpdateAndInsertOneMessage(uint32_t sync_id, int game_id, const char * message);



#endif
