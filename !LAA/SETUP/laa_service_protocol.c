/******************************************************************************
 *  Service protocol with CAD
 ******************************************************************************/
#include "laa_service_protocol.h"
#include "laa_interface.h"
#include "laa_components.h"
#include "laa_sdram.h"
#include "laa_sdcard.h"
#include "laa_config.h"
#include "laa_utils.h"
#include "laa_proj_structure.h"
#include "laa_crc16.h"
#include "string.h"
#include "stdlib.h"
#include "usbd_cdc_if.h"

//--- ID record ----
typedef struct TReplyID {
  uint16_t    idcode;
  uint16_t    version;
  uint32_t    crc;
  const char  time[8];
  const char  date[8];
} TReplyID;

const TReplyID repy_id = {
  .idcode =  0x0928,
  .version = 2019,
  .crc =     0xad128b01,
  .time =    "18:45:10",
  .date =    "04/06/19"
};

//--- OREC record ----
typedef struct TOREC {
  uint16_t   offset; // For data alignment
  uint8_t    size;
  uint8_t    version;
  uint32_t   code_addr;
  uint32_t   code_max_size;
  uint32_t   src_addr;
  uint32_t   src_max_size;
} TOREC;

const TOREC orec = {
  .offset =         0,
  .size =           14,
  .version =        1,
  .code_addr =      POJECT_ADDR,
  .code_max_size =  POJECT_END - POJECT_ADDR,
  .src_addr =       0,
  .src_max_size =   0x01000000
};

extern USBD_HandleTypeDef hUsbDeviceFS;

#define SP_RX_SIZE  350
#define SP_TX_SIZE  700
#define SP_BUFFER  ((uint8_t *)(VARS_INTEGER + 16)) // Temporary file buffer pointer
  
static uint8_t rx_buf[SP_RX_SIZE];
// ACK(0x10, 0x06) + STRT(0x10, 0x02) + CC(0x00) (fixed message start)
static uint8_t tx_buf[SP_TX_SIZE] = {0x10, 0x06, 0x10, 0x02, 0x00};
static uint8_t *rx_pnt;   // RX pointer
static uint8_t *tx_pnt;   // TX pointer
static uint8_t checksum;  // Checksum for RX/TX
static uint8_t *tpm_file_ptr;  // Temporary file buffer pointer
   
void spInputStartFirst(uint8_t rxb);  // [10h] 02h ... 10h 03h CS
void spInputStartSecond(uint8_t rxb); // 10h [02h] ... 10h 03h CS
void spInputData(uint8_t rxb);        // 10h 02h [...] 10h 03h CS
void spInputCommand(uint8_t rxb);     // 10h 02h ... 10h [03h] CS
void spInputChecksum(uint8_t rxb);    // 10h 02h ... 10h 03h [CS]

void spRun() {
  USBD_Start(&hUsbDeviceFS);
  uiSPNextByte = spInputStartFirst;
}

void spStop() {
  USBD_Stop(&hUsbDeviceFS);
  uiSPNextByte = 0;
}

void spStartReply() {
  tx_pnt = &tx_buf[5];
  checksum = 0;
}

void spAddReply(const uint8_t *data, uint16_t len) {
  const uint8_t *txb = data;
  for (uint8_t i = 0; i < len; i++) {
    checksum += *txb;
    *(tx_pnt++) = *txb;
    if (*txb == 0x10) {
      *(tx_pnt++) = 0x10;
    }
    txb++;
  }
}

void spSendReply() {
  *(tx_pnt++) = 0x10;
  *(tx_pnt++) = 0x03;
  *(tx_pnt++) = ~checksum + 1;
  CDC_Transmit_FS(tx_buf, tx_pnt - tx_buf);
  uiSPNextByte = spInputStartFirst;
}

void spReply(const uint8_t *data, uint16_t len) {
  spStartReply();
  spAddReply(data, len);
  spSendReply();
}

