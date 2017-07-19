#include <string>
#include <arpa/inet.h>

#include "async_server.h"

#include "file_config_manager.h"
#include "center_config_manager.h"

#include "proxy.h"
#include "control.h"
#include "service_group.h"

#include "server.h"
#include "i_mysql_iface.h"

#include "producer.h"
//#include "reissure.h"

static IProcessor* main = NULL;
static ConfigManager *cm = NULL;
i_mysql_iface * g_mysql_handler = NULL;

const char * g_sync_table_name = NULL;
const char * g_last_sync_table_name = NULL;

//std::map<chnl_key_t, uint32_t, chnl_key_comp> chnl_map;
//std::map<std::string, uint32_t> stat_id_map;
//std::map<std::string, cmd_info_t> g_cmd_id_map;
//uint32_t is_discount;
//uint32_t game_id;
//uint32_t cplan_yearvip;

extern "C" bool load_config(const char *source, int type)
{
    if (cm && cm->IsLoaded()) {
        return true;
    } else if (cm){
        return cm->LoadConfig(source);
    }

    if (type == 0)
        cm =  new (std::nothrow) FileConfigManager();
    else if(type == 1)
        cm =  new (std::nothrow) CenterConfigManager();
    else if(type == 2)
        cm =  new (std::nothrow) CenterConfigManager();
    else
        return false;

    if (!cm) {
        // TODO
        return false;
    }
    return cm->LoadConfig(source);
}

extern "C" bool reload_config(const char *source)
{
    if (!cm || !cm->IsLoaded()) {
        // TODO
        return false;
    }
    return cm->ReloadConfig(source);
}

extern "C" void set_config(const char* key, const char* value)
{
    if (!cm || !cm->IsLoaded()) {
        return;
    }
    cm->SetConfigMap(key, value);
}

extern "C" int config_get_intval(const char *key, int defult)
{
    if (!cm || !cm->IsLoaded()) {
        // TODO
        return defult;
    }
    return cm->ConfigGetIntVal(key, defult);
}

extern "C" const char *config_get_strval(const char *key, const char *defult)
{
    if (!cm || !cm->IsLoaded()) {
        // TODO
        return defult;
    }
    return cm->ConfigGetStrVal(key, defult);
}

extern "C" int plugin_init(int type)
{
    if (type == PROC_WORK) {
        // 初始化路由表
        // 初始化内存池
        // 初始化hash表<client_info, void*>，value取自内存池
        // 初始化fd链表数组，用于在客户端关闭时删除其fd对应的链表
        // 初始化定时器处理超时
        
		int ret = 0;
		ret = create_mysql_iface_instance(&g_mysql_handler);
		if (ret != 0 || g_mysql_handler == NULL) {
	    	ERROR_LOG("create mysql instance failed");
	    	return -1;
		}
	
		ret = g_mysql_handler->init(
				cm->ConfigGetStrVal("mysql_host", "localhost"),
				cm->ConfigGetIntVal("mysql_port", 0),
				cm->ConfigGetStrVal("mysql_db", " "),
				cm->ConfigGetStrVal("mysql_user", " "),
				cm->ConfigGetStrVal("mysql_passwd", " "),
				cm->ConfigGetStrVal("mysql_charset", "UTF8"));
		if (ret != 0)
		{
			ERROR_LOG("mysql init failed, error = %s",
					 g_mysql_handler->get_last_errstr());
			return -1;
		}
        

        g_sync_table_name = config_get_strval("sync_table_name", NULL);
        if (g_sync_table_name == NULL) {
            ERROR_LOG("config_get_strval sync_table_name failed");
            return -1;
        }

        g_last_sync_table_name = config_get_strval("last_sync_table_name", NULL);
        if (g_last_sync_table_name == NULL) {
            ERROR_LOG("config_get_strval last_sync_table_name failed");
            return -1;
        }

        DEBUG_LOG("total work num %u, current work index %u", get_work_num(), get_work_idx());
        if (get_work_idx() == get_work_num() - 1) {
            // 最后一个进程为control进程
            main = new (std::nothrow) Control();
            if (!main || main->Init()) {
                ERROR_LOG("new Control or Init failed");
                return -1;
            }
        } else if (get_work_idx() == get_work_num() - 2){
            // 此proxy进程为主表进程
            main = new (std::nothrow) Proxy();
            if (!main || main->Init()) {
                ERROR_LOG("new Proxy or Init failed");
                return -1;
            }
        } else {
            // 第一个进程为生成者进程，不断向数据库里插入数据
            main = new (std::nothrow) Producer();
            if (!main || main->Init()) {
                ERROR_LOG("new Producer or Init failed");
                return -1;
            }
        }

        // 还有一个issure进程，负责补单
    } else if (type == PROC_MAIN) {
        // 获取数据库操作句柄

		//int ret = 0;
		//ret = create_mysql_iface_instance(&g_mysql_handler);
		//if (ret != 0 || g_mysql_handler == NULL) {
	    //	ERROR_LOG("create mysql instance failed");
	    //	return -1;
		//}
	
		//ret = g_mysql_handler->init(
		//		cm->ConfigGetStrVal("mysql_host", "localhost"),
		//		cm->ConfigGetIntVal("mysql_port", 0),
		//		cm->ConfigGetStrVal("mysql_db", " "),
		//		cm->ConfigGetStrVal("mysql_user", " "),
		//		cm->ConfigGetStrVal("mysql_passwd", " "),
		//		cm->ConfigGetStrVal("mysql_charset", "UTF8"));
		//if (ret != 0)
		//{
		//	ERROR_LOG("mysql init failed, error = %s",
		//			 g_mysql_handler->get_last_errstr());
		//	return -1;
		//}
        //

        //g_sync_table_name = config_get_strval("sync_table_name", NULL);
        //if (g_sync_table_name == NULL) {
        //    ERROR_LOG("config_get_strval sync_table_name failed");
        //    return -1;
        //}

        //g_last_sync_table_name = config_get_strval("last_sync_table_name", NULL);
        //if (g_last_sync_table_name == NULL) {
        //    ERROR_LOG("config_get_strval last_sync_table_name failed");
        //    return -1;
        //}


        BOOT_LOG(0, "Proxy BuildTime: %s %s", __TIME__, __DATE__);
    }
    
    return 0;
}

