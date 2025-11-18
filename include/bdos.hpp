#pragma once
#include <cstdint>

enum FuncNum {
  // 0 - System reset / terminate (does not return), CP/M 1,2,3 core BDOS call
  P_TERMCPM = 0,
  // 1 - Console input (echoed), CP/M 1,2,3 core BDOS call
  C_READ = 1,
  // 2 - Console output, CP/M 1,2,3 core BDOS call
  C_WRITE = 2,
  // 3 - Auxiliary / reader input (or raw console on MP/M), CP/M 1,2,3 core BDOS
  // call
  A_READ = 3,
  // 4 - Auxiliary / punch output (or raw console on MP/M), CP/M 1,2,3 core BDOS
  // call
  A_WRITE = 4,
  // 5 - Printer output (list device), CP/M 1,2,3 core BDOS call
  L_WRITE = 5,
  // 6 - Direct console I/O (CP/M 2+ semantics), CP/M 1,2,3 core BDOS call
  C_RAWIO = 6,
  // 7 - Aux input status (CP/M 3+), or Get I/O byte (CP/M 1/2) – 7 and 8 differ
  // by CP/M version: CP/M 1/2: get/set IOBYTE, CP/M 3+: auxiliary status
  A_STATIN = 7,
  // 8 - Aux output status (CP/M 3+), or Set I/O byte (CP/M 1/2) – 7 and 8
  // differ by CP/M version: CP/M 1/2: get/set IOBYTE, CP/M 3+: auxiliary status
  A_STATOUT = 8,
  // 9 - Output $-terminated string
  C_WRITESTR = 9,
  // 10 - Buffered console input
  C_READSTR = 10,
  // 11 - Console status
  C_STAT = 11,
  // 12 - Return BDOS version / system type (CP/M 2+)
  S_BDOSVER = 12,
  // 13 - Reset all drives
  DRV_ALLRESET = 13,
  // 14 - Select disk
  DRV_SET = 14,
  // 15 - Open file
  F_OPEN = 15,
  // 16 - Close file
  F_CLOSE = 16,
  // 17 - Search first
  F_SFIRST = 17,
  // 18 - Search next
  F_SNEXT = 18,
  // 19 - Delete file
  F_DELETE = 19,
  // 20 - Sequential read (next record)
  F_READ = 20,
  // 21 - Sequential write (next record)
  F_WRITE = 21,
  // 22 - Create file
  F_MAKE = 22,
  // 23 - Rename file
  F_RENAME = 23,
  // 24 - Bitmap of logged‑in drives
  DRV_LOGINVEC = 24,
  // 25 - Get current drive
  DRV_GET = 25,
  // 26 - Set DMA address
  F_DMAOFF = 26,
  // 27 - Get allocation bitmap address
  DRV_ALLOCVEC = 27,
  // 28 - Set current drive read‑only (software)
  DRV_SETRO = 28,
  // 29 - Bitmap of read‑only drives
  DRV_ROVEC = 29,
  // 30 - Set file attributes (CP/M 2+) – 30 is heavily version‑dependent; in
  // CP/M 2+ it is file attributes
  F_ATTRIB = 30,
  // 31 - Get DPB (disk parameter block) address
  DRV_DPB = 31,
  // 32 - Get/set user number
  F_USERNUM = 32,
  // 33 - Random read
  F_READRAND = 33,
  // 34 - Random write
  F_WRITERAND = 34,
  // 35 - Compute file size (set random record count)
  F_SIZE = 35,
  // 36 - Set random record from last seq op
  F_RANDREC = 36,
  // 37 - Selective disk reset
  DRV_RESET = 37,
  // 38 - Access drives (lock drive bitmap) [MP/M, CP/Net], MP/M, CP/M 3, and
  // higher 8‑bit extension
  DRV_ACCESS = 38,
  // 39 - Free drive locks [MP/M, CP/Net], MP/M, CP/M 3, and higher 8‑bit
  // extension
  DRV_FREE = 39,
  // 40 - Random write with zero fill [CP/M 2.2+], MP/M, CP/M 3, and higher
  // 8‑bit extension
  F_WRITEZF = 40,
  // 41 - Test and write record [MP/M, Concurrent], MP/M, CP/M 3, and higher
  // 8‑bit extension
  F_TESTWRITE = 41,
  // 42 - Lock record(s) [MP/M, CP/Net], MP/M, CP/M 3, and higher 8‑bit
  // extension
  F_LOCK = 42,
  // 43 - Unlock record(s) [MP/M, CP/Net], MP/M, CP/M 3, and higher 8‑bit
  // extension
  F_UNLOCK = 43,
  // 44 - Set multisector count [MP/M II+, CP/M 3+], MP/M, CP/M 3, and higher
  // 8‑bit extension
  F_MULTISEC = 44,
  // 45 - Set hardware error handling mode [Personal CP/M, MP/M], MP/M, CP/M 3,
  // and higher 8‑bit extension
  F_ERRMODE = 45,
  // 46 - Get free space on drive [MP/M II+], MP/M, CP/M 3, and higher 8‑bit
  // extension
  DRV_SPACE = 46,
  // 47 - Chain to another program [MP/M II+, some 8‑bit], MP/M, CP/M 3, and
  // higher 8‑bit extension
  P_CHAIN = 47,
  // 48 - Flush/empty disk buffers [Personal, MP/M II+], MP/M, CP/M 3, and
  // higher 8‑bit extension
  DRV_FLUSH = 48,
  // 49 - Access System Control Block [CP/M 3], MP/M, CP/M 3, and higher 8‑bit
  // extension
  S_SCB = 49,
  // 50 - Call BIOS via BDOS [CP/M 3+], MP/M, CP/M 3, and higher 8‑bit extension
  S_BIOS = 50,
  // 54 - Get file timestamp buffer [Z80DOS, ZPM3], Z80DOS / ZPM3 time‑stamp
  // extension (still 8‑bit BDOS)
  Z80_GETSTAMP = 54,
  // 55 - Use timestamp buffer on next call [Z80DOS, ZPM3], Z80DOS / ZPM3
  // time‑stamp extension (still 8‑bit BDOS)
  Z80_USESTAMP = 55,
  // 59 - Load overlay / PRL / RSX [CP/M 3+ loader], Loader / RSX on CP/M 3+
  // (8‑bit BDOS)
  P_LOAD = 59,
  // 60 - Call RSX [CP/M 3+ RSX], Loader / RSX on CP/M 3+ (8‑bit BDOS)
  RSX_CALL = 60,
  // 64 - Log in to server [CP/Net], CP/Net call (8‑bit BDOS)
  N_LOGIN = 64,
  // 65 - Log off [CP/Net], CP/Net call (8‑bit BDOS)
  N_LOGOFF = 65,
  // 66 - Send network message [CP/Net], CP/Net call (8‑bit BDOS)
  N_SENDMSG = 66,
  // 67 - Receive network message [CP/Net], CP/Net call (8‑bit BDOS)
  N_RECVMSG = 67,
  // 68 - Get network status [CP/Net], CP/Net call (8‑bit BDOS)
  N_STATUS = 68,
  // 69 - Get CP/Net configuration table [CP/Net], CP/Net call (8‑bit BDOS)
  N_CONFIG = 69,
  // 70 - Set compatibility attributes [CP/Net‑86, some 8‑bit], CP/Net call
  // (8‑bit BDOS)
  N_COMPAT = 70,
  // 71 - Get server configuration [CP/Net‑86, some 8‑bit], CP/Net call (8‑bit
  // BDOS)
  N_SERVERCONF = 71,
  // 98 - Clean up disk on program exit [CP/M 3 internal], CP/M 3 / MP/M II+
  // time, date, label, password, serial, console
  F_CLEANUP = 98,
  // 99 - Truncate file [CP/M 3+], CP/M 3 / MP/M II+ time, date, label,
  // password, serial, console
  F_TRUNCATE = 99,
  // 100 - Set directory label [MP/M II+], CP/M 3 / MP/M II+ time, date, label,
  // password, serial, console
  DRV_SETLABEL = 100,
  // 101 - Get directory label flags byte [MP/M II+], CP/M 3 / MP/M II+ time,
  // date, label, password, serial, console
  DRV_GETLABEL = 101,
  // 102 - Get file date/time [MP/M II+], CP/M 3 / MP/M II+ time, date, label,
  // password, serial, console
  F_TIMEDATE = 102,
  // 103 - Set file password/protection [MP/M II+], CP/M 3 / MP/M II+ time,
  // date, label, password, serial, console
  F_WRITEXFCB = 103,
  // 104 - Set system date/time [MP/M II+, Z80DOS, DOS+], CP/M 3 / MP/M II+
  // time, date, label, password, serial, console
  T_SET = 104,
  // 105 - Get system date/time [MP/M II+, Z80DOS, DOS+], CP/M 3 / MP/M II+
  // time, date, label, password, serial, console
  T_GET = 105,
  // 106 - Set default password [MP/M II+, CP/Net], CP/M 3 / MP/M II+ time,
  // date, label, password, serial, console
  F_PASSWD = 106,
  // 107 - Get system serial number [MP/M II+], CP/M 3 / MP/M II+ time, date,
  // label, password, serial, console
  S_SERIAL = 107,
  // 108 - Get/set program return code [CP/M 3+], CP/M 3 / MP/M II+ time, date,
  // label, password, serial, console
  P_CODE = 108,
  // 109 - Get/set console mode [CP/M 3+, Personal], CP/M 3 / MP/M II+ time,
  // date, label, password, serial, console
  C_MODE = 109,
  // 110 - Get/set BDOS string delimiter [CP/M 3+, Personal], CP/M 3 / MP/M II+
  // time, date, label, password, serial, console
  C_DELIMIT = 110,
  // 111 - Write block of text to console [CP/M 3+, Personal], CP/M 3 / MP/M II+
  // time, date, label, password, serial, console
  C_WRITEBLK = 111,
  // 112 - Write block of text to printer [CP/M 3+, Personal], CP/M 3 / MP/M II+
  // time, date, label, password, serial, console
  L_WRITEBLK = 112,
  // 113 is Personal CP/M direct screen functions via BIOS (not mapped here)
  // 115 is GSX, 8‑bit but very system‑specific (not mapped here)
  // 128 - Allocate absolute memory [MP/M], MP/M / Concurrent CP/M process,
  // queue, and device function (8‑bit)
  M_ALLOC_ABS = 128,
  // 129 - Allocate relocatable memory [MP/M], MP/M / Concurrent CP/M process,
  // queue, and device function (8‑bit)
  M_ALLOC = 129,
  // 130 - Free memory [MP/M], MP/M / Concurrent CP/M process, queue, and device
  // function (8‑bit)
  M_FREE = 130,
  // 131 - Poll I/O device [MP/M], MP/M / Concurrent CP/M process, queue, and
  // device function (8‑bit)
  DEV_POLL = 131,
  // 132 - Wait on system flag [MP/M], MP/M / Concurrent CP/M process, queue,
  // and device function (8‑bit)
  DEV_WAITFLAG = 132,
  // 133 - Set system flag [MP/M], MP/M / Concurrent CP/M process, queue, and
  // device function (8‑bit)
  DEV_SETFLAG = 133,
  // 134 - Create message queue [MP/M], MP/M / Concurrent CP/M process, queue,
  // and device function (8‑bit)
  Q_MAKE = 134,
  // 135 - Open message queue [MP/M], MP/M / Concurrent CP/M process, queue, and
  // device function (8‑bit)
  Q_OPEN = 135,
  // 136 - Delete message queue [MP/M], MP/M / Concurrent CP/M process, queue,
  // and device function (8‑bit)
  Q_DELETE = 136,
  // 137 - Read from message queue [MP/M], MP/M / Concurrent CP/M process,
  // queue, and device function (8‑bit)
  Q_READ = 137,
  // 138 - Conditional read from queue [MP/M], MP/M / Concurrent CP/M process,
  // queue, and device function (8‑bit)
  Q_CREAD = 138,
  // 139 - Write to message queue [MP/M], MP/M / Concurrent CP/M process, queue,
  // and device function (8‑bit)
  Q_WRITE = 139,
  // 140 - Conditional write to queue [MP/M], MP/M / Concurrent CP/M process,
  // queue, and device function (8‑bit)
  Q_CWRITE = 140,
  // 141 - Delay [MP/M], MP/M / Concurrent CP/M process, queue, and device
  // function (8‑bit)
  P_DELAY = 141,
  // 142 - Yield / call dispatcher [MP/M], MP/M / Concurrent CP/M process,
  // queue, and device function (8‑bit)
  P_DISPATCH = 142,
  // 143 - Terminate process [MP/M], MP/M / Concurrent CP/M process, queue, and
  // device function (8‑bit)
  P_TERM = 143,
  // 144 - Create subprocess [MP/M], MP/M / Concurrent CP/M process, queue, and
  // device function (8‑bit)
  P_CREATE = 144,
  // 145 - Set process priority [MP/M], MP/M / Concurrent CP/M process, queue,
  // and device function (8‑bit)
  P_PRIORITY = 145,
  // 146 - Attach console [MP/M], MP/M / Concurrent CP/M process, queue, and
  // device function (8‑bit)
  C_ATTACH = 146,
  // 147 - Detach console [MP/M], MP/M / Concurrent CP/M process, queue, and
  // device function (8‑bit)
  C_DETACH = 147,
  // 148 - Set default console number [MP/M], MP/M / Concurrent CP/M process,
  // queue, and device function (8‑bit)
  C_SET = 148,
  // 149 - Assign console to process [MP/M], MP/M / Concurrent CP/M process,
  // queue, and device function (8‑bit)
  C_ASSIGN = 149,
  // 150 - Execute CLI command [MP/M], MP/M / Concurrent CP/M process, queue,
  // and device function (8‑bit)
  P_CLI = 150,
  // 151 - Call resident procedure library [MP/M], MP/M / Concurrent CP/M
  // process, queue, and device function (8‑bit)
  P_RPL = 151,
  // 152 - Parse ASCII filename into FCB [MP/M, CP/M 3+], MP/M / Concurrent CP/M
  // process, queue, and device function (8‑bit)
  F_PARSE = 152,
  // 153 - Get default console number [MP/M], MP/M / Concurrent CP/M process,
  // queue, and device function (8‑bit)
  C_GET = 153,
  // 154 - System data address [MP/M, CP/M-86 v4], MP/M / Concurrent CP/M
  // process, queue, and device function (8‑bit)
  S_SYSDAT = 154,
  // 155 - Get date/time including seconds [MP/M], MP/M / Concurrent CP/M
  // process, queue, and device function (8‑bit)
  T_SECONDS = 155,
  // 156 - Get process descriptor address [MP/M], MP/M / Concurrent CP/M
  // process, queue, and device function (8‑bit)
  P_PDADR = 156,
  // 157 - Abort process [MP/M], MP/M / Concurrent CP/M process, queue, and
  // device function (8‑bit)
  P_ABORT = 157,
  // 158 - Attach printer [MP/M], MP/M / Concurrent CP/M process, queue, and
  // device function (8‑bit)
  L_ATTACH = 158,
  // 159 - Detach printer [MP/M], MP/M / Concurrent CP/M process, queue, and
  // device function (8‑bit)
  L_DETACH = 159,
  // 160 - Set default printer [MP/M], MP/M / Concurrent CP/M process, queue,
  // and device function (8‑bit)
  L_SET = 160,
  // 161 - Conditionally attach printer [MP/M], MP/M / Concurrent CP/M process,
  // queue, and device function (8‑bit)
  L_CATTACH = 161,
  // 162 - Conditionally attach console [MP/M], MP/M / Concurrent CP/M process,
  // queue, and device function (8‑bit)
  C_CATTACH = 162,
  // 163 - Get OS (user-visible) version [MP/M, DOSPLUS], MP/M / Concurrent CP/M
  // process, queue, and device function (8‑bit)
  S_OSVER = 163,
  // 164 - Get default printer device number [MP/M], MP/M / Concurrent CP/M
  // process, queue, and device function (8‑bit)
  L_GET = 164
};

struct FileControlBlock {
  // 00h: Drive (0 = default, 1–16 = A–P; bit 7 may indicate subdir mode)
  uint8_t dr;
  // 01h–08h: Filename (8 chars, 7‑bit ASCII with attribute bits in high bit)
  uint8_t f[8];
  // 09h–0Bh: Filetype/extension (3 chars, 7‑bit ASCII with attribute bits)
  uint8_t t[3];
  // 0Ch: EX - Extent; set to 0 when opening, then leave to CP/M
  uint8_t ex;
  // 0Dh: S1 - Reserved
  uint8_t s1;
  // 0Eh: S2 - Reserved
  uint8_t s2;
  // 0Fh: RC - Record count within extent; set to 0 on open, then leave to CP/M
  uint8_t rc;
  // 10h–1Fh: AL - Allocation data (image of second half of directory entry)
  uint8_t al[16];
  // 20h: CR - Current record within extent
  uint8_t cr;
  // 21h–23h: Rn - Random record number (CP/M 2: 16‑bit + overflow; CP/M 3:
  // 18‑bit)
  uint8_t r[3];
};