char *spGetFileName(uint8_t pos, uint16_t len) {
  if (len < pos + 3) return 0;
  if (len != rx_buf[pos] + pos + 1) return 0;
  char *name = strrchr((char *)&rx_buf[pos + 1], '\\');
  if (!name) name = (char *)&rx_buf[pos];
  name++;
  return name;
}

uint32_t spGetFileCRC(uint8_t *data, uint32_t size) {
  memset(&data[size], 0, 4);
  int32_t crc = 0xFFFFFFFF;
  for (uint32_t i = 0; i < size + 4; i++) {
    uint8_t add = data[i];
    for (uint8_t j = 1; j < 8; j++) {
      uint8_t highByte = crc >> 24;
      crc <<= 1;
      if (add & 0x80) crc |= 1;
      add <<= 1;
      if (highByte & 0x80) crc ^= 0x04C11DB7;
    }  
  }
  return crc;
}

void spSendFileChank(uint8_t *data, uint8_t size) {
  uint32_t sav = laaGet24(data - 3);
  uint16_t crc = getCRC16(data, size);
  data[-3] = crc;
  data[-2] = crc >> 8;
  data[-1] = size;
  spReply(data - 3, size + 3);
  laaSet24(data - 3, sav);
}


void spProcesMessage() {
  uiSPProcesMessage = 0;
  uint16_t len = rx_pnt - rx_buf;
  asm("nop");
  switch (rx_buf[0]) {
  case 0x00:    // Request ID
    if (len != 4) break;
    if (memcmp(rx_buf, (const uint8_t[]){0x00, 0xE0, 0x6F, 0x18}, 4)) break;
    spReply((uint8_t *)&repy_id, 24);
    cmpLMPrintLn("������ IDE");
    break;
  case 0x34:   // Request OREC
    if (len != 1) break;
    spReply((uint8_t *)&orec + 2, 18);
    cmpLMPrintLn("������ OREC");
    break;
  case 0x30:   // Read memory
    {
      if (len != 6) break;
      uint8_t size = rx_buf[5];
      if (size == 0) break;
      uint8_t *addr = (uint8_t *)laaGet32(&rx_buf[1]);
      spReply(addr, size);
      if (addr == PROJECT) {
        cmpLMPrintLn("������ ����� �������");
      } else if (addr == prjCRC()) {
        cmpLMPrintLn("������ CRC �������");
      }
      break;
    }  
  case 0x0B:   // Clear panel
    if (len != 1) break;
    spReply(0, 0);
    prjClear();
    cmpLMPrintLn("�������� ������");
    break;
  case 0x2C:   // Delete project SRC
    if (len != 17) break;
    if (memcmp(rx_buf, (const uint8_t[]){0x2C, 0x0F, 0x43, 0x3A, 0x5C, 0x50, 0x50, 0x43,
               0x5C, 0x70, 0x72, 0x6A, 0x2E, 0x70, 0x70, 0x63, 0x00}, 17)) break;
    cmpLMPrintLn("������� �������� ���� �������");
    sdDelete("prj.ppc");
    spReply(0, 0);
    return;
  case 0x32:   // Write memory
    {
      if (len <= 5) break;
      uint16_t size = len - 5;
      uint8_t  *addr = (uint8_t *)laaGet32(&rx_buf[1]);
      if (addr < PROJECT) return;
      if (((uint32_t)addr + size) >= POJECT_END) return;
      memcpy(addr, &rx_buf[5], size);
      spReply(0, 0);
      if (addr == PROJECT) {
        cmpLMPrintLn("������ ������ �������");
      }  
      break;
    }
  case 0x25:   // Save bytecode to SD "mainproj.mpr"
    if (len != 1) break;
    if (prjSave()) {
      cmpLMPrintLnColor("������ �������� �� ����", 0x33FF33);
    } else {
      cmpLMPrintLnColor("������ ���������� ������� �� ����", 0xFF3333);
    }
    spReply(0, 0);
    break;
  case 0x26:   // Create DOS file
    if (len != 1) break;
    cmpLMPrintLn("����� ������ �����");
    tpm_file_ptr = SP_BUFFER;
    spReply(0, 0);
    break;
  case 0x27:   // Write data to DOS file (temporary buffer)
    if (len < 9) break;
    if (len != rx_buf[7] + 8) break;
    memcpy(tpm_file_ptr, &rx_buf[8], rx_buf[7]);
    tpm_file_ptr += rx_buf[7];
    spReply(0, 0);
    break;
  case 0x28:   // Close DOS file (save to SD)
    {
      char *name = spGetFileName(9, len);
      if (!name) break;
      if (sdWriteFile(name, SP_BUFFER, tpm_file_ptr - SP_BUFFER)) {
        cmpLMPrintColor("�������� �� ���� ", 0x33FF33);
        cmpLMPrintLnColor(name, 0x33FF33);
      } else {
        cmpLMPrintColor("������ ���������� ", 0xFF3333);
        cmpLMPrintLnColor(name, 0xFF3333);
      }
      spReply(0, 0);
      break;
    }
  case 0x29:   // Open DOS file for read (read it in temporary buf, calculate CRC)
    {
      char *name = spGetFileName(1, len);
      if (!name) break;
      uint32_t size = sdReadFile(name, SP_BUFFER);
      if (size) {
        uint32_t rep[3];
        rep[0] = size;
        rep[1] = 0xffffffff;
        rep[2] = spGetFileCRC(SP_BUFFER, size);
        spReply((uint8_t *)rep, 12);
        cmpLMPrint("������ ����� ");
        cmpLMPrintLn(name);
      } else {
        cmpLMPrintColor("������ ������ ", 0xFF3333);
        cmpLMPrintLnColor(name, 0xFF3333);
      }
      break;
    }
  case 0x2A:   // Read (send to sp) chunk of the openned DOS file
    if (len != 6) break;
    spSendFileChank(SP_BUFFER + laaGet32(&rx_buf[1]), rx_buf[5]);
    break;
  case 0x2B:   // Close DOS file (after reading)
    if (len != 1) break;
    cmpLMPrintLn("������ ���������");
    spReply(0, 0);
    break;
  }
}