extern "C" int plugin_fini(int type)
{
	DEBUG_LOG("Proxy finiting...");

    if (g_mysql_handler)
    {
        delete g_mysql_handler;
        g_mysql_handler = NULL;
    }

    if(main)
    {
        main->Uninit();
        delete main;
        main = NULL;
    }
    

	DEBUG_LOG("Proxy finit successfully!");
	return 0;
}

extern "C" int get_pkg_len_cli(const char *buf, uint32_t len) 
{
    if (len < sizeof(uint32_t)) {
        return 0;
    }

    return *(uint32_t*)buf;
}

extern "C" int get_pkg_len_ser(int fd, const char *buf, uint32_t len) 
{
    return main->GetPkgLenSer(fd, buf, len);
}

extern "C" int check_open_cli(uint32_t ip, uint16_t port) 
{
    //in_addr in_ip;
    //in_ip.s_addr = ip;
    //string str_ip = inet_ntoa(in_ip);
    //string allowd_id = config_get_strval("allowd_ip", "");
	//if(allowd_id.size() <= 1) {
	//	return 0;
	//}

    //if(allowd_id.find(str_ip) == std::string::npos) {
    //    ERROR_LOG("Remote ip[%s] illegal, port[%u].", str_ip.c_str(), (int)port);
    //    return -1;
    //}
	return 0;
}

extern "C" int select_channel(int fd, const char *buf, uint32_t len, uint32_t ip, uint32_t work_num) 
{
    static int serialize_num = 0;
    return (serialize_num++) % (work_num - 2);  // 留出control进程,一个同步进程，只留下一个进程收客户端信息
}

extern "C" int shmq_pushed(int fd, const char *buf, uint32_t len, int flag)
{
    return 0;
}
 
extern "C" void time_event()
{
    // TODO 调试加的限制
    // if (get_work_idx() < get_work_num() - 1) 
    main->TimeEvent();

    // 检查超时请求
    // 检查有没有后端服务连接断开，如果有进行重连
}

extern "C" void proc_pkg_cli(int fd, const char *buf, uint32_t len)
{
    main->ProcPkgCli(fd, buf, len);
    
    // 从内存池中获取client_info空间，插入hash表
    // 插入fd链表数组，以便客户端关闭时删除其对应的所有请求
    // 根据路由表net_send_ser，即通过ServiceGroup（同时要将此client_info加入到timeout定时器）
}

extern "C" void proc_pkg_ser(int fd, const char *buf, uint32_t len)
{
    main->ProcPkgSer(fd, buf, len);
    
    // 从hash表中找到此client_info，未找到则丢弃(可能是超时了)
    // 检查此fd链表数组是否存在，若不存在则客户端已关闭
    // 从timeout定时器中删除(准备返回给客户端或是下一次加入到timeout定时器)
    // 发送到client或是server
}

extern "C" void link_up_cli(int fd, uint32_t ip)
{
    // TODO 调试加的限制
    // if (get_work_idx() < get_work_num() - 1) 
    main->LinkUpCli(fd, ip);

    // TODO 初始化此fd对应的fd链表数组
}

extern "C" void link_up_ser(int fd, uint32_t ip, uint16_t port)
{
    main->LinkUpSer(fd, ip, port);
    
    // 修改此service为Enable
}

extern "C" void link_down_cli(int fd)
{
    // TODO 调试加的限制
    // if (get_work_idx() < get_work_num() - 1) 
    main->LinkDownCli(fd);
    
    // TODO 删除fd链表数组中此fd对应的链表的所有的client_info
}

extern "C" void link_down_ser(int fd)
{
    main->LinkDownSer(fd);
    
    // 修改backend_service_map中此service为Disable，并重新异步建立连接(此处实现须谨慎)
}
