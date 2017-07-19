#include "startegy_proto_handler.h"
#include "net_utils.h"
#include "common.h"
#include "singleton.h"
#include "client_info.h"
extern mempool_t* g_mempool;

bool StrategyProtoHandler::Init(vector<string> &name_list, map<string, ServiceGroupInfo> &group_list)
{
	//get ip_list
	if(!GetCenterIps(ip_list)){	
		ERROR_LOG("Init startegyHandler failed.");
		return false;
	}
	
	//Init proto_hander
	ServiceGroupInfo g = GetCenterGroup();
	StrategySyncProtoHandler *sync_handler = Singleton<StrategySyncProtoHandler>::GetInstance();
	 if (!sync_handler || !sync_handler->Init(g)) {
		 ERROR_LOG("Init StrategySyncProtoHandler failed.");
		 return false;
	 }

	m_service_group.clear();
	if(!sync_handler->SyncGetProcessIdentity(name_list, m_service_group)){
		ERROR_LOG("StrategyProtoHandler:SyncGetProcessIdentity failed.");
		return false;
	}

	if(m_service_group.size()) {
		group_list.clear();
		group_list = m_service_group;
	} else {
		ERROR_LOG("StrategyProtoHandler:service_group is empty.");
		return false;
	}

	StrategyAsyncProtoHandler *async_handler = Singleton<StrategyAsyncProtoHandler>::GetInstance();
	if (!async_handler || !async_handler->Init(g)) {
	    ERROR_LOG("Init StrategyAsyncProtoHandler failed.");
	    return false;
	}
	return true;
}

void StrategyProtoHandler::Uinit()
{
	StrategySyncProtoHandler *sync_handler = Singleton<StrategySyncProtoHandler>::GetInstance();
	if(sync_handler) {
		delete sync_handler;
	}

	StrategyAsyncProtoHandler *async_handler = Singleton<StrategyAsyncProtoHandler>::GetInstance();
	if(async_handler) {
		delete async_handler;
	}
}

bool StrategyProtoHandler::Query(vector<string> &name_list, mempool_t* g_mempool)
{
	vector<string> ips;
	if(!GetCenterIps(ips)) {
		ERROR_LOG("CheckStrategy failed:conn center failed.");
		return false;
	}
	
	if(ips.size() == 0) {
		ERROR_LOG("CheckStrategy failed: ip list is empyt.");
		return false;
	}
	
	if(ips != ip_list) {
		ip_list.clear();
		ip_list = ips;
	}

	ClientInfo *ci = (ClientInfo*)mempool_calloc(g_mempool);
    if (!ci) {
        ERROR_LOG("StrategyProtoHandler failed for get NULL from mempool");
        return false;
    }
	event_init(&ci->event);
    ci->cli_seq = 0;
    ci->cli_fd = 0;
    ci->cli_cmd = QUERY_STRATEGY_CMD;
    ci->userid = 0;
	uint32_t backend_sn = ClientInfo::SetClientInfo(ci);

	StrategyAsyncProtoHandler *async_handler = Singleton<StrategyAsyncProtoHandler>::GetInstance();
	if(!async_handler || !async_handler->AsyncGetProcessIdentity(ci, name_list, backend_sn)){
		ERROR_LOG("StrategyProtoHandler:AsyncGetProcessIdentity failed.");
		return false;
	}

	return true;
}

bool StrategyProtoHandler::Check(map<string, ServiceGroupInfo> &group_list)
{
	if(m_service_group == group_list) {
		return false;
	}

	m_service_group.clear();
	m_service_group = group_list;
	return true;
}

bool StrategyProtoHandler::GetCenterIps(vector<string> &ips)
{
    if(!Common::get_ip_by_host(center_name, ips)) {
        ERROR_LOG("Get ips failes from %s", center_name.c_str());
        return false;
    }
	return true;
}

ServiceGroupInfo StrategyProtoHandler::GetCenterGroup() 
{
	vector<HostInfo> list;
	HostInfo *hp = NULL;
	vector<string>::const_iterator it;
	for(it = ip_list.begin(); it != ip_list.end(); it++) {
		hp = new HostInfo(*it, center_port);
		list.push_back(*hp);
		delete hp;
	}

	ServiceReplicasInfo sri;
	sri.replicas_strategy = Strategy::STRATEGY_ROLL;
	for(unsigned int i = 0; i < list.size(); ++i) {
		sri.vec.push_back(list[i]);
	}

	ServiceGroupInfo sgi;
	sgi.group_strategy = Strategy::STRATETY_INT_REGION;
	sgi.vec.push_back(sri);
	return sgi;
}

void StrategyProtoHandler::TimeOutEvent(ClientInfo *ci)
{
    mempool_free(g_mempool, ci);
    ClientInfo::RemoveClientInfo(ci->ser_seq);                                                                                
    timer_del_event(&ci->event);
}
