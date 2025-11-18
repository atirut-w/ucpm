#include <Z80.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>

Z80 cpu;
uint8_t memory[65536];

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
    std::cerr << "Error: Could not open program file: " << args->program << std::endl;
    return 1;
  }

  for (unsigned addr = 0x0100; addr < sizeof(memory) && !program_file.eof(); addr++) {
    memory[addr] = program_file.get();
  }

  return 0;
}
