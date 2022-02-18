# API Reference

## I2C Communication

Address: 0x16/0x17

0b[0 COMMAND][1 MODE][2:7 RESERVE]

COMMAND:
 - 1: Config

MODE:
 - 0: Cat Face Detection;
 - 1: Code Recognition;
 - 2: Color detection;
 - 3: Human face detection;
 - 4: Human face Recognition;
 - 5: Motion detection;
