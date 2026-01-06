#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#define CFG_TUSB_OS_NONE 1
#define CFG_TUSB_RHPORT0_MODE OPT_MODE_HOST
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(4)))

#define CFG_TUH_HID 4     // Maximum HID devices supported
#define CFG_TUH_ENUMERATION_BUFSIZE 256

#endif /* _TUSB_CONFIG_H_ */
