# bluno-arduino

This repo is the code for programming an arduino to controll the bluno battle for ElderStrolls.

You'll need the Arduino WaveHC library and the SD library. There're a bit finiky with regards to the pins that they use on the arduino, so I'd recommend not changing the pins from what the settings are in the code.

Pin 7 should be connected to a switch relay that controlls the blue LEDs. The BDW sells them. You should use the relay to control an extension cord that the lights are plugged into

Pin 8 should be connected to a IR remote control receiver. They're $5 on amazon and they come with a remote. This enables the transitions between battle phases.
