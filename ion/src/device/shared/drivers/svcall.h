#ifndef ION_DEVICE_SHARED_DRIVERS_SVCALL_H
#define ION_DEVICE_SHARED_DRIVERS_SVCALL_H

namespace Ion {
namespace Device {
namespace SVCall {

/*#define SVC_CLOCK_LOW_FREQUENCY 1
#define SVC_CLOCK_STANDARD_FREQUENCY 2
#define SVC_POWER_SLEEP_OR_STOP 3
#define SVC_POWER_STANDBY 4
#define SVC_USB_DFU 5
//#define SVC_RESET_CORE 6
//#define SVC_EXAM_MODE_TOGGLE 7*/
#define SVC_DISPLAY_PUSH_RECT 0
#define SVC_DISPLAY_PUSH_RECT_UNIFORM 1
#define SVC_DISPLAY_PULL_RECT 2
#define SVC_DISPLAY_POST_PUSH_MULTICOLOR 3
#define SVC_USB_IS_PLUGGED 4
#define SVC_USB_IS_ENUMERATED 5
#define SVC_USB_CLEAR_ENUMERATION_INTERRUPT 6
#define SVC_USB_ENABLE 7
#define SVC_USB_DISABLE 8
#define SVC_USB_DFU 9

}
}
}

#endif
