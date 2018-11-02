#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#define RCV
#ifdef RCV
    #define RCV_PRINTF printf
#else
    #define RCV_PRINTF
#endif

#endif