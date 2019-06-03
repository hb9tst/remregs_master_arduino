#include <Servo.h> 
#include <remregs_master.h>

/**********************************************************************
  \file   remregs_master_demo.ino
  \author Alessandro Crespi
  \date   June 2019
  \brief  Arduino remregs master side test/example
 
  This program is a basic demonstration of how to call register
  oeprations on the remregs master library.
 
**********************************************************************/

// Register bank, using Serial as communication port
RegisterBankMaster regs(Serial);

void setup()
{
  // built-in LED is an output
  pinMode(LED_BUILTIN, OUTPUT);

  // initializes the serial port at 115.2 kbps
  Serial.begin(115200);

  // synchronises with the remote device (e.g. Arduino running the remregs demo)
  regs.sync();
}

void loop()
{
  // repeatedly set the remote 8-bit register at address 1 to 0 and 1, in alternance
  // with the local LED -- with the remregs demo this makes a remotely controlled LED
  // blinker

  regs.set_reg_8(1, 0);
  digitalWrite(LED_BUILTIN, 1);
  delay(250);
  regs.set_reg_8(1, 1);
  digitalWrite(LED_BUILTIN, 0);
  delay(250);
}