//************** Line Data Handlers ****************
void spInputStartFirst(uint8_t rxb) {  // [10h] 02h ... 10h 03h CS
  if (rxb == 0x10) {
    uiSPNextByte = spInputStartSecond;
  }
}

void spInputStartSecond(uint8_t rxb) {  // 10h [02h] ... 10h 03h CS
  if (rxb == 0x02) {
    uiSPNextByte = spInputData;
    rx_pnt = rx_buf;
    checksum = 0;
  } else {
    uiSPNextByte = spInputStartFirst;
  }
}

void spInputData(uint8_t rxb) {         // 10h 02h [...] 10h 03h CS
  if (rxb == 0x10) {
    uiSPNextByte = spInputCommand;
  } else {
    if (rx_pnt < &rx_buf[SP_RX_SIZE]) {
      *(rx_pnt++) = rxb;
      checksum += rxb;
    } else {
      uiSPNextByte = spInputStartFirst;
    }
  }
}

void spInputCommand(uint8_t rxb) {      // 10h 02h ... 10h [03h] CS
  if (rxb == 0x03) {
    uiSPNextByte = spInputChecksum;
    checksum = ~checksum + 1;
  } else if (rxb == 0x10) { // Stuffing inside data
    if (rx_pnt < &rx_buf[SP_RX_SIZE]) {
      *(rx_pnt++) = rxb;
      checksum += rxb;
      uiSPNextByte = spInputData;
    } else {
      uiSPNextByte = spInputStartFirst;
    }
  }
}

void spInputChecksum(uint8_t rxb) {      // 10h 02h ... 10h 03h [CS]
  if (checksum == rxb) {
    uiSPNextByte = 0; // Don't read data until process current message
    uiSPProcesMessage = spProcesMessage;
  } else {
    uiSPNextByte = spInputStartFirst;
  }
}

