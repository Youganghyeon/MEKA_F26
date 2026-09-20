/*
 * flash.c
 *
 *  Created on: 2020. 12. 14.
 *      Author: baram
 */

// flash erase must sector size or everything
// flash write generally 32bit but we are now 8bit.
#include "flash.h"
#include "cli.h"

#ifdef _USE_HW_FLASH


#define FLASH_SECTOR_MAX          8



typedef struct
{
  uint32_t addr;
  uint32_t length;
} flash_tbl_t;



static flash_tbl_t flash_tbl[FLASH_SECTOR_MAX]=
{
  {0x08000000,1024*16},
  {0x08004000,1024*16},
  {0x08008000,1024*16},
  {0x0800C000,1024*16},
  {0x08010000,1024*64},
  {0x08020000,1024*128},
  {0x08040000,1024*128},
  {0x08060000,1024*128},
};

static bool flashInSector(uint16_t sector_num, uint32_t addr, uint32_t length);


#ifdef _USE_HW_CLI
static void cliFlash(cli_args_t* args);
#endif


bool flashInit(void)
{
#ifdef _USE_HW_CLI
cliAdd("flash", cliFlash);
#endif
  return true;
}

bool flashErase(uint32_t addr, uint32_t length)
{
  bool ret = false;
  HAL_StatusTypeDef status;
  FLASH_EraseInitTypeDef init;
  uint32_t page_error;

  int16_t  start_sector_num = -1;
  uint32_t sector_count = 0;


  for (int i=0; i<FLASH_SECTOR_MAX; i++)
  {
    if (flashInSector(i, addr, length) == true)
    {
      if (start_sector_num < 0)
      {
        start_sector_num = i;
      }
      sector_count++;
    }
  }


  if (sector_count > 0)
  {
    HAL_FLASH_Unlock();

    init.TypeErase     = FLASH_TYPEERASE_SECTORS;
    init.VoltageRange  = FLASH_VOLTAGE_RANGE_3;      // Operating Voltage : 3.3V
    init.Banks         = FLASH_BANK_1;
    init.Sector        = start_sector_num;  //stm32F103 -> page address stmf411 -> page (sector)
    init.NbSectors     = sector_count;


    status = HAL_FLASHEx_Erase(&init, &page_error);
    if (status == HAL_OK)
    {
      ret = true;
    }

    HAL_FLASH_Lock();
  }

  return ret;
}

bool flashWrite(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  HAL_StatusTypeDef status;

  HAL_FLASH_Unlock();

  for (int i=0; i<length; i++)
  {
    uint8_t data;
    data = p_data[i+0]<<0;

    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr + i, (uint64_t)data);
    if (status != HAL_OK)
    {
      ret = false;
      break;
    }
  }

  HAL_FLASH_Lock();

  return ret;
}

bool flashRead(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  uint8_t *p_byte = (uint8_t *)addr;


  for (int i=0; i<length; i++)
  {
    p_data[i] = p_byte[i];
  }

  return ret;
}

bool flashInSector(uint16_t sector_num, uint32_t addr, uint32_t length)
{
  bool ret = false;

  uint32_t sector_start;
  uint32_t sector_end;
  uint32_t flash_start;
  uint32_t flash_end;


  sector_start = flash_tbl[sector_num].addr;
  sector_end   = flash_tbl[sector_num].addr + flash_tbl[sector_num].length - 1;
  flash_start  = addr;
  flash_end    = addr + length - 1;


  if (sector_start >= flash_start && sector_start <= flash_end)
  {
    ret = true;
  }

  if (sector_end >= flash_start && sector_end <= flash_end)
  {
    ret = true;
  }

  if (flash_start >= sector_start && flash_start <= sector_end)
  {
    ret = true;
  }

  if (flash_end >= sector_start && flash_end <= sector_end)
  {
    ret = true;
  }

  return ret;
}


#ifdef _USE_HW_CLI
void cliFlash(cli_args_t* args)
{
  bool ret=false;
  if((args->argc == 1) && args->isStr(0, "info"))
  {
    for(int i=0; i<FLASH_SECTOR_MAX; i++)
   {
     cliPrintf("%X \n", flash_tbl[i].addr);
     cliPrintf("%d KB \n", flash_tbl[i].length/1024);
   }
    ret=true;
  }
  else if((args->argc == 3) && args->isStr(0, "read"))
  {
    uint32_t addr = (uint32_t)args->getData(1);
    uint32_t length = (uint32_t)args->getData(2);
    uint8_t buf[length];

     flashRead(addr, &buf[0], length);
     for(int i=0; i<length; i++)
      {
        cliPrintf("0x%X \t 0x%X \t 0x%X\n",(addr+i), buf[i], *((uint8_t *)(addr+i)));
      }
     ret=true;
  }
  else if((args->argc == 3) && args->isStr(0, "write"))
  {
    uint32_t addr = (uint32_t)(args->getData(1));
    uint32_t buf = (uint32_t)(args->getData(2));

    if(flashWrite(addr, (uint8_t *)&buf, 4)==true)
    {
     cliPrintf("Write! \n");

    }
    else
    {
      cliPrintf("Write Fail \n");
    }
    ret=true;
  }
  else if((args->argc == 3) && args->isStr(0, "erase"))
  {
    uint32_t addr = (uint32_t)(args->getData(1));
    uint32_t length = (uint32_t)(args->getData(2));

    if(flashErase(addr, length) == true)
    {
      cliPrintf("Erase ! \n");
    }
    else
    {
      cliPrintf("Erase fail \n");
    }
    ret=true;
  }

  if(ret != true)
   {
     cliPrintf("flash info\n");
     cliPrintf("flash read addr len \n");
     cliPrintf("flash write addr data \n");
     cliPrintf("flash erase addr len \n");
   }
}
#endif
#endif
