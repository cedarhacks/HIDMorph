# ReMapper

## TODO
### Sep 2, 2025
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
 - [ ] Add LCD to schematic
 - [ ] Add 1 switch + 2 buttons to schematic 