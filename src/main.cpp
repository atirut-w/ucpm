#include <Z/types/integral.h>
#include <Z80.h>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>

enum FuncNum {
  // CP/M 1,2,3 core BDOS calls
  P_TERMCPM = 0, // 0 - System reset / terminate (does not return)
  C_READ = 1,    // 1 - Console input (echoed)
  C_WRITE = 2,   // 2 - Console output
  A_READ = 3,    // 3 - Auxiliary / reader input (or raw console on MP/M)
  A_WRITE = 4,   // 4 - Auxiliary / punch output (or raw console on MP/M)
  L_WRITE = 5,   // 5 - Printer output (list device)
  C_RAWIO = 6,   // 6 - Direct console I/O (CP/M 2+ semantics)
  // 7 and 8 differ by CP/M version:
  //  - CP/M 1/2: get/set IOBYTE
  //  - CP/M 3+: auxiliary status
  A_STATIN = 7,   // 7 - Aux input status (CP/M 3+), or Get I/O byte (CP/M 1/2)
  A_STATOUT = 8,  // 8 - Aux output status (CP/M 3+), or Set I/O byte (CP/M 1/2)
  C_WRITESTR = 9, // 9  - Output $-terminated string
  C_READSTR = 10, // 10 - Buffered console input
  C_STAT = 11,    // 11 - Console status
  S_BDOSVER = 12, // 12 - Return BDOS version / system type (CP/M 2+)
  DRV_ALLRESET = 13, // 13 - Reset all drives
  DRV_SET = 14,      // 14 - Select disk
  F_OPEN = 15,       // 15 - Open file
  F_CLOSE = 16,      // 16 - Close file
  F_SFIRST = 17,     // 17 - Search first
  F_SNEXT = 18,      // 18 - Search next
  F_DELETE = 19,     // 19 - Delete file
  F_READ = 20,       // 20 - Sequential read (next record)
  F_WRITE = 21,      // 21 - Sequential write (next record)
  F_MAKE = 22,       // 22 - Create file
  F_RENAME = 23,     // 23 - Rename file
  DRV_LOGINVEC = 24, // 24 - Bitmap of logged‑in drives
  DRV_GET = 25,      // 25 - Get current drive
  F_DMAOFF = 26,     // 26 - Set DMA address
  DRV_ALLOCVEC = 27, // 27 - Get allocation bitmap address
  DRV_SETRO = 28,    // 28 - Set current drive read‑only (software)
  DRV_ROVEC = 29,    // 29 - Bitmap of read‑only drives

  // 30 is heavily version‑dependent; in CP/M 2+ it is file attributes
  F_ATTRIB = 30,    // 30 - Set file attributes (CP/M 2+)
  DRV_DPB = 31,     // 31 - Get DPB (disk parameter block) address
  F_USERNUM = 32,   // 32 - Get/set user number
  F_READRAND = 33,  // 33 - Random read
  F_WRITERAND = 34, // 34 - Random write
  F_SIZE = 35,      // 35 - Compute file size (set random record count)
  F_RANDREC = 36,   // 36 - Set random record from last seq op
  DRV_RESET = 37,   // 37 - Selective disk reset

  // MP/M, CP/M 3, and higher 8‑bit extensions
  DRV_ACCESS = 38, // 38 - Access drives (lock drive bitmap)      [MP/M, CP/Net]
  DRV_FREE = 39,   // 39 - Free drive locks                       [MP/M, CP/Net]
  F_WRITEZF = 40,  // 40 - Random write with zero fill           [CP/M 2.2+]
  F_TESTWRITE =
      41,      // 41 - Test and write record                 [MP/M, Concurrent]
  F_LOCK = 42, // 42 - Lock record(s)                         [MP/M, CP/Net]
  F_UNLOCK = 43, // 43 - Unlock record(s)                      [MP/M, CP/Net]
  F_MULTISEC =
      44, // 44 - Set multisector count                 [MP/M II+, CP/M 3+]
  F_ERRMODE =
      45, // 45 - Set hardware error handling mode      [Personal CP/M, MP/M]
  DRV_SPACE = 46, // 46 - Get free space on drive               [MP/M II+]
  P_CHAIN =
      47, // 47 - Chain to another program              [MP/M II+, some 8‑bit]
  DRV_FLUSH =
      48,     // 48 - Flush/empty disk buffers              [Personal, MP/M II+]
  S_SCB = 49, // 49 - Access System Control Block           [CP/M 3]
  S_BIOS = 50, // 50 - Call BIOS via BDOS                    [CP/M 3+]
  // 51–58 are 16‑bit memory management in CP/M‑86 (not 8‑bit CALL 5)

  // Z80DOS / ZPM3 time‑stamp extensions (still 8‑bit BDOS)
  Z80_GETSTAMP =
      54, // 54 - Get file timestamp buffer             [Z80DOS, ZPM3]
  Z80_USESTAMP =
      55, // 55 - Use timestamp buffer on next call     [Z80DOS, ZPM3]

  // Loader / RSX on CP/M 3+ (8‑bit BDOS)
  P_LOAD = 59,   // 59 - Load overlay / PRL / RSX              [CP/M 3+ loader]
  RSX_CALL = 60, // 60 - Call RSX                              [CP/M 3+ RSX]

