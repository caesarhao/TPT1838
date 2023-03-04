#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void disable_batterypolling(void);
char CRC8_Calculator(uint8_t *data, int32_t length);
void display_version(void);
void display_usage(void);
// int open(const char *path, int oflag, ... );
//ssize_t write(int fildes, const void *buf, size_t nbyte);
// ssize_t read(int fildes, void *buf, size_t nbyte);
// off_t lseek(int fildes, off_t offset, int whence);
void disable_batterypolling(void)
{
  ssize_t nbytes;
  uint8_t cmd[4];
  int fd;
  
  cmd[0] = '1';
  fd = open("/sys/EcControl/ECflashMode", (O_RDWR|O_NONBLOCK));
  if (fd < 0) {
    puts("error: cannot open EC device file!");
  }
  else {
    nbytes = write(fd, cmd, 1);
    if (nbytes < 0) {
      puts("error: write ec node.");
    }
  }
  close(fd);
  return;
}

int read_ec_device(uint8_t reg, void *buf)
{
  ssize_t nbytes;
  off_t fl;
  int retu;
  uint8_t cmd[4];
  int fd;
  
  fd = 0;
  cmd[0] = reg;
  fd = open("/sys/EcControl/ECflashread", (O_RDWR|O_NONBLOCK));
  if (fd < 0) {
    puts("peter ==== error: cannot open EC device file!");
    retu = 0;
  }
  else {
    nbytes = write(fd, cmd, 1);
    if (nbytes < 0) {
      puts("peter ===== error: write ec node.");
      retu = 0;
    }
    else {
      fl = lseek(fd, 0, SEEK_SET);
      if (fl < 0) {
        puts("peter ===== error: lseek ec node.");
        retu = 0;
      }
      else {
        nbytes = read(fd, buf, 2);
        if (nbytes < 0) {
          puts("peter ===== error: read ec node.");
          retu = 0;
        }
        else {
          close(fd);
          retu = 1;
        }
      }
    }
  }
  return retu;
}

int write_ec_device(uint8_t reg, void *sndBuf, int sndLen)
{
  ssize_t nbytes;
  int retu;
  uint8_t local_18;
  uint8_t local_17;
  uint8_t local_16;
  int fd;
  fd = 0;
  local_17 = sndBuf[1];
  local_16 = *sndBuf;
  if (sndLen == 1) {
    local_17 = 0;
    local_16 = *sndBuf;
  }
  local_18 = reg;
  fd = open("/sys/EcControl/ECflashwrite", (O_RDWR|O_NONBLOCK));
  if (fd < 0) {
    puts("peter ==== error: cannot open EC device file!");
    retu = 0;
  }
  else {
    nbytes = write(fd, &local_18, 3);
    if (nbytes < 0) {
      puts("peter ===== error: write ec node.");
      retu = 0;
    }
    else {
      close(fd);
      retu = 1;
    }
  }
  return retu;
}

int KBC_CMD_DATAS(uint8_t reg, void *sndBuf, int sndLen, void *rcvBuf, int rcvLen)
{
  int nbytes;
  
  if (sndLen < 1) {
    nbytes = read_ec_device(reg, rcvBuf);
    if (nbytes == 0) {
      puts("peter ==== read_ec_device fail");
      return 0;
    }
  }
  else {
    nbytes = write_ec_device(reg, sndBuf, sndLen);
    if (nbytes == 0) {
      puts("peter ==== write_ec_device fail");
      return 0;
    }
  }
  return 1;
}

uint32_t HexStr2DWORD(char *str)
{
  uint8_t *p_c;
  uint32_t retu;
  
  retu = 0;
  p_c = str;
  do {
    if ('\0' == *p_c) {
      return retu;
    }
    if ((*p_c < '0') || ('9' < *p_c)) 
    {
      if ((*p_c < 'a') || ('f' < *p_c)) 
      {
        if ((*p_c < 'A') || ('F' < *p_c)) {
          return 0;
        }
        else
        {
          retu = retu * 0x10 + (uint32_t)*p_c + 0x0A - 'A';
        }
      }
      else 
      {
        retu = retu * 0x10 + (uint32_t)*p_c + 0x0A - 'a';
      }
    }
    else {
      retu = retu * 0x10 + (uint32_t)*p_c + - '0';
    }
    p_c = p_c + 1;
  } while( true );
  return retu;
}

void display_version(void)

{
  puts("***************************************************************************** \r");
  puts("*  ENE Flash Utility for ARM platform                                       * \r");
  printf("*  V%u.%u.%u                                                   Created by LJC  * \r\n",1,0
         ,2);
  puts("*                                              Modified based on Nick, RYC  * \r");
  puts("***************************************************************************** \r");
  return;
}

void display_usage(void)

