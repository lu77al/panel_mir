#define OFF    0
#define ON     1

#define MEM16(addr) (*(uint8_t *)(addr) | (*(uint8_t *)(addr + 1) << 8))
#define MEM32(addr) (*(uint8_t *)(addr) | (*(uint8_t *)(addr + 1) << 8) | (*(uint8_t *)(addr + 2) << 16) | (*(uint8_t *)(addr + 3) << 24))

#define SAVE_MEM16(addr, val) {*(uint8_t *)(addr) = val; *(uint8_t *)(addr + 1) = val >> 8;}
