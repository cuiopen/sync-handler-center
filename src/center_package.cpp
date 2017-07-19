#include "center_package.h"
#include "center_proto.h"
#include "net_utils.h"
#include "async_server.h"
#include "log.h"
#include <string.h>

bool CenterPackHandler::ReqPack(char*in_buf, int buf_len, const vector<string> &list)
{
	memset(in_buf, 0, sizeof(proto_header));
	proto_header *in_header = (proto_header*)in_buf;
	in_header->pkg_len = sizeof(proto_header) + sizeof(serv_query_group_req) + (list.size() * sizeof(name_info));
	in_header->seq_num = 0;
	in_header->cmd_id = QUERY_STRATEGY_CMD;
	in_header->status_code = 0;
	in_header->user_id = 0;

	serv_query_group_req *in_body = (serv_query_group_req*)(in_header + 1);
	memset(in_body, '\0', buf_len - sizeof(proto_header));

	string local_ip = Common::get_local_ip();
	memcpy(in_body->ip, (local_ip).c_str(), (local_ip).size());
	memcpy(in_body->path, (Common::local_path).c_str(), (Common::local_path).size());
	
	//DEBUG_LOG("path is %s", in_body->path);

	in_body->version_id = version_id;
	in_body->name_num = list.size();
	name_info *p = in_body->name_list;
	vector<string>::const_iterator iter;
	for(iter = list.begin(); iter != list.end(); iter++) {
		if((*iter).size() > sizeof(name_info)) {
			ERROR_LOG("Service NameSize is too long.");
			return false;
		}

		if((char*)p > (in_buf + buf_len)) {
			ERROR_LOG("Too many service names.");
			return false;
		}
		memcpy(p->name, (*iter).c_str(), (*iter).size());
		p++;
	}

	return true;
}

bool CenterPackHandler::AckPack(const char* out_buf, const int buf_len, map<string, ServiceGroupInfo> &list)
{
	proto_header *out_header = (proto_header*)out_buf;
	if((int)(out_header->pkg_len) > buf_len) {
		ERROR_LOG("Get Strategy too long, size:%u", out_header->pkg_len);
		return false;
	}
	if(out_header->status_code != 0) {
		ERROR_LOG("Get Strategy failed, status from center:%u", out_header->status_code);
		return false;
	}

	serv_query_group_ack_1 *out_body = (serv_query_group_ack_1*)(out_header + 1);
	service_group_info *gp;
	service_replicas_info *rp;
	host_info *hp;
	ServiceGroupInfo *gi;
	ServiceReplicasInfo *ri;
	HostInfo *hi;
	char *p = (char*)out_body->list;
	for(unsigned int i = 0; i < out_body->num; ++i) {
		if((unsigned int)(p - out_buf) > out_header->pkg_len) {
			ERROR_LOG("out of memory.");
			return false;
		}
		gp = (service_group_info*)p;
		gi = new ServiceGroupInfo();
		gi->group_strategy = (Strategy::strategy_type)gp->group_strategy;
		gi->forbidden = (bool)gp->forbidden;
		p = (char*)(gp->list);
		for(unsigned int j = 0; j < gp->num; ++j) {
			if((unsigned int)(p - out_buf) > out_header->pkg_len) {
				ERROR_LOG("out of memory.");
				return false;
			}
			rp = (service_replicas_info*)p;
			ri = new ServiceReplicasInfo();
			ri->replicas_strategy = (Strategy::strategy_type)rp->replicas_strategy;
			ri->forbidden = (bool)rp->forbidden;
			p = (char*)rp->list;
			for(unsigned int k = 0; k < rp->num; k++) {
				if((unsigned int)(p - out_buf) > out_header->pkg_len) {
					ERROR_LOG("out of memory.");
					return false;
				}
				hp = (host_info*)p;
				hi = new HostInfo(hp->ip, hp->port, hp->timeout_ms, (bool)hp->forbidden);
				(ri->vec).push_back(*hi);
				delete hi;
				p = (char*)(hp + 1);
			}
			(gi->vec).push_back(*ri);
			delete ri;
		}
		list[gp->name] = *gi;
		delete gi;
	}

//	if((unsigned int)(p - out_buf) > out_header->pkg_len) {
//		return false;
//	}

	serv_query_group_ack_2 *out_body2 = (serv_query_group_ack_2*)p;
	if(out_body2->num <= 0) {
		if(out_body->version_id > 0) {
			version_id = out_body->version_id;
		} else {
			ERROR_LOG("Get version_id failed, version_id:%u", out_body->version_id);
			return false;
		}
		return true;
	}

	DEBUG_LOG("Get reload config! Config number:%u", out_body2->num);
	p = (char*)out_body2->list;
	char key_buf[64];
	char value_buf[MAX_BUF_LEN];

	ofstream fout;
	string path;
//	INFO_LOG("------------------PKG--------------\n");
	for(unsigned int i = 0; i < out_body2->num; ++i) {
		if((unsigned int)(p - out_buf) > out_header->pkg_len) {
			ERROR_LOG("out of memory.");
			return false;
		}
		config_info* cp = (config_info*)p;
		if(cp->type == 0) {
			memset(key_buf, '\0', sizeof(key_buf));
			memcpy(key_buf, cp->key, sizeof(key_buf) - 1);
			memset(value_buf, '\0', sizeof(value_buf));
			memcpy(value_buf, cp->value, cp->value_len);
			set_config(key_buf, value_buf);
//			INFO_LOG("key:%s, value:%s\n", key_buf, value_buf);
		} else if(cp->type == 1) {
			path = "./conf/" + string(cp->key);
			fout.open(path.c_str(), std::ios::out|std::ios::in|std::ios::trunc);
			if(fout.is_open()) {
				fout << string(cp->value);
				fout.close();
			}else {
				ERROR_LOG("Creat file:%s failed.", path.c_str());
				return false;
			}
		}
		p += (sizeof(config_info) + cp->value_len);
	}

	if(out_body->version_id > 0) {
		version_id = out_body->version_id;
	} else {
		ERROR_LOG("Get version_id failed, version_id:%u", out_body->version_id);
		return false;
	}
	return true;
}

uint32_t CenterPackHandler::GetVersion()
{
    return version_id;
}


