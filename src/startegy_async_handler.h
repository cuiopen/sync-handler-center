#ifndef _STARTEGY_ASYNC_HANDLER_H
#define _STARTEGY_ASYNC_HANDLER_H

#include "service_group.h"
#include "proto_handler.h"
#include "client_info.h"
#include "center_package.h"

class StrategyAsyncProtoHandler : public IntAsyncProtoHandler,public CenterPackHandler
{
public:
	StrategyAsyncProtoHandler(){}
	virtual ~StrategyAsyncProtoHandler(){}

    // proc_pkg_ser通过fd找到此ProtoHandler后的处理
    virtual int GetPkgLenSer(const char *buf, uint32_t len);
    virtual bool GetSeqNum(const char *buf, uint32_t len, uint32_t &seq_num);
    virtual void ProcPkgSer(const char *buf, uint32_t len);
    
    // 真正的逻辑代码，各个ServiceGroup有不通的处理函数，名称各有不同
    // proc_pkg_cli调用或是proc_pkg_ser中在其他ProtoHandler中调用
    
    // return 0表示成果，否则返回系统错误给client
    bool AsyncGetProcessIdentity(ClientInfo *ci, vector<string> &name_list, uint32_t seq_num);
};

#endif
