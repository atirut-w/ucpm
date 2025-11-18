#include <bdos.hpp>
#include <cstring>
#include <format>
#include <iostream>
#include <machine.hpp>

static zuint8 read_memory(void *ctx, zuint16 address) {
  return static_cast<Machine *>(ctx)->memory[address];
}

static void write_memory(void *context, zuint16 address, zuint8 value) {
  static_cast<Machine *>(context)->memory[address] = value;
}

static zuint8 fetch_opcode(void *context, zuint16 address) {
  Machine &machine = *static_cast<Machine *>(context);

  if (!address) {
    machine.running = false;
    return 0x00; // NOP
  } else if (address == 5) {
    uint8_t func = machine.cpu.bc.uint8_array[0];
    uint16_t arg = machine.cpu.de.uint16_value;
    uint16_t result = 0;

    switch (func) {
    case DRV_SET:
      result = arg == 0 ? 0 : 0x00ff; // Success if drive A
      break;
    case F_OPEN: {
      FileControlBlock fcb;
      machine.memout(&fcb, arg, sizeof(fcb));
      std::string filename;
    }
    case DRV_GET:
      result = 0; // Always drive A
      break;
    default:
      std::cout << std::format(
                       "Fatal: unknown BDOS system call {} with argument {}",
                       func, arg)
                << std::endl;
      machine.running = false;
      break;
    }

    machine.cpu.af.uint8_array[1] = result & 0xFF;
    machine.cpu.bc.uint8_array[0] = (result >> 8) & 0xFF;
    machine.cpu.hl.uint16_value = result;
    return 0xC9; // RET
  }
  return machine.memory[address];
}

Machine::Machine() {
  memset(&cpu, 0, sizeof(cpu));
  cpu.context = this;
  cpu.pc.uint16_value = 0x0100;
  cpu.sp.uint16_value = 0;

  cpu.fetch_opcode = fetch_opcode;
  cpu.fetch = read_memory;
  cpu.read = read_memory;
  cpu.write = write_memory;
}

void Machine::memin(uint16_t dest, void *src, uint16_t count) {
  for (uint16_t i = 0; i < count; i++) {
    memory[dest + i] = ((uint8_t *)src)[i];
  }
}

void Machine::memout(void *dest, uint16_t src, uint16_t count) {
  for (uint16_t i = 0; i < count; i++) {
    ((uint8_t *)dest)[i] = memory[src + i];
  }
}
