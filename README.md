# HIDMorph

**HIDMorph** is a programmable USB HID host/device translator built on a dual-MCU architecture (ESP32-S3 + RP2040).  
It enables real-time translation, remapping, and transformation of USB HID inputs (keyboards, mice, controllers) without requiring firmware recompilation.

The system is designed as an embedded product, combining real-time firmware, USB protocol handling, and custom hardware.

---

## What HIDMorph Does

- Acts as a **USB HID host and USB HID device** simultaneously  
- Translates HID inputs (e.g. keyboard → joystick, macros, custom mappings) in real time  
- Supports **runtime-configurable behavior** via a Lua scripting layer  
- Designed for low-latency, deterministic input handling  
- Runs entirely on-device (no PC software required after setup)

---

## Architecture Overview

HIDMorph uses a **dual-MCU design** to cleanly separate USB responsibilities:

- **RP2040**
  - USB Host stack
  - Enumerates and reads input HID devices
  - Handles timing-sensitive USB host transactions

- **ESP32-S3**
  - USB Device stack (HID output)
  - Runs FreeRTOS-based application logic
  - Executes Lua scripts for dynamic input translation
  - Manages on-device filesystem and configuration

Communication between MCUs is handled over a dedicated internal interface designed for low latency and robustness.

*(Architecture diagram coming soon)*

---

## Firmware

- **RTOS:** FreeRTOS  
- **USB Stack:** TinyUSB  
- **Languages:** C/C++, Lua  
- **Features:**
  - USB HID descriptor management
  - Class handling for common HID devices
  - Scriptable runtime for input translation
  - Fault handling and recovery paths for USB disconnects

---

## Hardware

- Custom PCB designed in KiCad  
- Dual-MCU architecture (ESP32-S3 + RP2040)  
- USB host and device interfaces  
- Designed for iterative hardware bring-up and revision

*(Schematics, PCB renders, and photos will be added as the design stabilizes)*

---

## Project Status

HIDMorph is under **active development**.

Completed:
- Core dual-MCU architecture
- USB host/device communication path
- Base firmware framework
- Lua runtime integration
- Initial PCB revisions and bring-up

In progress:
- Expanded HID device support
- Improved scripting APIs
- Documentation and examples
- Enclosure refinement

