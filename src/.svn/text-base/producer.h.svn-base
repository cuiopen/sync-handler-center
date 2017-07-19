#ifndef _PRODUCER_H
#define _PRODUCER_H

#include "processor.h"
#include "service_group.h"
#include "client_info.h"



class Producer : public IProcessor
{
public:
    virtual int Init();
    virtual int Uninit();
    // virtual int GetPkgLenCli(const char *buf, uint32_t len);
    virtual int GetPkgLenSer(int fd, const char *buf, uint32_t len);
	// virtual int CheckOpenCli(uint32_t ip, uint16_t port);
    // virtual int SelectChannel(int fd, const char *buf, uint32_t len, uint32_t ip, uint32_t work_num);
    //virtual int ShmqPushed(int fd, const char *buf, uint32_t len, int flag);
    virtual void TimeEvent();
    virtual void ProcPkgCli(int fd, const char *buf, uint32_t len);
    virtual void ProcPkgSer(int fd, const char *buf, uint32_t len);
    virtual void LinkUpCli(int fd, uint32_t ip);
    virtual void LinkUpSer(int fd, uint32_t ip, uint16_t port);
    virtual void LinkDownCli(int fd);
    virtual void LinkDownSer(int fd);

    virtual int InitFdList();
    virtual IAsyncProtoHandler *GetAsyncProtoHandlerByFd(int fd);

    static void DestroyClient(uint32_t sn);

    // ClientInfo没有创建成功时调用
    static void SendToClient(int fd, uint32_t seq_num, uint32_t cmd_id, 
                         uint32_t status_code, uint32_t user_id);
    // ClientInfo已经创建成功调用，协议发送完毕后会移除此ClientInfo
    static void SendToClient(ClientInfo *ci, const char *buf, 
                      int len, uint32_t status_code);

    static int DoServerTimeout(void *owner, void *arg);

    
    // 处理同步的逻辑，非协议驱动
    // 此为定时回调函数，处理主要业务逻辑
    static void SyncToServer();


};


#endif
