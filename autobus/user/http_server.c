#include "lwip/arch.h"
#include "lwip/api.h"
#include "freeRTOS_wrapper.h"
#include "http_server.h"
#include <string.h>
#include <stdlib.h>	
#include "Flash_driver.h"

SemaphoreHandle_t Provising = NULL;
//#define DEBUG
#if LWIP_NETCONN

#ifndef HTTPD_DEBUG
#define HTTPD_DEBUG         LWIP_DBG_OFF
#endif

const static char http_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
//const static char http_index_html[] = "<html><head><title>Congrats!</title></head><body><h1>Welcome to our lwIP HTTP server!</h1><p>This is a small test page, served by httpserver-netconn.</body></html>";
const static char http_index_html[] = HTMLCODE;

bool end_task;

//Callbacks to Flashset
Flash_func_t Flash_set[6];

/** Serve one HTTP connection accepted in the http thread */
static void
http_server_netconn_serve(struct netconn *conn)
{
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;
  
  /* Read the data from the port, blocking if nothing yet there. 
   We assume the request (the part we care about) is in one netbuf */
  err = netconn_recv(conn, &inbuf);
  
  if (err == ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);
    
    /* Is this an HTTP GET command? (only check the first 5 chars, since
    there are other formats for GET, and we're keeping it very simple )*/
    if (buflen>=5 &&
        buf[0]=='G' &&
        buf[1]=='E' &&
        buf[2]=='T' &&
        buf[3]==' ' &&
        buf[4]=='/' ) 
      {
        #ifdef DEBUG
        printf("*******************\r\n");
        printf("Data: %s\r\n",buf );
        printf("*******************\r\n");
        #endif
      /*This code tokenize the Http data*/
        if (
        buf[5]=='W' &&
        buf[6]=='i' &&
        buf[7]=='f' &&
        buf[8]=='i')
        {
          http_paser(buf);
          
          netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);
          netconn_write(conn, http_index_html, sizeof(http_index_html)-1, NETCONN_NOCOPY);
          
          netconn_close(conn);
          netconn_delete(conn);

          Flash_write();

          xSemaphoreGive(Provising);

          vTaskDelete(NULL);
          //Semaphore read, data is gotten and thread task delete
          
        }

      /* Send the HTML header 
             * subtract 1 from the size, since we dont send the \0 in the string
             * NETCONN_NOCOPY: our data is const static, so no need to copy it
       */
      netconn_write(conn, http_html_hdr, sizeof(http_html_hdr)-1, NETCONN_NOCOPY);
      
      /* Send our HTML page */
      netconn_write(conn, http_index_html, sizeof(http_index_html)-1, NETCONN_NOCOPY);
      
    }
  }
  /* Close the connection (server closes in HTTP) */
  netconn_close(conn);
  
  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);
}

/** The main function, never returns! */
static void
http_server_netconn_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);
  
  /* Create a new TCP connection handle */
  conn = netconn_new(NETCONN_TCP);
  LWIP_ERROR("http_server: invalid conn", (conn != NULL), return;);
  
  /* Bind to port 80 (HTTP) with default IP address */
  netconn_bind(conn, NULL, 80);
  
  /* Put the connection into LISTEN state */
  netconn_listen(conn);
  
  do {
    err = netconn_accept(conn, &newconn);
    if (err == ERR_OK) {
      http_server_netconn_serve(newconn);
      netconn_delete(newconn);
    }
  } while(err == ERR_OK);
  LWIP_DEBUGF(HTTPD_DEBUG,
    ("http_server_netconn_thread: netconn_accept received error %d, shutting down",
    err));
  netconn_close(conn);
  netconn_delete(conn);
  vTaskDelete(NULL);
}
void http_paser(char *buf)
{
  char * RAW_data;
  char RAW_data_index=0;
  char RAW_data_len;

	bool read_begin;
	bool read_end;

	char string_parsed[30];
	char parsed_index;

	char callback_index = 0 ;
	
  /*if there are files set dont set again
      This list of callbacks are used
      to set flash driver data
  */
  if(Flash_set[0]==NULL)
  {
    Flash_set[0]= &set_FLASH_SSID;
    Flash_set[1]= &set_FLASH_PASS;
    Flash_set[2]= &set_FLASH_RUTA;
    Flash_set[3]= &set_FLASH_UNIDAD;
    Flash_set[4]= &set_FLASH_COSTO;
    Flash_set[5]= &set_FLASH_EMAIL;
    Flash_set[6]= &set_FLASH_EMAIL_TIME;
  }
  
	RAW_data = strtok(buf,"?");
	RAW_data = strtok(NULL," ");
	RAW_data_len = strlen(RAW_data);
  printf("RAW_DATA: %s\r\n",RAW_data);
	memset(string_parsed,0,sizeof(string_parsed));
	do
	{
	    if(RAW_data[ RAW_data_index ] == '=')
	    {
	        read_begin = true;
	        read_end = false;
	        RAW_data_index++;
	        printf("\r\n");
	        printf("equal detected\r\n");
	    }
        if(RAW_data[ RAW_data_index ]=='&' || RAW_data_index == ( RAW_data_len-1))
        {
            /*for send the last data*/
            if(RAW_data_index == ( RAW_data_len-1))
            {
              string_parsed[parsed_index] = RAW_data [RAW_data_index];
            }
            read_end=true;
            printf("%s\r\n",string_parsed);
            
            Flash_set[callback_index](string_parsed);
            
            callback_index++;
            memset(string_parsed,0,sizeof(string_parsed));
            parsed_index=0;
        }
	    if(read_begin==true && read_end==false)
	    {
	    	string_parsed[parsed_index] = RAW_data [ RAW_data_index ];
        parsed_index++;
	    }
	    RAW_data_index++;
	}
	while( RAW_data_index < RAW_data_len); 
}
/** Initialize the HTTP server (start its thread) */
void
http_server_netconn_init(void)
{
  if(Provising == NULL)
  {
    Provising = xSemaphoreCreateMutex();
  }
  if(Provising!=NULL)
  {
    xSemaphoreTake( Provising, ( TickType_t ) 0 );
  }
  xTaskCreate( http_server_netconn_thread, (signed char *)"Thread", 256, NULL, 8, NULL );
}

#endif /* LWIP_NETCONN*/