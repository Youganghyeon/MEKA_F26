/*
 * cmd.c
 *
 *  Created on: 2026. 3. 26.
 *      Author: yougang
 */

#include "cmd.h"

#ifdef _USE_HW_CMD

#include "uart.h"


#define CMD_STX                     0x02
#define CMD_ETX                     0x03

#define CMD_STATE_WAIT_STX          0
#define CMD_STATE_WAIT_CMD          1
#define CMD_STATE_WAIT_DIR          2
#define CMD_STATE_WAIT_ERROR        3
#define CMD_STATE_WAIT_LENGTH_L     4
#define CMD_STATE_WAIT_LENGTH_H     5
#define CMD_STATE_WAIT_DATA         6
#define CMD_STATE_WAIT_CHECKSUM     7
#define CMD_STATE_WAIT_ETX          8

uint8_t debug_data;
void cmdInit(cmd_t *p_cmd)
{
  p_cmd->is_init=false;
  p_cmd->state=CMD_STATE_WAIT_STX;
  p_cmd->rx_packet.data=&p_cmd->rx_packet.buffer[CMD_STATE_WAIT_STX];
  p_cmd->tx_packet.data=&p_cmd->tx_packet.buffer[CMD_STATE_WAIT_STX];
}

bool cmdOpen(cmd_t *p_cmd, uint8_t ch, uint32_t baud)
{
  p_cmd->is_init=true;
  p_cmd->ch =ch;
  p_cmd->baud = baud;
  p_cmd->state=CMD_STATE_WAIT_STX;
  return uartOpen(ch,baud);
}

bool cmdClose(cmd_t *p_cmd)
{
  return uartClose(p_cmd->ch);
}
bool cmdReceivePacket(cmd_t *p_cmd)
{
  bool ret=false;
  if(p_cmd->is_init==true)
  {
    uint8_t rx_data;
    if(uartAvailable(p_cmd->ch>0))
    {
      rx_data=uartRead(p_cmd->ch);
      switch(p_cmd->state)
      {
        case CMD_STATE_WAIT_STX:
          if(rx_data==CMD_STX)
          {
            p_cmd->rx_packet.check_sum = 0;
            p_cmd->state=CMD_STATE_WAIT_CMD;
          }
          break;
        case CMD_STATE_WAIT_CMD:
            p_cmd->rx_packet.cmd=rx_data;
            p_cmd->rx_packet.check_sum ^= rx_data;
            p_cmd->state=CMD_STATE_WAIT_DIR;
          break;
        case CMD_STATE_WAIT_DIR:
          p_cmd->rx_packet.dir=rx_data;
          p_cmd->rx_packet.check_sum ^= rx_data;
          p_cmd->state=CMD_STATE_WAIT_ERROR;
          break;
        case CMD_STATE_WAIT_ERROR:
          p_cmd->rx_packet.error=rx_data;
          p_cmd->rx_packet.check_sum ^= rx_data;
          p_cmd->state=CMD_STATE_WAIT_LENGTH_L;
          break;
        case CMD_STATE_WAIT_LENGTH_L:
          p_cmd->rx_packet.length=rx_data;
          p_cmd->rx_packet.check_sum ^= rx_data;
          p_cmd->state=CMD_STATE_WAIT_LENGTH_H;
          break;
        case CMD_STATE_WAIT_LENGTH_H:
          p_cmd->rx_packet.length|=rx_data<<8;
          p_cmd->rx_packet.check_sum ^= rx_data;
          if(p_cmd->rx_packet.length>0)
          {
            p_cmd->index=0;
            p_cmd->state=CMD_STATE_WAIT_DATA;
          }
          else
          {
            p_cmd->state=CMD_STATE_WAIT_CHECKSUM;
          }
          break;
        case CMD_STATE_WAIT_DATA:
          p_cmd->rx_packet.data[p_cmd->index]=rx_data;
          p_cmd->index++;
          p_cmd->rx_packet.check_sum ^= rx_data;
          if(p_cmd->index==p_cmd->rx_packet.length)
          {
            p_cmd->state=CMD_STATE_WAIT_CHECKSUM;
          }
          break;
        case CMD_STATE_WAIT_CHECKSUM:
          p_cmd->rx_packet.check_sum_recv=rx_data;
          p_cmd->state=CMD_STATE_WAIT_ETX;
          break;
        case CMD_STATE_WAIT_ETX:
          if(rx_data==CMD_ETX)
          {
            if(p_cmd->rx_packet.check_sum==p_cmd->rx_packet.check_sum_recv)
            {
              ret=true;
            }
          }
          p_cmd->state=CMD_STATE_WAIT_STX;
          break;
      }
    }
  }
  return ret;
}

void cmdSendCmd(cmd_t *p_cmd, uint8_t cmd, uint8_t *p_data, uint32_t length)
{
  uint32_t index=0;
  p_cmd->tx_packet.buffer[index++]=CMD_STX;
  p_cmd->tx_packet.buffer[index++]=cmd;
  p_cmd->tx_packet.buffer[index++]=CMD_DIR_S_TO_M;
  p_cmd->tx_packet.buffer[index++]=CMD_OK;
  p_cmd->tx_packet.buffer[index++]=((uint8_t)(length &0xFF));
  p_cmd->tx_packet.buffer[index++]= ((uint8_t)((length>>8)&0xFF));
  for(int i=0; i<length; i++)
  {
    p_cmd->tx_packet.buffer[index++] = p_data[i];
  }
  uint8_t checksum=0;
  for(int i=0; i<length+5; i++)
  {
    checksum ^=p_cmd->tx_packet.buffer[i+1];
  }
  p_cmd->tx_packet.buffer[index++] = checksum;
  p_cmd->tx_packet.buffer[index++] = CMD_ETX;

  uartWrite(_DEF_UART1, &(p_cmd->tx_packet.buffer[0]),index);

}
void cmdSendResp(cmd_t *p_cmd, uint8_t cmd, uint8_t err_code, uint8_t *p_data, uint32_t length)
{
  uint32_t index=0;
  p_cmd->tx_packet.buffer[index++]=CMD_STX;
  p_cmd->tx_packet.buffer[index++]=cmd;
  p_cmd->tx_packet.buffer[index++]=CMD_DIR_S_TO_M;
  p_cmd->tx_packet.buffer[index++]=err_code;
  p_cmd->tx_packet.buffer[index++]=((uint8_t)(length &0xFF));
  p_cmd->tx_packet.buffer[index++]=((uint8_t)((length>>8)&0xFF));
  for(int i=0; i<length; i++)
  {
    p_cmd->tx_packet.buffer[index++] = p_data[i];
  }
  uint8_t checksum=0;
  for(int i=0; i<length+5; i++)
  {
    checksum ^=p_cmd->tx_packet.buffer[i+1];
  }
  p_cmd->tx_packet.buffer[index++] = checksum;
  p_cmd->tx_packet.buffer[index++] = CMD_ETX;

  uartWrite(_DEF_UART1, &(p_cmd->tx_packet.buffer[0]),index);
}

bool cmdSendCmdRxResp(cmd_t *p_cmd, uint8_t cmd, uint8_t *p_data, uint32_t length, uint32_t timeout)
{
 bool ret =false;
 cmdSendCmd(p_cmd, cmd, p_data, length);
 uint32_t pre_time = millis();
 while(1)
 {
   if(cmdReceivePacket(p_cmd))
   {
     ret=true;
     break;
   }
   if(millis()-pre_time >= timeout)
   {
     break;
   }
 }
 return ret;
}
#endif
