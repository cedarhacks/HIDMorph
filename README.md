# ReMapper

## TODO
### Sep 2, 2025
 - [ ] Switch from webserver to file system in programming mode
   - [ ] Programming Mode switch with LED indication - electrical
   - [ ] Show file system when plugged in programming mode - software
 - [x] read/write from the QSPI memory attached on the esp
 - [ ] tinyusb mcs usb drive
 - [ ] Clean up software to support new boot flow and execution
 - [ ] Move USB device code into it's own OS task
   - [ ] Needs to be able to initialize either as a MSC or HID (or both??) 
   - [ ] Neesd to accept commands either for HID or handle the file storage (It will have the handle)