#ifndef __GPS_H__
#define __GPS_H__

#define MAXROUTERS 8
#define MAX_VALID_DATA 10
#define pheadbuffer "Connection: keep-alive\r\n\
Cache-Control: max-age=0\r\n\
Upgrade-Insecure-Requests: 1\r\n\
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.3497.100 Safari/537.36 \r\n\
DNT: 1\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8\r\n\
Accept-Encoding: deflate\r\n\
Accept-Language: es-US,es;q=0.9,en-US;q=0.8,en;q=0.7,es-419;q=0.6\r\n\r\n"

#define DEMO_SERVER "104.31.85.110"
#define DEMO_SERVER_PORT 80

void get_cordanates(void *pvParameters);
void http_parse(char* info);
void JSON_parse(char *word);
bool Data_Result(char * string);
int integer_part(void);
char * location(void);

typedef enum {
  IDLE,
  INIT,
  BRACKET,
  LAT,
  LON,
  ERR,
  COMMA,
  END
} parse_state ;

typedef struct
{
    uint32_t decimal_average;
    uint32_t sum;
    uint32_t weight_sum;
}cordenate;


#endif