{
  puts("\r");
  puts("Usage: EC_flash [-bbl] [-e1] [-e2] RomFileName   \r\n");
  puts(" -bbl  Flash boot block too                      \r");
  puts(" -e1   Erase EC EEPROM 116K - 120K               \r");
  puts(" -e2   Erase system EEPROM 120K - 128K           \r\n");
                    /* WARNING: Subroutine does not return */
  exit(1);
}

uint8_t CRC8_Calculator(uint8_t *data, int32_t length)
{
  uint8_t *data_p;
  uint8_t j;
  uint8_t i;
  uint8_t remainder;
  uint8_t crc;
  
  crc = *data;
  data_p = data;
  for (i = 0; i < length; i++) {
    remainder = *data_p;
    for (j = 0; j < 8; j++) {
      if ((char)crc < '\0') {
        crc = (crc >> 7 | crc << 1) ^ 7;
      }
      else {
        crc = remainder >> 7 | crc << 1;
      }
      remainder = remainder << 1;
    }
    data_p = data_p + 1;
  }
  return crc ^ 0x80;
}

int main (int argc, char *argv[])
{
  uint16_t SendData;
  uint16_t ReData;
  int iVar1;
  char *pcVar2;
  undefined4 uVar3;
  int iVar4;
  char local_84 [80];
  int local_34;
  int arg_cnt;
  char local_27;
  char local_26;
  byte local_25;
  FILE *local_24;
  uint local_20;
  int local_1c;
  int local_18;
  uint local_14;
  
  local_34 = 0;
  gOperation_Volume = 0;
  gFile_Offset = 0;
  filesize = 0;
  pcVar2 = (char *)display_version();
  local_34 = 0;
  local_27 = '\0';
  for (arg_cnt = 1; arg_cnt < argc; arg_cnt = arg_cnt + 1) {
    pcVar2 = (char *)strlwr(*(undefined4 *)(arg_cnt * 4 + argv));
    strcpy(local_84,pcVar2);
    pcVar2 = (char *)strncmp("-d",local_84,2);
    if (pcVar2 == (char *)0x0) {
      local_34 = 2;
      local_27 = '\x01';
    }
    else {
      pcVar2 = (char *)strncmp("-bbl",local_84,4);
      if (pcVar2 == (char *)0x0) {
        gBootLoader = '\x01';
        if (local_34 != 2) {
          local_34 = 1;
        }
      }
      else {
        pcVar2 = (char *)strncmp("-e1",local_84,3);
        if (pcVar2 == (char *)0x0) {
          gEraseBlock = gEraseBlock | 1;
        }
        else {
          pcVar2 = (char *)strncmp("-e2",local_84,3);
          if (pcVar2 == (char *)0x0) {
            gEraseBlock = gEraseBlock | 2;
          }
          else {
            pcVar2 = (char *)strncmp("-nid",local_84,4);
            if (pcVar2 == (char *)0x0) {
              gNid = '\x01';
            }
            else {
              pcVar2 = (char *)strncmp("-ota",local_84,4);
              if (pcVar2 == (char *)0x0) {
                gOTA = '\x01';
              }
              else {
                pcVar2 = (char *)strncmp("-sd",local_84,3);
                if (pcVar2 == (char *)0x0) {
                  gFlashSD = '\x01';
                }
                else if (local_84[0] != '-') {
                  pcVar2 = strcpy(FileName,local_84);
                  if (local_34 != 2) {
                    local_34 = 1;
                  }
                  local_27 = '\x01';
                }
              }
            }
          }
        }
      }
    }
  }
  if ((local_27 == '\0') || (local_34 == 0)) {
    display_usage(pcVar2);
  }
  if (local_34 == 1) {
    printf("File Name : [%s]",FileName);
    local_24 = fopen(FileName,"rb");
    fseek(local_24,0x4000,0);
    fread(&gROM,1,5,local_24);
    if (local_24 == (FILE *)0x0) {
      printf("\r\nOpen File       : %s => Failed !!!\r\n",FileName);
      ExitPro(1,0);
    }
    else {
      fseek(local_24,0,2);
      filesize = ftell(local_24);
      fseek(local_24,0,0);
      printf(",[0x%lX Bytes]\n\r",filesize);
      disable_batterypolling();
      usleep(1000000);
      usleep(1000000);
    }
  }
  if ((local_34 != 2) && (local_34 == 1)) {
    local_14 = 0;
    // read register 0x30
    KBC_CMD_DATAS(0x30,&SendData,0,&ReData,2);
    gRamID = ReData;
    gRamMVer = DAT_00093279;
    usleep(100000);
    // read register 0x31
    KBC_CMD_DATAS(0x31,&SendData,0,&ReData,2);
    gRamSVer = ReData;
    gRamTVer = DAT_00093279;
    printf("File EC Version : V%1X%1XT%1X\n\r",(uint)gROM._1_1_,(uint)gROM._2_1_,(uint)gROM._4_1_);
    printf("Current EC Version : V%1X%1XT%1X\n\r",(uint)gRamMVer,(uint)gRamSVer,(uint)gRamTVer);
    if ((((byte)gROM != gRamID) || (filesize != 0x20000)) && (gNid == '\0')) {
      puts("ROM file was broken!!           \r\n");
      ExitPro(1,0);
    }
    KBC_CMD_DATAS(0xb1,&SendData,0,&ReData,2);
    local_1c = (uint)ReData * 0x100 + (uint)DAT_00093279;
    KBC_CMD_DATAS(0xb0,&SendData,0,&ReData,2);
    local_25 = ReData;
    if ((ReData == 'U') && (gBootLoader != '\0')) {
      gBootLoader = '\0';
      puts("Boot block will not be updated. \r");
    }
    if (gBootLoader == '\0') {
      SendData._0_1_ = 1;
      local_20 = filesize - local_1c;
    }
    else {
      SendData._0_1_ = 2;
      local_20 = filesize;
    }
    SendData._1_1_ = gEraseBlock;
    KBC_CMD_DATAS(0xb7,&SendData,2,&ReData,0);
    usleep(500000);
    local_26 = '\0';
    local_18 = filesize - local_20;
    SendData._0_1_ = (undefined)local_18;
    SendData._1_1_ = (byte)((uint)local_18 >> 8);
    KBC_CMD_DATAS(0xb8,&SendData,2,&ReData,0);
    while (local_20 != 0) {
      if (local_26 != '\0') {
        local_26 = '\0';
        usleep(200000);
        SendData._0_1_ = 1;
        SendData._1_1_ = gEraseBlock;
        KBC_CMD_DATAS(0xb7,&SendData,2,&ReData,0);
        usleep(2000000);
        KBC_CMD_DATAS(0xb0,&SendData,0,&ReData,2);
        local_25 = ReData;
        if (ReData != 0x55) {
          puts("\r");
          puts("Failed to jump boot block. Process stopped! \r");
          break;
        }
        SendData._0_1_ = (undefined)local_1c;
        SendData._1_1_ = (byte)((uint)local_1c >> 8);
        KBC_CMD_DATAS(0xb8,&SendData,2,&ReData,0);
      }
      KBC_CMD_DATAS(0xb6,&SendData,0,&ReData,1);
      if (((ReData == 0) || (ReData == 1)) || (ReData == 2)) {
        uVar3 = __aeabi_uidiv((filesize - local_20) * 100,filesize);
        printf("Flash complete: [%u%%]  \r",uVar3);
        if (ReData == 1) {
          local_20 = local_20 - 0x20;
          if (filesize - local_1c == local_20) {
            local_26 = '\x01';
          }
          else if (local_20 < 0x4001) {
            local_20 = 0;
          }
          if (firstTemp != '\0') {
            temp = temp + 1;
          }
          firstTemp = '\x01';
        }
        if (gBootLoader == '\0') {
          fseek(local_24,temp * 0x20 + local_1c,0);
        }
        else {
          fseek(local_24,temp << 5,0);
        }
        for (local_14 = 0; iVar1 = local_14, (int)local_14 < 0x20; local_14 = local_14 + 1) {
          iVar4 = fgetc(local_24);
          DataArray[iVar1] = (char)iVar4;
        }
        for (local_14 = 0; (int)local_14 < 0x20; local_14 = local_14 + 1) {
          if (local_14 == 0 || (local_14 & 1) == 0) {
            SendData._0_1_ = DataArray[local_14];
          }
          else {
            SendData._1_1_ = DataArray[local_14];
            usleep(100);
            KBC_CMD_DATAS(0xb9,&SendData,2,&ReData,0);
          }
        }
        SendData._0_1_ = CRC8_Calculator(DataArray, 0x20);
        SendData._1_1_ = (byte)temp;
        KBC_CMD_DATAS(0xb9,&SendData,2,&ReData,0);
      }
      usleep(5000);
    }
    printf("Flash complete: [100%%]   \r\n");
    fclose(local_24);
    usleep(50000);
    SendData._0_1_ = 4;
    SendData._1_1_ = 0;
    KBC_CMD_DATAS(0xb7,&SendData,2,&ReData,0);
    usleep(50000);
    if (gOTA == '\0') {
      if (gFlashSD == '\0') {
        SendData._0_1_ = 3;
      }
      else {
        SendData._0_1_ = 2;
      }
    }
    else {
      SendData._0_1_ = 1;
    }
    SendData._1_1_ = 0;
    KBC_CMD_DATAS(0xba,&SendData,2,&ReData,0);
    puts("Flash Finish. Please wait for system restart ... \r\n");
  }
  return 0;
}

