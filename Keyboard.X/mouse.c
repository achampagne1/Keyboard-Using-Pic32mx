/** INCLUDES *******************************************************/
#include "usb.h"
#include "HardwareProfile.h"
#include "usb_function_hid.h"

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
uint8_t old_emulate_switch;
bool emulate_mode;
uint8_t movement_length;
uint8_t vector = 0;
char buffer[3];
USB_HANDLE lastTransmission;

//The direction that the mouse will move in
ROM signed char dir_table[]={-4,-4,-4, 0, 4, 4, 4, 0};

/** PRIVATE PROTOTYPES *********************************************/
void BlinkUSBStatus(void);
bool Switch2IsPressed(void);
bool SwitchIsPressed(void);
void Emulate_Mouse(void);
static void InitializeSystem(void);
void ProcessIO(void);
void UserInit(void);
void YourHighPriorityISRCode();
void YourLowPriorityISRCode();


int main(void)
{
    InitializeSystem();

    #if defined(USB_INTERRUPT)
        USBDeviceAttach();
    #endif

    while(1)
    {
        #if defined(USB_POLLING)
        USBDeviceTasks();
        #endif

        ProcessIO();        

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


void UserInit(void)
{
    // Initialize all of the LED pins not needed
    //mInitAllLEDs();
    
    // Initialize all of the push buttons
    mInitAllSwitches();
    old_emulate_switch = emulate_switch;

    // Initialize all of the mouse data to 0,0,0 (no movement)
    buffer[0]=buffer[1]=buffer[2]=0;

    emulate_mode = false;
    
    lastTransmission = 0;

}//end UserInit


void ProcessIO(void)
{   
    //Blink the LEDs according to the USB device status
    //BlinkUSBStatus();

    //
    // User Application USB tasks
    //

    // If not configured, do nothing
    if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1))
    {
        return;
    }

    //Call the function that emulates the mouse
    Emulate_Mouse();
    
}


void Emulate_Mouse(void)
{   
    if(emulate_mode == true)
    {
        // Go 14 times in the same direction before changing direction
        if(movement_length > 14)
        {
            buffer[0] = 0;
            buffer[1] = dir_table[vector & 0x07];           // X-Vector
            buffer[2] = dir_table[(vector+2) & 0x07];       // Y-Vector
            //go to the next direction in the table
            vector++;
            //reset the counter for when to change again
            movement_length = 0;
        }//end if(movement_length > 14)
    }
    else
    {
        // Don't move the mouse
        buffer[0] = buffer[1] = buffer[2] = 0;
    }

    if(HIDTxHandleBusy(lastTransmission) == 0)
    {
        //copy over the data to the HID buffer
        hid_report_in[0] = buffer[0];
        hid_report_in[1] = buffer[1];
        hid_report_in[2] = buffer[2];
     
        //Send the 3 byte packet over USB to the host.
        lastTransmission = HIDTxPacket(HID_EP, (uint8_t*)hid_report_in, 0x03);

        //increment the counter of when to change the data sent
        movement_length++;
    }
}//end Emulate_Mouse


/*Uncomment if you want to add switches
bool SwitchIsPressed(void)
{
    if(emulate_switch != old_emulate_switch)
    {
        old_emulate_switch = emulate_switch;    // Save new value
        if(emulate_switch == 0)                 // If pressed
            return true;                        // Was pressed
    }//end if
    return false;                               // Was not pressed
}//end SwitchIsPressed
*/

/* currently dont have LED KEEP INCASE YOU DO WANT LED
void BlinkUSBStatus(void)
{
    static uint16_t led_count=0;
    
    if(led_count == 0)led_count = 10000U;
    led_count--;

    #define mLED_Both_Off()         {mLED_1_Off();mLED_2_Off();}
    #define mLED_Both_On()          {mLED_1_On();mLED_2_On();}
    #define mLED_Only_1_On()        {mLED_1_On();mLED_2_Off();}
    #define mLED_Only_2_On()        {mLED_1_Off();mLED_2_On();}

    if(USBSuspendControl == 1)
    {
        if(led_count==0)
        {
            mLED_1_Toggle();
            if(mGetLED_1())
            {
                mLED_2_On();
            }
            else
            {
                mLED_2_Off();
            }
        }//end if
    }
    else
    {
        if(USBDeviceState == DETACHED_STATE)
        {
            mLED_Both_Off();
        }
        else if(USBDeviceState == ATTACHED_STATE)
        {
            mLED_Both_On();
        }
        else if(USBDeviceState == POWERED_STATE)
        {
            mLED_Only_1_On();
        }
        else if(USBDeviceState == DEFAULT_STATE)
        {
            mLED_Only_2_On();
        }
        else if(USBDeviceState == ADDRESS_STATE)
        {
            if(led_count == 0)
            {
                mLED_1_Toggle();
                mLED_2_Off();
            }//end if
        }
        else if(USBDeviceState == CONFIGURED_STATE)
        {
            if(led_count==0)
            {      
                mLED_1_Toggle();         
                if(mGetLED_1())
                {
                    mLED_2_Off();
                }
                else
                {
                    mLED_2_On();
                }
            }//end if
        }
    }
}//end BlinkUSBStatus
*/

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

