#ifndef __REMREGS_MASTER_H
#define __REMREGS_MASTER_H

/**
 * \file   remregs_master.h
 * \brief  Header for Arduino implementation of a register bank master compatible with the remregs library
 * \author Alessandro Crespi
 * \date   June 2019
 * \note   Derived from AmphiBot III/Envirobot radio register bank
 */

#include <stdint.h>
#include <HardwareSerial.h>

/// maximal size (in bytes) of a multibyte register
#define MAX_MB_SIZE  29

class RegisterBankMaster {
  
public:

  /// Constructor
  RegisterBankMaster(HardwareSerial& s);
  
  /// Destructor
  virtual ~RegisterBankMaster();

  /// Synchronizes the communication between the PIC and the remregs slave
  bool sync();

  /** \brief Reads an 8-bit register
    * \param addr The address of the register (0 - 1023)
    * \return The read value (0x00 - 0xff) or 0xff on failure
    */
  uint8_t get_reg_8(const uint16_t addr);

  /** \brief Reads a 16-bit register
    * \param addr The address of the register (0 - 1023)
    * \return The read value (0x0000 - 0xffff) or 0xffff on failure
    */
  uint16_t get_reg_16(const uint16_t addr);

  /** \brief Reads a 32-bit register
    * \param addr The address of the register (0 - 1023)
    * \return The read value (0x00000000 - 0xffffffff) or 0xffffffff on failure
    */
  uint32_t get_reg_32(const uint16_t addr);

  /** \brief Reads a multibyte register
    * \param addr The address of the register (0 - 1023)
    * \param data A pointer to the output buffer (at least 29 bytes long)
    * \param len Pointer to a variable that will contain the length of the returned data
    * \return true if the operation succeeded, false if not
    */
  bool get_reg_mb(const uint16_t addr, uint8_t* data, uint8_t* len);


  /** \brief Writes an 8-bit register
    * \param addr The address of the register (0 - 1023)
    * \param val The value to write to the register
    * \return true if the operation succeeded, false if not
    */  
  bool set_reg_8(const uint16_t addr, const uint8_t val);

  /** \brief Writes a 16-bit register
    * \param addr The address of the register (0 - 1023)
    * \param val The value to write to the register
    * \return true if the operation succeeded, false if not
    */  
  bool set_reg_16(const uint16_t addr, const uint16_t val);

  /** \brief Writes a 32-bit register
    * \param addr The address of the register (0 - 1023)
    * \param val The value to write to the register
    * \return true if the operation succeeded, false if not
    */
  bool set_reg_32(const uint16_t addr, const uint32_t val);

  /** \brief Writes a multibyte register
    * \param addr The address of the register (0 - 1023)
    * \param data Pointer to the data to write to the register
    * \param len Length of the data to write (0 - 29 bytes)
    * \return true if the operation succeeded, false if not
    */
  bool set_reg_mb(const uint16_t addr, const uint8_t* data, const uint8_t len);

private:

  /// Reference to the serial port to use
  HardwareSerial& port;

  /// Synchronization state (by default: not synchronized -> no communication possible)
  uint8_t sync_state;
  
  /// true if a timeout occurred during the last call to ::read_byte()
  bool timeout;

  /// Utility function: reads a single byte from the input port, setting ::timeout to
  /// true in case of timeout
  uint8_t read_byte();
  
  /// Sends a stream of 0xff bytes to (eventually) inform the slave that
  /// there is a synchronization problem
  void desync(bool force = false);

  /// Internal implementation of any register operation
  bool reg_op(const uint8_t op, const uint16_t addr, const uint8_t* data, const uint8_t len);

};

#endif
