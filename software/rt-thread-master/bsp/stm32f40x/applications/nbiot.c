#include "nbiot.h"
#include "stdio.h"
#include "string.h"
#include <rtthread.h>
#include "uart.h"
#include "drv_oled.h"

#define NB73_BOOT_INFO        "[WH-NB73]"
#define NB73_REBOOT           "AT+NRB\r\n"

#define NB73_CHECK_SERVER     "AT+NCDP?\r\n"
#define NB73_SERVER_SYMBOL    "+NCDP"
#define NB73_SERVER_USR_CLOUD "+NCDP:117.60.157.137,5683"
#define NB73_SET_USR_CLOUD    "AT+NCDP=117.60.157.137,5683\r\n"

#define NB73_SET_INDICATE_MSG "AT+NNMI=1\r\n"

#define NB73_CHECK_NETWORK    "AT+CGATT?\r\n"
#define NB73_NETWORK_ATTACHED "+CGATT:1"
#define NB73_NETWORK_DETACHED "+CGATT:0"

#define NB73_CMD_LED_ON       "+NNMI:8,010600020001E9CA"
#define NB73_CMD_LED_OFF      "+NNMI:8,010600020000280A"


#define REPORT_INTVL_20SEC  20000
#define NETWORK_INTVL_2MIN  120000



static unsigned int ccitt_table[256] =
{ 0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280,
0xC241, 0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481,
0x0440, 0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81,
0x0E40, 0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880,
0xC841, 0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81,
0x1A40, 0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80,
0xDC41, 0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680,
0xD641, 0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081,
0x1040, 0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281,
0x3240, 0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480,
0xF441, 0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80,
0xFE41, 0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881,
0x3840, 0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80,
0xEA41, 0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81,
0x2C40, 0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681,
0x2640, 0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080,
0xE041, 0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281,
0x6240, 0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480,
0xA441, 0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80,
0xAE41, 0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881,
0x6840, 0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80,
0xBA41, 0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81,
0x7C40, 0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681,
0x7640, 0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080,
0xB041, 0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280,
0x9241, 0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481,
0x5440, 0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81,
0x5E40, 0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880,
0x9841, 0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81,
0x4A40, 0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80,
0x8C41, 0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680,
0x8641, 0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081,
0x4040,
};

/**
* @brief  Calculate CRC16
* @param  q: Pointer to data
* @param  len: data length
* @retval CRC16 value
*/
static uint16 crc16(unsigned char *q, int len)
{
  unsigned int crc = 0xffff;
  
  while (len-- > 0)
    crc = ccitt_table[(crc ^ *q++) & 0xff] ^ (crc >> 8);
  
  return crc;
}

/**
* @brief  Notify DHT11 sensor data and LED state
* @param  None
* @retval None
*/
void Nb_Iot_Send_data(void)
{
		uint16 crc;
											//01 46 00 00 00 03 06 00 25 00 14 00 00 23 8B
		uint8 tmp[15] = {0x01, 0x46, 0x00, 0x00, 0x00, 0x03, 0x06, 0x00, 0x25, 0x00, 0x35, 0x00, 0x45, 0x00, 0x00};
		tmp[8] ++; //²âÊÔÓÃ
    crc = crc16(tmp, sizeof(tmp) - 2);
		tmp[13] = crc >> 8;
		tmp[14] = crc & 0xff;
		USART6_Sends(tmp,sizeof(tmp));	
}
MSH_CMD_EXPORT(Nb_Iot_Send_data,notify_data);


/**
* @brief  NB73 task
* @param  None
* @retval None
*/
//void nb73_task(void)
//{
//  uint32 now_tick;
//  char *ptr = (char *)rx_buffer;
//  
//  if(rx_done == true)
//  {
//    if(strstr(ptr, NB73_BOOT_INFO))
//    {
//      rt_kprintf("WH-NB73 is ready. Check whether USRCLOUD is the server?\r\n");
//      uart_send_stream(NB73_SET_INDICATE_MSG);
//      
//      nb73_state = NB_SERVER;
//    }
//    else if(strstr(ptr, NB73_SERVER_SYMBOL))
//    {
//      if(strstr(ptr, NB73_SERVER_USR_CLOUD))
//      {
//        rt_kprintf("Yes. Wait WH-NB73 to attache the network\r\n");
//        nb73_state = NB_NETWORK;
//      }
//      else
//      {
//        rt_kprintf("No. Set USRCLOUD as server and reboot WH-NB73\r\n");
//        uart_send_stream(NB73_SET_USR_CLOUD);
//        //HAL_Delay(300);
//        
//        uart_send_stream(NB73_REBOOT);
//        
//        nb73_state = NB_NONE;
//      }
//    }
//    else if(strstr(ptr, NB73_NETWORK_ATTACHED))
//    {
//      rt_kprintf("WH-NB73 has attached network, ready to notify data\r\n");
//      nb73_state = NB_REPORT;
//      
//      //network_tick = HAL_GetTick();
//    }
//    else if(strstr(ptr, NB73_NETWORK_DETACHED))
//    {
//      nb73_state = NB_NETWORK;
//    }
//    else if(strstr(ptr, NB73_CMD_LED_ON))
//    {
//      rt_kprintf("Turn on the led\r\n");
//      //turn_on_led();
//      
//      notify_data();
//    }
//    else if(strstr(ptr, NB73_CMD_LED_OFF))
//    {
//      rt_kprintf("Turn off the led\r\n");
//      //turn_off_led();
//      
//      notify_data();
//    }	
//    
//    //rx_done = false;
//    //rx_len = 0;
//  }
//  
//  switch(nb73_state)
//  {
//  case NB_SERVER:
//    uart_send_stream(NB73_CHECK_SERVER);
//    //HAL_Delay(1000);
//    break;
//    
//  case NB_NETWORK:
//    uart_send_stream(NB73_CHECK_NETWORK);
//    //HAL_Delay(1000);
//    break;		
//    
//  case NB_REPORT:
//    
//   // now_tick = HAL_GetTick();
//    if(now_tick - report_tick >= REPORT_INTVL_20SEC)
//    {
//      report_tick = now_tick;
//      
//      notify_data();        
//    }
//    else if(now_tick - network_tick >= NETWORK_INTVL_2MIN)
//    {
//      network_tick = now_tick;
//      
//      nb73_state = NB_NETWORK;     
//    }
//    
//    break;
//    
//  default:
//    break;
//  }
//}




