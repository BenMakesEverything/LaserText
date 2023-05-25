# LaserText v1.0
### This is the code for my portable laser projector - as seen here: https://youtu.be/u9TpJ-_hBR8

Original version: this code has a lot of issues - I recommend using v1.1, but I'm putting this here because it's what was shown in the video

## General usage notes:

This was designed to run on an Arduino nano - I don't know if it will work on anything else or not

You will have to set the left right alignment manually for each mirror - this is the number (e.g 24) as seen here: "frameOffset + 24" this accounts for imperfections in the mirror array

This will work with either serial data over USB or with an Arduino Bluetooth module. The BT module has to be disconnected to use USB though. 

Use the app "Serial Bluetooth Terminal" for BT communication

### Enjoy!
