#pragma once
#include "Z80.h"
#include <cstdint>

struct Machine {
  Z80 cpu;
  uint8_t memory[65536];
  bool running = true;
  uint16_t dma_address = 0x80;

  Machine();

  void init_cpm_zero_page();
  void memin(uint16_t dest, void *src, uint16_t count);
  void memout(void *dest, uint16_t src, uint16_t count);
};
