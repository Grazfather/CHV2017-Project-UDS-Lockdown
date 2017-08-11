This is (most) of the source code to the DEFCON Car Hacking Village 2017 challenge "Project Uniform Delta Sierra Lockdown". The implementation of the UDS services themselves is proprietary.

## Hardware

This project was built on a CANBus triple, which is an Arduino Leonardo-equivalent board, with MPC2515 and MPC2515 CAN tranceivers. They are pin compatible with an Arduino Leonardo with a Sparkfun CANBus shield.

## Software

This code is sloppy. I build the code on top of the example sketch CANBusTriple, simply because it already provided working boilerplate.

The UDS stack on top of CAN is proprietary and not included. I linked against it by implementing a send/recv-like interface in _tp_link_canbustriple.h_.

## Challenge

The hex provided to contestants has the flag manually patched out so that the flag was replaced with some other dummy strings. The goal was the reverse engineer the protocol and figure out that RDBI id 0 would fail without security access being granted. You would then need to figure out how the security access routine validates the challenge response. Once this was done, you could connect to the real hardware to interact with the 'true' binary, which would then spit out the flag.
