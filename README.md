# ReMapper

## TODO
 - [x] Switch from webserver to file system in programming mode
   - [x] Programming Mode switch with LED indication - electrical
   - [x] Show file system when plugged in programming mode - software
 - [x] read/write from the QSPI memory attached on the esp
 - [x] tinyusb mcs usb drive
 - [x] Clean up software to support new boot flow and execution
 - [x] Move USB device code into it's own OS task
   - [x] Needs to be able to initialize either as a MSC or HID (or both??) 
   - [x] Neesd to accept commands either for HID or handle the file storage (It will have the handle)
 - [ ] Read and parse files in qspi
 - [x] Write OLED Driver
   - [ ] Done here: https://github.com/cedarhacks/SSD1306_esp32_driver
 - [ ] Add LCD to schematic
   - [ ] i2C IO15 data
   - [ ] i2C IO16 CLK
 - [ ] Add 1 switch + 2 buttons to schematic
   - [ ] switch to IO1
   - [ ] button 1 to IO2
   - [ ] button 2 to IO3