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
    case P_TERMCPM:
      machine.running = false;
      break;
    case C_RAWIO: {
      uint8_t code = static_cast<uint8_t>(arg & 0xff);
      switch (code) {
      case 0xff: {
        // CP/M 2.2/3: Non-blocking raw console read, no echo
        if (!console_has_char()) {
          result = 0; // No character available
        } else {
          char ch;
          if (read(STDIN_FILENO, &ch, 1) == 1) {
            if (ch == '\n') {
              ch = '\r';
            }
            result = static_cast<uint8_t>(ch);
          } else {
            result = 0; // No character read
          }
        }
        break;
      }
      case 0xfe:
      case 0xfd:
      case 0xfc:
        // For now, treat unsupported variants as simple console output,
        // per CP/M rule that unsupported codes output the character.
        std::cout << static_cast<char>(code) << std::flush;
        result = code;
        break;
      default:
        // Values of E not supported output the character
        std::cout << static_cast<char>(code) << std::flush;
        result = code;
        break;
      }
      break;
    }
    case C_READSTR: {
      // CP/M 2.2/3 buffered console input:
      // buffer[0] = max size, buffer[1] = current length, buffer[2..] = data
      char buffer[256];
      uint8_t index = 0;

      // Simple line editor: echo, handle BS/DEL, stop on CR/LF
      while (index < sizeof(buffer)) {
        char ch;
        if (read(STDIN_FILENO, &ch, 1) != 1) {
          continue;
        }
        if (ch == '\r' || ch == '\n') {
          break;
        }
        if (ch == '\b' || static_cast<unsigned char>(ch) == 0x7f) {
          if (index > 0) {
            index--;
            std::cout << "\b \b" << std::flush;
          }
          continue;
        }
        buffer[index++] = ch;
        std::cout << ch << std::flush;
      }

      uint8_t size = 0;

      if (arg == 0) {
        // Use DMA buffer; read size from DMA[0]
        machine.memout(&size, machine.dma_address, 1);
        if (size < 2) {
          size = 2;
        }
        uint8_t len = std::min<uint8_t>(static_cast<uint8_t>(size - 2), index);
        machine.memin(machine.dma_address + 1, &len, 1);
        if (len > 0) {
          machine.memin(machine.dma_address + 2, buffer, len);
        }
      } else {
        // Use buffer at DE; read size from [DE]
        machine.memout(&size, arg, 1);
        if (size < 2) {
          size = 2;
        }
        uint8_t len = std::min<uint8_t>(static_cast<uint8_t>(size - 2), index);
        machine.memin(arg + 1, &len, 1);
        if (len > 0) {
          machine.memin(arg + 2, buffer, len);
        }
      }

      result = 0; // Success
      break;
    }
    case DRV_ALLRESET:
      // Single-drive system: nothing to reset, report success
      result = 0;
      break;
    case DRV_SET:
      // Only drive A: (0) is valid; others return 0xFF in A
      result = (arg == 0) ? 0x0000 : 0x00ff;
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
        // Wildcards not allowed for F_OPEN
        result = (FilenameContainsWildcard << 8) | 0xff;
        break;
      }

      std::fstream file;
      file.open(path, std::ios::in | std::ios::out | std::ios::binary);
      if (!file.is_open()) {
        // File not found or other software-level error
        result = (SoftwareError << 8) | 0xff;
        break;
      }

      open_files[path] = std::move(file);
      // For CP/M 2/3 compatibility, sequential position fields (EX,RC,CR)
      // should be initialized to 0; assume caller did so.
      result = 0; // Success (A=0)
      break;
    }
    case F_CLOSE: {
      FileControlBlock fcb;
      machine.memout(&fcb, arg, sizeof(fcb));

      std::string path = get_filename_from_fcb(fcb);
      auto it = open_files.find(path);
      if (it == open_files.end()) {
        // File not open: treat as software-level error (A=0xFF, H=0)
        result = 0x00ff;
        break;
      }

      it->second.close();
      open_files.erase(it);
      result = 0; // Success
      break;
    }
    case F_DELETE: {
      FileControlBlock fcb;
      machine.memout(&fcb, arg, sizeof(fcb));

      std::string path = get_filename_from_fcb(fcb);

      // NOTE: CP/M F_DELETE allows wildcards; this simple implementation
      // treats the name literally and does not implement wildcard matching.
      // Do NOT reject '?' here, to remain compatible with software that
      // passes wildcards.

      if (open_files.find(path) != open_files.end()) {
        // CP/M would normally not allow deleting open files
        result = (FileAlreadyOpen << 8) | 0xff;
        break;
      }

      if (std::remove(path.c_str()) != 0) {
        // File not found or other software-level error
        result = (SoftwareError << 8) | 0xff;
        break;
      }

      result = 0; // Success
      break;
    }
    case F_READ: {
      FileControlBlock fcb;
      machine.memout(&fcb, arg, sizeof(fcb));

      std::string path = get_filename_from_fcb(fcb);
      auto it = open_files.find(path);
      if (it == open_files.end()) {
        result = 9; // Invalid FCB / file not open
        break;
      }

      std::fstream &file = it->second;

      char buffer[128];
      file.read(buffer, sizeof(buffer));
      std::streamsize got = file.gcount();

      if (got <= 0) {
        // End of file
        result = 1; // EOF
        break;
      }

      if (got < static_cast<std::streamsize>(sizeof(buffer))) {
        std::memset(buffer + got, 0, sizeof(buffer) - static_cast<size_t>(got));
      }

      machine.memin(machine.dma_address, buffer, sizeof(buffer));

      // For full CP/M compatibility, EX/RC/CR in the FCB should be updated
      // to track sequential position; this implementation omits that logic.
      result = 0; // Success
      break;
    }
    case F_WRITE: {
      FileControlBlock fcb;
      machine.memout(&fcb, arg, sizeof(fcb));

      std::string path = get_filename_from_fcb(fcb);
      auto it = open_files.find(path);
      if (it == open_files.end()) {
        result = 9; // Invalid FCB / file not open
        break;
      }

      std::fstream &file = it->second;

      char buffer[128];
      machine.memout(buffer, machine.dma_address, sizeof(buffer));
      file.write(buffer, sizeof(buffer));

      if (!file.good()) {
        // Treat as software-level error (disk full etc.)
        result = (SoftwareError << 8) | 0xff;
        break;
      }

      // As with F_READ, EX/RC/CR in the FCB should track sequential position.
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

      if (path.find('?') != std::string::npos) {
        // Wildcards not allowed for F_MAKE
        result = (FilenameContainsWildcard << 8) | 0xff;
        break;
      }

      // CP/M F_MAKE creates and opens the file for subsequent writes.
      {
        std::fstream file;
        file.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!file.is_open()) {
          result = (SoftwareError << 8) | 0xff;
          break;
        }
        file.close();
      }

      std::fstream rwfile;
      rwfile.open(path, std::ios::in | std::ios::out | std::ios::binary);
      if (!rwfile.is_open()) {
        // Creation succeeded but reopen for R/W failed: treat as software error
        result = (SoftwareError << 8) | 0xff;
        break;
      }

      open_files[path] = std::move(rwfile);
      // On a real CP/M system, FCB allocation info would be initialized here.
      // The file is now open at record 0 for sequential F_WRITE calls.
      result = 0; // Success
      break;
    }
    case DRV_GET:
      // Always drive A: (0)
      result = 0;
      break;
    case F_READRAND: {
      FileControlBlock fcb;
      machine.memout(&fcb, arg, sizeof(fcb));

      std::string path = get_filename_from_fcb(fcb);
      auto it = open_files.find(path);
      if (it == open_files.end()) {
        result = 9; // Invalid FCB / file not open
        break;
      }

      std::fstream &file = it->second;

      uint32_t recnum = 0;
      recnum |= static_cast<uint32_t>(fcb.r[0]);
      recnum |= static_cast<uint32_t>(fcb.r[1]) << 8;
      recnum |= static_cast<uint32_t>(fcb.r[2]) << 16;

      std::streamoff offset = static_cast<std::streamoff>(recnum) * 128;
      file.clear();
      file.seekg(offset, std::ios::beg);
      if (!file.good()) {
        // Record out of range
        result = 6;
        break;
      }

      char buffer[128];
      file.read(buffer, sizeof(buffer));
      std::streamsize got = file.gcount();

      if (got <= 0) {
        // Reading unwritten data / beyond EOF
        result = 1;
        break;
      }
      if (got < static_cast<std::streamsize>(sizeof(buffer))) {
        std::memset(buffer + got, 0, sizeof(buffer) - static_cast<size_t>(got));
      }

      machine.memin(machine.dma_address, buffer, sizeof(buffer));

      // Real BDOS would also update EX/RC/CR to match this random position.
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

    // CP/M 2+/3 style return:
    // A = low byte; HL = full 16-bit result; H may carry CP/M 3 error code.
    machine.cpu.af.uint8_array[1] = result & 0xFF;
    machine.cpu.bc.uint8_array[1] = (result >> 8) & 0xFF;
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
