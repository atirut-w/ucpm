#include <filesystem>
#include <iostream>
#include <optional>

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

  std::cout << "Program path: " << args->program << std::endl;
  return 0;
}
