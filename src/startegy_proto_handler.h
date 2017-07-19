#ifndef _STRATEGY_PROTO_HANDLER_H
#define _STRATEGY_PROTO_HANDLER_H
#include "startegy_sync_handler.h"
#include "startegy_async_handler.h"
#include "client_info.h"
#include "mempool.h"
#include "center_proto.h"
#include "proxy.h"
#include <vector>
#include <string>
using std::vector;
using std::string;

class StrategyProtoHandler
{
	vector<string> ip_list;
	map<string, ServiceGroupInfo> m_service_group;

	string center_name;
	int center_port;
public:
	StrategyProtoHandler(string name, int port):center_name(name), center_port(port){}
	bool Init(vector<string> &name_list, map<string, ServiceGroupInfo> &group_list);
	void Uinit();
	bool Query(vector<string> &name_list, mempool_t* g_mempool);
	//如果改变了返回true，未改变返回false
	bool Check(map<string, ServiceGroupInfo> &group_list);
	void TimeOutEvent(ClientInfo *ci);

private:
	bool GetCenterIps(vector<string> &ips);
	ServiceGroupInfo GetCenterGroup();
};

#endif
