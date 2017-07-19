#ifndef _SWITCH_PROTO_HANDLER_H_
#define _SWITCH_PROTO_HANDLER_H_

#include "proto.h"
#include "service_group.h"
#include "proto_handler.h"

class ClientInfo;
class SwitchProtoHandler : public IntAsyncProtoHandler
{
public:

    SwitchProtoHandler(){}
    virtual ~SwitchProtoHandler(){}
    virtual int GetPkgLenSer(const char *buf, uint32_t len);
    virtual bool GetSeqNum(const char *buf, uint32_t len, uint32_t &seq_num);
    virtual void ProcPkgSer(const char *buf, uint32_t len);
    
    // 真正的逻辑代码，各个ServiceGroup有不通的处理函数，名称各有不同
    // proc_pkg_cli调用或是proc_pkg_ser中在其他ProtoHandler中调用
    virtual bool ProcessIdentity(ClientInfo *ci, uint32_t seq_num, uint32_t userid);
    
    // 获得类型
    //virtual int  GetType()
    //{
    //    return m_type;
    //}

public:

    //int      m_type;
   


private:


    
    // 调用接口是可能需要做下略微调整
    //std::string            m_dbproxy_name;
    //ServiceGroupInfo       m_group_info;
    //int                    m_game_id;
    
    //int                    m_timeout_s;
    //int                    m_failed_times; 
};



#endif
