/**
 * \file   remregs_master.cpp
 * \brief  Arduino implementation of a register bank compatible with the remregs library (master side)
 * \author Alessandro Crespi
 * \date   June 2019
 */

#include "remregs_master.h"

/// 8-bit register read
#define ROP_READ_8   0
/// 16-bit register read
#define ROP_READ_16  1
/// 32-bit register read
#define ROP_READ_32  2
/// multibyte register read
#define ROP_READ_MB  3
/// 8-bit register write
#define ROP_WRITE_8  4
/// 16-bit register write
#define ROP_WRITE_16 5
/// 32-bit register write
#define ROP_WRITE_32 6
/// multibyte register write
#define ROP_WRITE_MB 7

#define SYNC_NONE     0        ///< no sync with client
#define SYNC_OK       1        ///< successfully synchronized with client
#define SYNC_CHECKSUM 2        ///< same as SYNC_OK, but enable use of data checksums (not implemented yet)

#define ACK           6        ///< acknowledge transmission
#define NAK          15        ///< negative acknowledge (e.g. checksum error)

RegisterBankMaster::RegisterBankMaster(HardwareSerial& s) : port(s), sync_state(SYNC_NONE), timeout(false)
{
  port.setTimeout(5000);
  desync(true);
}

RegisterBankMaster::~RegisterBankMaster()
{
  // nothing to destroy (yet)
}

uint8_t RegisterBankMaster::read_byte()
{
  timeout = false;
  
  uint8_t result;
  if (port.readBytes(&result, 1) == 1) {
    return result;
  } else {
    timeout = true;
    return 0xff;
  }
}

void RegisterBankMaster::desync(bool force)
{
  if (force || sync_state != SYNC_NONE) {
    for (uint8_t i(0); i < MAX_MB_SIZE + 5; i++) {
      port.write(0xff);
    }
  }
  sync_state = SYNC_NONE;
}

bool RegisterBankMaster::sync()
{
  sync_state = SYNC_NONE;
  port.setTimeout(250);
  desync();
  port.write(0xaa);
  timeout = false;
  uint8_t b(0), timer(0), count(0);
  do {
    b = read_byte();
    if (timeout) {
      timeout = false;
      timer++;
      if (timer > 10) {
        return false;
      }
    }
    if (b != 0xaa && b != 0x55) {
      count++;
      if (count > 200) {
        return false;
      }
    }
  } while (b != 0xaa && b != 0x55);
  sync_state = SYNC_OK;
  port.setTimeout(5000);
  return true;
}

bool RegisterBankMaster::reg_op(const uint8_t op, const uint16_t addr, const uint8_t* data, const uint8_t len)
{
  // check if a sync operation is needed
  if (sync_state == SYNC_NONE) {
    if (!sync()) {
      return false;
    }
  }

  // sends the request (2-byte opcode/address plus data if any)
  port.write((uint8_t) ((op << 2) | ((addr & 0x300) >> 8)));
  port.write((uint8_t) (addr & 0xff));
  if (op == ROP_WRITE_MB) {
    port.write(len);
  }
  for (uint8_t i(0); i < len; i++) {
    port.write(data[i]);
  }

  // reads the ACK
  timeout = false;
  uint8_t r = read_byte();
  if (timeout || r == 0xff) {
    sync_state = SYNC_NONE;
    return false;
  }
  
  if (r == ACK) {
    return true;
  } else {
    return false;
  }
}

uint8_t RegisterBankMaster::get_reg_8(const uint16_t addr)
{
  if (!reg_op(ROP_READ_8, addr, NULL, 0)) {
    return 0xff;
  }
  uint8_t res;
  if (port.readBytes((char*) &res, 1) == 1) {
    return res;
  } else {
    return 0xff;
  }
}

uint16_t RegisterBankMaster::get_reg_16(const uint16_t addr)
{
  if (!reg_op(ROP_READ_16, addr, NULL, 0)) {
    return 0xff;
  }
  uint16_t res;
  if (port.readBytes((char*) &res, 2) == 2) {
    return res;
  } else {
    return 0xffff;
  }
}

uint32_t RegisterBankMaster::get_reg_32(const uint16_t addr)
{
  if (!reg_op(ROP_READ_32, addr, NULL, 0)) {
    return 0xff;
  }
  uint32_t res;
  if (port.readBytes((char*) &res, 4) == 4) {
    return res;
  } else {
    return 0xffffffff;
  }
}

bool RegisterBankMaster::get_reg_mb(const uint16_t addr, uint8_t* data, uint8_t* len)
{
  if (!reg_op(ROP_READ_MB, addr, NULL, 0)) {
    return 0;
  }
  timeout = false;
  *len = read_byte();
  if (timeout) {
    return false;
  }
  return (port.readBytes((char*) data, *len) == *len);
}

bool RegisterBankMaster::set_reg_8(const uint16_t addr, const uint8_t val)
{
  return reg_op(ROP_WRITE_8, addr, &val, 1);
}

bool RegisterBankMaster::set_reg_16(const uint16_t addr, const uint16_t val){
  return reg_op(ROP_WRITE_16, addr, (uint8_t*) &val, 2);
}

bool RegisterBankMaster::set_reg_32(const uint16_t addr, const uint32_t val)
{
  return reg_op(ROP_WRITE_32, addr, (uint8_t*) &val, 4);
}

bool RegisterBankMaster::set_reg_mb(const uint16_t addr, const uint8_t* data, const uint8_t len)
{
  return reg_op(ROP_WRITE_MB, addr, data, len);
}