  // CP/Net calls (8‑bit BDOS)
  N_LOGIN = 64,   // 64 - Log in to server                      [CP/Net]
  N_LOGOFF = 65,  // 65 - Log off                               [CP/Net]
  N_SENDMSG = 66, // 66 - Send network message                  [CP/Net]
  N_RECVMSG = 67, // 67 - Receive network message               [CP/Net]
  N_STATUS = 68,  // 68 - Get network status                    [CP/Net]
  N_CONFIG = 69,  // 69 - Get CP/Net configuration table        [CP/Net]
  N_COMPAT =
      70, // 70 - Set compatibility attributes          [CP/Net‑86, some 8‑bit]
  N_SERVERCONF =
      71, // 71 - Get server configuration              [CP/Net‑86, some 8‑bit]

  // CP/M 3 / MP/M II+ time, date, label, password, serial, console
  F_CLEANUP =
      98, // 98 - Clean up disk on program exit         [CP/M 3 internal]
  F_TRUNCATE = 99,    // 99 - Truncate file                         [CP/M 3+]
  DRV_SETLABEL = 100, // 100 - Set directory label                  [MP/M II+]
  DRV_GETLABEL = 101, // 101 - Get directory label flags byte       [MP/M II+]
  F_TIMEDATE = 102,   // 102 - Get file date/time                   [MP/M II+]
  F_WRITEXFCB = 103,  // 103 - Set file password/protection         [MP/M II+]
  T_SET = 104, // 104 - Set system date/time                 [MP/M II+, Z80DOS,
               // DOS+]
  T_GET = 105, // 105 - Get system date/time                 [MP/M II+, Z80DOS,
               // DOS+]
  F_PASSWD =
      106, // 106 - Set default password                 [MP/M II+, CP/Net]
  S_SERIAL = 107, // 107 - Get system serial number             [MP/M II+]
  P_CODE = 108,   // 108 - Get/set program return code          [CP/M 3+]
  C_MODE =
      109, // 109 - Get/set console mode                 [CP/M 3+, Personal]
  C_DELIMIT =
      110, // 110 - Get/set BDOS string delimiter        [CP/M 3+, Personal]
  C_WRITEBLK =
      111, // 111 - Write block of text to console       [CP/M 3+, Personal]
  L_WRITEBLK =
      112, // 112 - Write block of text to printer       [CP/M 3+, Personal]
  // 113 is Personal CP/M direct screen functions via BIOS
  // 115 is GSX, 8‑bit but very system‑specific

  // MP/M / Concurrent CP/M process, queue, and device functions (8‑bit)
  M_ALLOC_ABS = 128,  // 128 - Allocate absolute memory             [MP/M]
  M_ALLOC = 129,      // 129 - Allocate relocatable memory          [MP/M]
  M_FREE = 130,       // 130 - Free memory                          [MP/M]
  DEV_POLL = 131,     // 131 - Poll I/O device                      [MP/M]
  DEV_WAITFLAG = 132, // 132 - Wait on system flag                  [MP/M]
  DEV_SETFLAG = 133,  // 133 - Set system flag                      [MP/M]
  Q_MAKE = 134,       // 134 - Create message queue                 [MP/M]
  Q_OPEN = 135,       // 135 - Open message queue                   [MP/M]
  Q_DELETE = 136,     // 136 - Delete message queue                 [MP/M]
  Q_READ = 137,       // 137 - Read from message queue              [MP/M]
  Q_CREAD = 138,      // 138 - Conditional read from queue          [MP/M]
  Q_WRITE = 139,      // 139 - Write to message queue               [MP/M]
  Q_CWRITE = 140,     // 140 - Conditional write to queue           [MP/M]
  P_DELAY = 141,      // 141 - Delay                                [MP/M]
  P_DISPATCH = 142,   // 142 - Yield / call dispatcher              [MP/M]
  P_TERM = 143,       // 143 - Terminate process                    [MP/M]
  P_CREATE = 144,     // 144 - Create subprocess                    [MP/M]
  P_PRIORITY = 145,   // 145 - Set process priority                 [MP/M]
  C_ATTACH = 146,     // 146 - Attach console                       [MP/M]
  C_DETACH = 147,     // 147 - Detach console                       [MP/M]
  C_SET = 148,        // 148 - Set default console number           [MP/M]
  C_ASSIGN = 149,     // 149 - Assign console to process            [MP/M]
  P_CLI = 150,        // 150 - Execute CLI command                  [MP/M]
  P_RPL = 151,        // 151 - Call resident procedure library      [MP/M]
  F_PARSE = 152, // 152 - Parse ASCII filename into FCB        [MP/M, CP/M 3+]
  C_GET = 153,   // 153 - Get default console number           [MP/M]
  S_SYSDAT =
      154, // 154 - System data address                  [MP/M, CP/M-86 v4]
  T_SECONDS = 155, // 155 - Get date/time including seconds      [MP/M]
  P_PDADR = 156,   // 156 - Get process descriptor address       [MP/M]
  P_ABORT = 157,   // 157 - Abort process                        [MP/M]
  L_ATTACH = 158,  // 158 - Attach printer                       [MP/M]
  L_DETACH = 159,  // 159 - Detach printer                       [MP/M]
  L_SET = 160,     // 160 - Set default printer                  [MP/M]
  L_CATTACH = 161, // 161 - Conditionally attach printer         [MP/M]
  C_CATTACH = 162, // 162 - Conditionally attach console         [MP/M]
  S_OSVER = 163,   // 163 - Get OS (user-visible) version        [MP/M, DOSPLUS]
  L_GET = 164      // 164 - Get default printer device number    [MP/M]
};

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
      std::cout << std::format(
                       "Fatal: unknown BDOS system call {} with argument {}",
                       func, arg)
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
