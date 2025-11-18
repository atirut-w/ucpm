#include <algorithm>
#include <bdos.hpp>
#include <cstdint>
#include <cstring>
#include <format>
#include <fstream>
#include <iostream>
#include <machine.hpp>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <unordered_map>
#include <algorithm>

static std::unordered_map<std::string, std::fstream> open_files;

static std::string get_filename_from_fcb(const FileControlBlock &fcb) {
  std::string name = std::format("{:.8}", (char *)fcb.f);
  std::string type = std::format("{:.3}", (char *)fcb.t);

  name.erase(name.find_last_not_of(' ') + 1);
  type.erase(type.find_last_not_of(' ') + 1);

  return std::format("{}.{}", name, type);
}

static bool console_has_char() {
  fd_set set;
  struct timeval tv = {0, 0};
  FD_ZERO(&set);
  FD_SET(STDIN_FILENO, &set);

  return select(STDIN_FILENO + 1, &set, nullptr, nullptr, &tv) > 0;
}

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
    case C_RAWIO:
      switch (arg & 0xff) {
      case 0xff:
        if (!console_has_char()) {
          result = 0; // No character available
        }

        char ch;
        if (read(STDIN_FILENO, &ch, 1) == 1) {
          if (ch == '\n') {
            ch = '\r';
          }
          result = ch;
        } else {
          result = 0; // No character read
        }

        break;
      case 0xfe:
      case 0xfd:
      case 0xfc:
        std::cout << std::format("Fatal: unsupported direct console I/O {}",
                                 (int)(arg & 0xff))
                  << std::endl;
        machine.running = false;
        break;
      default:
        std::cout << (char)(arg & 0xff) << std::flush;
        break;
      }
      break;
    case C_READSTR: {
      char buffer[256];
      uint8_t index = 0;

      while (index < 255) {
        char ch;
        if (read(STDIN_FILENO, &ch, 1) != 1) {
          continue;
        }
        if (ch == '\r' || ch == '\n') {
          break;
        }
        if (ch == '\b' || ch == 0x7f) {
          if (index > 0) {
            index--;
            std::cout << "\b \b" << std::flush;
          }
          continue;
        }
        buffer[index++] = ch;
        std::cout << ch << std::flush;
      }
      buffer[index] = '$';

      uint8_t size;
      uint8_t len;

      machine.memout(&size, arg, 1);
      if (arg == 0) {
        uint8_t size = 128;
        machine.memin(machine.dma_address, &size, 1);
        machine.memin(machine.dma_address + 1, &index, 1);
        machine.memin(machine.dma_address + 2, buffer, std::min<int>(128, sizeof(buffer)));
      } else {
        len = std::min<uint8_t>(size - 2, index);
        machine.memin(arg + 1, &len, 1);
        machine.memin(arg + 2, buffer, len);
      }

      result = 0; // Success
      break;
    }
    case DRV_ALLRESET:
      result = 0; // Success
      break;
    case DRV_SET:
      result = arg == 0 ? 0 : 0x00ff; // Success if drive A
      break;
    case F_OPEN: {
      FileControlBlock fcb;
      machine.memout(&fcb, arg, sizeof(fcb));

      std::string path = get_filename_from_fcb(fcb);
      if (open_files.find(path) != open_files.end()) {
        result = (FileAlreadyOpen << 8) | 0xff;
        break;
      }
      if (path.find('?') != std::string::npos) {
        result = (FilenameContainsWildcard << 8) | 0xff;
        break;
      }
      // TODO: More robust error checking?

      std::fstream file;
      file.open(path, std::ios::in | std::ios::out | std::ios::binary);
      if (!file.is_open()) {
        result = (SoftwareError << 8) | 0xff;
        break;
      }

      open_files[path] = std::move(file);
      result = 0; // Success
      break;
    }
    case F_CLOSE: {
      FileControlBlock fcb;
      machine.memout(&fcb, arg, sizeof(fcb));

      std::string path = get_filename_from_fcb(fcb);
      if (open_files.find(path) == open_files.end()) {
        result = 0xff; // File not open
        break;
      }

      open_files[path].close();
      open_files.erase(path);
      result = 0; // Success
      break;
    }
    case F_DELETE: {
      FileControlBlock fcb;
      machine.memout(&fcb, arg, sizeof(fcb));

      std::string path = get_filename_from_fcb(fcb);

      // TODO: Wildcards support
      if (path.find('?') != std::string::npos) {
        result = (FilenameContainsWildcard << 8) | 0xff;
        break;
      }

      if (open_files.find(path) != open_files.end()) {
        result = (FileAlreadyOpen << 8) | 0xff;
        break;
      }

      if (std::remove(path.c_str()) != 0) {
        result = 0xff;
        break;
      }

      result = 0; // Success
      break;
    }
    case F_READ: {
      FileControlBlock fcb;
      machine.memout(&fcb, arg, sizeof(fcb));

      std::string path = get_filename_from_fcb(fcb);
      if (open_files.find(path) == open_files.end()) {
        result = 9; // Invalid FCB
      }

      std::fstream &file = open_files[path];

      char buffer[128];
      file.read(buffer, 128);
      machine.memin(machine.dma_address, buffer, 128);

      result = 0; // Success
      break;
    }
    case F_WRITE: {
      FileControlBlock fcb;
      machine.memout(&fcb, arg, sizeof(fcb));

      std::string path = get_filename_from_fcb(fcb);
      if (open_files.find(path) == open_files.end()) {
        result = 9; // Invalid FCB
      }

      std::fstream &file = open_files[path];

      char buffer[128];
      machine.memout(buffer, machine.dma_address, 128);
      file.write(buffer, 128);

      result = 0; // Success
      break;
    }
    case F_MAKE: {
      FileControlBlock fcb;
      machine.memout(&fcb, arg, sizeof(fcb));

      std::string path = get_filename_from_fcb(fcb);
      if (open_files.find(path) != open_files.end()) {
        result = (FileAlreadyExists << 8) | 0xff;
        break;
      }

      if (path.contains('?')) {
        result = (FilenameContainsWildcard << 8) | 0xff;
        break;
      }

      std::fstream file;
      file.open(path, std::ios::out | std::ios::binary);
      if (!file.is_open()) {
        result = (SoftwareError << 8) | 0xff;
        break;
      }

      file.close();
      result = 0; // Success
      break;
    }
    case DRV_GET:
      result = 0; // Always drive A
      break;
    case F_READRAND: {
      FileControlBlock fcb;
      machine.memout(&fcb, arg, sizeof(fcb));

      std::string path = get_filename_from_fcb(fcb);
      if (open_files.find(path) == open_files.end()) {
        result = 9; // Invalid FCB
      }

      std::fstream &file = open_files[path];

      int recnum = 0;
      recnum |= fcb.r[0];
      recnum |= fcb.r[1] << 8;
      recnum |= fcb.r[2] << 16;

      file.seekg(recnum * 128, std::ios::beg);
      char buffer[128];
      file.read(buffer, 128);
      machine.memin(machine.dma_address, buffer, 128);

      result = 0; // Success
      break;
    }
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
