#ifndef _STRATEGTY_SYNC_HANDLER_H
#define _STRATEGTY_SYNC_HANDLER_H
#include "service_group.h"
#include "proto_handler.h"
#include "center_package.h"

class StrategySyncProtoHandler : public IntSyncProtoHandler, public CenterPackHandler
{
public:
	StrategySyncProtoHandler(){}
	~StrategySyncProtoHandler(){}

	bool SyncGetProcessIdentity(vector<string> &name_list, 
				map<string, ServiceGroupInfo> &group_list);
};

#endif
