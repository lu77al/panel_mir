/******************************************************************************
 *  Service protocol with CAD
 ******************************************************************************/
#include "laa_service_protocol.h"
#include "string.h"
#include "stdlib.h"
#include "usbd_cdc_if.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

#define SP_RX_SIZE  350
#define SP_TX_SIZE  700
   
static uint8_t rx_buf[SP_RX_SIZE];
static uint8_t tx_buf[SP_TX_SIZE];
static uint8_t *rx_pnt;   // RX pointer
static uint8_t *tx_pnt;   // TX pointer
static uint8_t checksum;  // Checksum for RX process

void (*uiSPNextByte)() = 0;


void spRun() {
  USBD_Start(&hUsbDeviceFS);
}

void spStop() {
  USBD_Stop(&hUsbDeviceFS);
}


#ifdef nothing   
   
//*** Line Data Handlers ***
void spInputStartFirst(uint8_t rxb);  // [10h] 02h ... 10h 03h CS
void spInputStartSecond(uint8_t rxb); // 10h [02h] ... 10h 03h CS
void spInputData(uint8_t rxb);        // 10h 02h [...] 10h 03h CS
void spInputCommand(uint8_t rxb);     // 10h 02h ... 10h [03h] CS
void spInputChecksum(uint8_t rxb);    // 10h 02h ... 10h 03h [CS]

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
    *(rx_pnt++) = rxb;
    checksum += rxb;
  }
}

void spInputCommand(uint8_t rxb) {      // 10h 02h ... 10h [03h] CS
  if (rxb == 0x03) {
    uiSPNextByte = spInputChecksum;
  } else if (rxb == 0x10) { // Stuffing inside data
    *(rx_pnt++) = rxb;
    checksum += rxb;
    uiSPNextByte = spInputData;
  }
}

void spInputChecksum(uint8_t rxb) {      // 10h 02h ... 10h 03h [CS]
  if (checksum == rxb) {
    uiSPNextByte = 0; // Don't read data until process current message
  } else {
    uiSPNextByte = spInputStartFirst;
  }
}

#endif