#include "startegy_sync_handler.h"
#include "center_proto.h"
#include <vector>
#include <string>
#include <map>
#include <string.h>
using std::vector;
using std::string;
using std::map;

bool StrategySyncProtoHandler::SyncGetProcessIdentity(vector<string> &name_list,
		map<string, ServiceGroupInfo> &group_list)
{
	char in_buf[4096];
	memset(in_buf, 0, sizeof(proto_header));
	proto_header *in_header = (proto_header*)in_buf;
 	ReqPack(in_buf, sizeof(in_buf), name_list);

	char out_buf[MAX_BUF_LEN];
	memset(out_buf, 0, sizeof(out_buf));
	size_t out_len;

	if (!SendAndRecvByInt(in_buf, in_header->pkg_len, out_buf, sizeof(out_buf), out_len, 1)) {
		ERROR_LOG("StartegySyncProtoHandler SyncGetProcessIdentity failed for SendAndRecvByInt failed");
		return false;
	}

	proto_header *out_header = (proto_header*)out_buf; 
	if (out_len == sizeof(proto_header) || out_header->status_code || out_len == 0) {
		ERROR_LOG("Get Startegy failed.");
		return false;
	}

	if(!AckPack(out_buf, out_len, group_list)) {
		ERROR_LOG("Get Startegy failed.");
		return false;
	}

	return true;
}
