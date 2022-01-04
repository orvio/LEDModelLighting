The circuit schematic for this example has been built on a prototype PCB with wires instead of a manufactured PCB (see images).

This means that the PCB layout in the KiCad folder should not be sent directly to a PCB manufacturing service.

This example sketch monitors the 5V and 7.5V VIN values and indicates the measured range with LEDs on the board in addition to the light control functions.
The yard office has three outside lights that are active permanently. The three illuminated rooms are on random cycles where the light is on for 5 to 15
minutes and then off for 5 to 15 minutes. One of the rooms monitors the status of another room and is only illuminated when the light of the monitored room is on.
