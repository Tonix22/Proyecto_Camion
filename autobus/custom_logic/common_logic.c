#include "common_logic.h"
#include "esp_common.h"
#include <stdio.h>
#include <string.h>

//this function deletes + and changes to spaces 
void SSID_space_parse(char* word)
{
    char match;
	char * token_str;
	
	token_str=strchr(word,'+');
	  
	while (token_str!=NULL)
	{
	    match = token_str-word;
	    word[match]=' ';
	    token_str=strchr(token_str+1,'+');
	}
}

void half_of_string_number(char *ACTUAL,char* HALF)
{
    uint32_t temp;
	uint8_t price_size;
	uint8_t i;
	uint32_t decimal    = 0;
	uint32_t ten        = 1;
	uint8_t price_index = 0;
	
    price_size = strlen(ACTUAL);
	do
	{
		for(i=0;i<price_size-1;i++)
		{
			ten*=10;
		}
		decimal+=(ACTUAL[price_index]-0x30)*ten;
		ten=1;
		price_index++;
		price_size--;
	}
	while(price_size!=0);

    if(decimal%2 == 0)
	{
    
   	 	decimal=decimal/2;
   	 	sprintf(HALF,"%d",decimal);
    }
	else
    {
   	 	decimal=decimal/2;
   	  	sprintf(HALF,"%d.5",decimal);
    }
}