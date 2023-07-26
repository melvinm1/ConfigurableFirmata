/*
  FirmataEncoder.h - Firmata library
  Copyright (C) 2013 Norbert Truchsess. All rights reserved.
  Copyright (C) 2014 Nicolas Panel. All rights reserved.
  Copyright (C) 2015 Jeff Hoefs. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Provide encoder feature based on PJRC implementation.
  See http://www.pjrc.com/teensy/td_libs_Encoder.html for more informations

  FirmataEncoder handles instructions and is able to automatically report positions.
  See protocol details in "examples/SimpleFirmataEncoder/SimpleFirmataEncoder.ino"


  melvinm1 2023-07-26:
    - Changed auto report to report position change since last report. Reports 
      are only sent if a change has occured.
    - Removed all methods except enabling/disabling auto report and attaching/
      detaching encoders.
    - Updated to Firmata 3.x.
*/

#ifndef FirmataEncoder_h
#define FirmataEncoder_h

#include <ConfigurableFirmata.h>
#include <FirmataFeature.h>

// This optional setting causes Encoder to use more optimized code
// safe if 'attachInterrupt' is never used in the same time
//#define ENCODER_OPTIMIZE_INTERRUPTS // => not compiling
#include <Encoder.h>

#define MAX_ENCODERS                5 // arbitrary value, may need to adjust
#define ENCODER_ATTACH              (0x00)
#define ENCODER_REPORT_AUTO         (0x04)
#define ENCODER_DETACH              (0x05)

class FirmataEncoder:public FirmataFeature
{
public:
  FirmataEncoder();
  //~FirmataEncoder(); => never destroy in practice

  // FirmataFeature implementation
  boolean handlePinMode(byte pin, int mode);
  void handleCapability(byte pin);
  boolean handleSysex(byte command, byte argc, byte *argv);
  void reset();
  void report(bool elapsed) override;

  // FirmataEncoder implementation
  boolean isEncoderAttached(byte encoderNum);
  void attachEncoder(byte encoderNum, byte pinANum, byte pinBNum);
  void detachEncoder(byte encoderNum);
  void toggleAutoReport(byte report);
  bool isReportingEnabled();

private:
  static void _reportEncoderPosition(byte encoder, int32_t position);
};

#endif
