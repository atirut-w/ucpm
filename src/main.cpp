#include "machine.hpp"
#include <Z/types/integral.h>
#include <Z80.h>
#include <bdos.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <termios.h>
#include <unistd.h>

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

  Machine machine;
  machine.init_cpm_zero_page();

  for (unsigned addr = 0x0100; addr < sizeof(machine.memory) && !program_file.eof();
       addr++) {
    machine.memory[addr] = program_file.get();
  }

  // We need to disable canonical mode and echoing for proper console I/O
  termios old;
  tcgetattr(STDIN_FILENO, &old);
  termios newt = old;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);

  while (machine.running) {
    z80_execute(&machine.cpu, 1);
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &old);

  return 0;
}
