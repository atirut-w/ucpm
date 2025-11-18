#include <Z/types/integral.h>
#include <Z80.h>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>

Z80 cpu;
uint8_t memory[65536];
bool running = true;

zuint8 read_memory(void *context, zuint16 address) { return memory[address]; }

void write_memory(void *context, zuint16 address, zuint8 value) {
  memory[address] = value;
}

zuint8 fetch_opcode(void *context, zuint16 address) {
  if (!address) {
    running = false;
    return 0x00; // NOP
  } else if (address == 5) {
    uint8_t func = cpu.bc.uint8_array[0];
    uint16_t arg = cpu.de.uint16_value;
    uint16_t result = 0;

    switch (func) {
    default:
      std::cout << std::format("Fatal: unknown BDOS system call {}", func)
                << std::endl;
      running = false;
      break;
    }

    cpu.af.uint8_array[1] = result & 0xFF;
    cpu.bc.uint8_array[0] = (result >> 8) & 0xFF;
    cpu.hl.uint16_value = result;
    return 0xC9; // RET
  }
  return memory[address];
}

struct Args {
  std::filesystem::path program;
};

std::optional<Args> parse_args(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <program_path> [...]" << std::endl;
    return std::nullopt;
  }
  Args args;
  args.program = std::filesystem::path(argv[1]);
  return args;
}

int main(int argc, char *argv[]) {
  std::optional<Args> args = parse_args(argc, argv);
  if (!args) {
    return 1;
  }

  std::ifstream program_file(args->program, std::ios::binary);
  if (!program_file) {
    std::cerr << "Error: Could not open program file: " << args->program
              << std::endl;
    return 1;
  }

  for (unsigned addr = 0x0100; addr < sizeof(memory) && !program_file.eof();
       addr++) {
    memory[addr] = program_file.get();
  }

  memset(&cpu, 0, sizeof(cpu));
  cpu.pc.uint16_value = 0x0100;
  cpu.sp.uint16_value = 0;

  cpu.fetch_opcode = fetch_opcode;
  cpu.fetch = read_memory;
  cpu.read = read_memory;
  cpu.write = write_memory;

  while (running) {
    z80_execute(&cpu, 1);
  }

  return 0;
}
