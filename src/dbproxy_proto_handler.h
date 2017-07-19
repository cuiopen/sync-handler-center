#ifndef _DBPROXY_PROTO_HANDLER_H_
#define _DBPROXY_PROTO_HANDLER_H_

//#include <stdint.h>

#include "proto.h"
#include "service_group.h"
#include "proto_handler.h"

// 前向声明
class ClientInfo;
class DbproxyProtoHandler : public IntAsyncProtoHandler
{
public:
    DbproxyProtoHandler(){}
    virtual ~DbproxyProtoHandler(){}
    virtual int GetPkgLenSer(const char *buf, uint32_t len);
    virtual bool GetSeqNum(const char *buf, uint32_t len, uint32_t &seq_num);
    virtual void ProcPkgSer(const char *buf, uint32_t len);
    
    //真正的逻辑代码，各个ServiceGroup有不通的处理函数，名称各有不同
    //proc_pkg_cli调用或是proc_pkg_ser中在其他ProtoHandler中调用
    virtual bool ProcessIdentity(ClientInfo *ci, uint32_t seq_num, uint32_t userid);
};

// 增加一个异步的protohandler
// 同步和异步的，进行处理，对于路由表的优化



#endif
