/** INCLUDES *******************************************************/
#include "usb.h"
#include "HardwareProfile.h"
#include "usb_function_hid.h"
#include <stdio.h>

/** CONFIGURATION **************************************************/
#ifndef OVERRIDE_CONFIG_BITS
        
    #pragma config UPLLEN   = ON            // USB PLL Enabled
    #pragma config FPLLMUL  = MUL_20        // PLL Multiplier
    #pragma config UPLLIDIV = DIV_2         // USB PLL Input Divider
    #pragma config FPLLIDIV = DIV_2         // PLL Input Divider
    #pragma config FPLLODIV = DIV_2         // PLL Output Divider
    #pragma config FPBDIV   = DIV_1         // Peripheral Clock divisor
    #pragma config FWDTEN   = OFF           // Watchdog Timer 
    #pragma config WDTPS    = PS1           // Watchdog Timer Postscale
    #pragma config FCKSM    = CSDCMD        // Clock Switching & Fail Safe Clock Monitor
    #pragma config OSCIOFNC = OFF           // CLKO Enable
    #pragma config POSCMOD  = HS            // Primary Oscillator
    #pragma config IESO     = OFF           // Internal/External Switch-over
    #pragma config FSOSCEN  = OFF           // Secondary Oscillator Enable
    #pragma config FNOSC    = PRIPLL        // Oscillator Selection
    #pragma config CP       = OFF           // Code Protect
    #pragma config BWP      = OFF           // Boot Flash Write Protect
    #pragma config PWP      = OFF           // Program Flash Write Protect
    #pragma config ICESEL   = ICS_PGx2      // ICE/ICD Comm Channel Select
    #pragma config DEBUG    = OFF           // Debugger Disabled for Starter Kit
            
#endif // OVERRIDE_CONFIG_BITS


/** VARIABLES ******************************************************/
#define SYS_FREQ 40000000
bool pressFlag = true;
uint8_t report[8];
USB_HANDLE USBInHandle;
int count = 0 ;

/** PRIVATE PROTOTYPES *********************************************/
void delay_ms(unsigned int ms);
void copyArray(uint8_t* arr1, uint8_t* arr2, int size);
static void InitializeSystem(void);
void ProcessIO(void);
void UserInit(void);
void YourHighPriorityISRCode();
void YourLowPriorityISRCode();


int main(void)
{
    InitializeSystem();
    TRISBbits.TRISB0 = 1; // Set RB0 as input


    while (1) {
        #if defined(USB_POLLING)
        USBDeviceTasks();  // Maintain the USB stack if polling is used
        #endif

        // Ensure USB is in the configured state before sending reports
        if (USBGetDeviceState() == CONFIGURED_STATE) {
            // Check if the previous transfer is complete and the button is pressed
            if (!HIDTxHandleBusy(USBInHandle) && (PORTBbits.RB0 == 1)) {  // Button pressed (active-low)
                // Prepare the report for button press (send "b")
                uint8_t report[8] = { 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 };
                USBInHandle = HIDTxPacket(HID_EP, report, 8);  // Send the HID report
            }
            else if (!HIDTxHandleBusy(USBInHandle)) {
                // Prepare the report for no button press (send nothing)
                uint8_t report[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                USBInHandle = HIDTxPacket(HID_EP, report, 8);  // Send the HID report
            }
        }
    }
}

static void InitializeSystem(void)
{
    ANSELA = 0x0000;  // Configure all Port A pins as digital
    ANSELB = 0x0000;  // Configure all Port B pins as digital
    ANSELC = 0x0000;  // Configure all Port C pins as digital

    #if defined(USE_USB_BUS_SENSE_IO)
    tris_usb_bus_sense = INPUT_PIN; // See HardwareProfile.h
    #endif
    
    #if defined(USE_SELF_POWER_SENSE_IO)
    tris_self_power = INPUT_PIN;    // See HardwareProfile.h
    #endif
    
    UserInit();

    USBDeviceInit(); 
}

void copyArray(uint8_t* arr1, uint8_t* arr2, int size){
    for(int i=0;i<size;i++){
        arr2[i]=arr1[i];
    }
}


void UserInit(void)
{

}//end UserInit


void ProcessIO(void)
{   
    
}

void USBCBSuspend(void)
{
    
}

void USBCBWakeFromSuspend(void)
{
    
}

void USBCB_SOF_Handler(void)
{
    // No need to clear UIRbits.SOFIF to 0 here.
    // Callback caller is already doing that.
}


void USBCBErrorHandler(void)
{
    
}

void USBCBCheckOtherReq(void)
{
    USBCheckHIDRequest();
}

void USBCBStdSetDscHandler(void)
{
    // Must claim session ownership if supporting this request
}

void USBCBInitEP(void)
{
    //enable the HID endpoint
    USBEnableEndpoint(HID_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
}

void USBCBSendResume(void)
{
    static uint16_t delay_count;
    
    USBResumeControl = 1;                // Start RESUME signaling
    
    delay_count = 1800U;                // Set RESUME line for 1-13 ms
    do
    {
        delay_count--;
    }while(delay_count);
    USBResumeControl = 0;
}


bool USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, uint16_t size)
{
    switch(event)
    {
        case EVENT_CONFIGURED: 
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER:
            Nop();
            break;
        default:
            break;
    }      
    return true; 
}

void delay_ms(unsigned int ms) {
    unsigned int tWait, tStart;

    // Convert milliseconds to core timer ticks
    tWait = (SYS_FREQ / 2000) * ms; // Core timer increments at half the system clock speed

    // Read the starting core timer value
    tStart = _CP0_GET_COUNT();

    // Wait until the required number of ticks have passed
    while ((_CP0_GET_COUNT() - tStart) < tWait);
}

