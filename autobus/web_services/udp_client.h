#include "user_config.h"

//const uint8 udp_server_ip[4]={192,168,1,100};


#define UDP_SERVER_LOCAL_PORT (1024)
#define UDP_SERVER_GREETING "Hello!This is a udp server test\n"
#define UDP_Client_GREETING "Hello!This is a udp Client test\n"


#define DBG_PRINT(fmt,...)	do{\
	    os_printf("[Dbg]");\
	    os_printf(fmt,##__VA_ARGS__);\
	}while(0)

#define ERR_PRINT(fmt,...) do{\
	    os_printf("[Err] Fun:%s Line:%d ",__FUNCTION__,__LINE__);\
	    os_printf(fmt,##__VA_ARGS__);\
	}while(0)
#define DBG_LINES(v) os_printf("------------------%s---------------\n",v)

