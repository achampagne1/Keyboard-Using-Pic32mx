/*******************************************************************************

    USB Device header file

Summary:
    This file, with its associated C source file, provides the main substance of
    the USB device side stack.  These files will receive, transmit, and process
    various USB commands as well as take action when required for various events
    that occur on the bus.

Description:
    This file, with its associated C source file, provides the main substance of
    the USB device side stack.  These files will receive, transmit, and process
    various USB commands as well as take action when required for various events
    that occur on the bus.

    This file is located in the "\<Install Directory\>\\Microchip\\Include\\USB"
    directory.
    
    When including this file in a new project, this file can either be
    referenced from the directory in which it was installed or copied
    directly into the user application folder. If the first method is
    chosen to keep the file located in the folder in which it is installed
    then include paths need to be added so that the library and the
    application both know where to reference each others files. If the
    application folder is located in the same folder as the Microchip
    folder (like the current demo folders), then the following include
    paths need to be added to the application's project:
    
    ..\\..\\Microchip\\Include

    .
    
    If a different directory structure is used, modify the paths as
    required. An example using absolute paths instead of relative paths
    would be the following:
    
    C:\\Microchip Solutions\\Microchip\\Include
    
    C:\\Microchip Solutions\\My Demo Application 

******************************************************************************/
//DOM-IGNORE-BEGIN
/******************************************************************************
 FileName:     	usb_device.h
 Dependencies:	See INCLUDES section
 Processor:		PIC18 or PIC24 USB Microcontrollers
 Hardware:		The code is natively intended to be used on the following
 				hardware platforms: PICDEM� FS USB Demo Board, 
 				PIC18F87J50 FS USB Plug-In Module, or
 				Explorer 16 + PIC24 USB PIM.  The firmware may be
 				modified for use on other USB platforms by editing the
 				HardwareProfile.h file.
 Complier:  	Microchip C18 (for PIC18) or C30 (for PIC24)
 Company:		Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the �Company�) for its PIC� Microcontroller is intended and
 supplied to you, the Company�s customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN �AS IS� CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*******************************************************************/

/********************************************************************
 Change History:
  Rev    Description
  ----   -----------
  2.1    Added "(" & ")" to EP definitions
         updated for simplicity and to use common
         coding style

  2.6    Removed many of the device specific information to the
         HAL layer files.  Moved many of the CH9 defintions to the
         CH9 file.

  2.6a   No Change

  2.7    Fixed error where USBHandleGetAddr() didn't convert the
         return address from a physical address to a virtual address 
         for PIC32.

         Added macro versions of USBDeviceAttach() and USBDeviceDetach()
         so they will compile without error when using polling mode.

  2.8    Added EVENT_TRANSFER_TERMINATED event enum item.

********************************************************************/
#pragma once
#include <stdbool.h> //included bool instead
#include <stdint.h> //included int type
//added for byte and bool support

#ifndef USBDEVICE_H
#define USBDEVICE_H
//DOM-IGNORE-END

/** DEFINITIONS ****************************************************/

//USB_HANDLE is a pointer to an entry in the BDT.  This pointer can be used
//  to read the length of the last transfer, the status of the last transfer,
//  and various other information.  Insure to initialize USB_HANDLE objects
//  to NULL so that they are in a known state during their first usage.
#define USB_HANDLE void*

#define USB_EP0_ROM            0x00     //Data comes from RAM
#define USB_EP0_RAM            0x01     //Data comes from ROM
#define USB_EP0_BUSY           0x80     //The PIPE is busy
#define USB_EP0_INCLUDE_ZERO   0x40     //include a trailing zero packet
#define USB_EP0_NO_DATA        0x00     //no data to send
#define USB_EP0_NO_OPTIONS     0x00     //no options set

/********************************************************************
 * Standard Request Codes
 * USB 2.0 Spec Ref Table 9-4
 *******************************************************************/

/* USB Device States as returned by USBGetDeviceState().  Only the defintions
   for these states should be used.  The actual value for each state should
   not be relied upon as constant and may change based on the implementation. */
typedef enum
{
    /* Detached is the state in which the device is not attached to the bus.  When
    in the detached state a device should not have any pull-ups attached to either
    the D+ or D- line.  */
    DETACHED_STATE 
    /*DOM-IGNORE-BEGIN*/    = 0x00                         /*DOM-IGNORE-END*/,
    /* Attached is the state in which the device is attached ot the bus but the
    hub/port that it is attached to is not yet configured. */
    ATTACHED_STATE
    /*DOM-IGNORE-BEGIN*/    = 0x01                         /*DOM-IGNORE-END*/,
    /* Powered is the state in which the device is attached to the bus and the 
    hub/port that it is attached to is configured. */
    POWERED_STATE
    /*DOM-IGNORE-BEGIN*/    = 0x02                         /*DOM-IGNORE-END*/,
    /* Default state is the state after the device receives a RESET command from
    the host. */
    DEFAULT_STATE
    /*DOM-IGNORE-BEGIN*/    = 0x04                         /*DOM-IGNORE-END*/,
    /* Address pending state is not an official state of the USB defined states.
    This state is internally used to indicate that the device has received a 
    SET_ADDRESS command but has not received the STATUS stage of the transfer yet.
    The device is should not switch addresses until after the STATUS stage is
    complete.  */
    ADR_PENDING_STATE
    /*DOM-IGNORE-BEGIN*/    = 0x08                         /*DOM-IGNORE-END*/,
    /* Address is the state in which the device has its own specific address on the
    bus. */
    ADDRESS_STATE
    /*DOM-IGNORE-BEGIN*/    = 0x10                         /*DOM-IGNORE-END*/,
    /* Configured is the state where the device has been fully enumerated and is
    operating on the bus.  The device is now allowed to excute its application
    specific tasks.  It is also allowed to increase its current consumption to the
    value specified in the configuration descriptor of the current configuration.
    */
    CONFIGURED_STATE
    /*DOM-IGNORE-BEGIN*/    = 0x20                        /*DOM-IGNORE-END*/
} USB_DEVICE_STATE;


/* USB device stack events description here - DWF */
typedef enum
{
    // Notification that a SET_CONFIGURATION() command was received (device)
    EVENT_CONFIGURED
    /*DOM-IGNORE-BEGIN*/    = EVENT_DEVICE_STACK_BASE        /*DOM-IGNORE-END*/,

    // A SET_DESCRIPTOR request was received (device)
    EVENT_SET_DESCRIPTOR,

    // An endpoint 0 request was received that the stack did not know how to
    // handle.  This is most often a request for one of the class drivers.  
    // Please refer to the class driver documenation for information related
    // to what to do if this request is received. (device)
    EVENT_EP0_REQUEST,

//    // A USB transfer has completed.  The data associated with this event is of
//    // the data type HOST_TRANSFER_DATA if the event is generated from the host
//    // stack.
//    EVENT_TRANSFER,
//    
//    // A USB Start of Frame token has been received.  This event is not
//    // used by the Host stack.
//    EVENT_SOF,                  
//    
//    // Device-mode resume received.  This event is not used by the Host stack.
//    EVENT_RESUME,
//    
//    // Device-mode suspend/idle event received.  This event is not used by the
//    // Host stack.
//    EVENT_SUSPEND,
//                  
//    // Device-mode bus reset received.  This event is not used by the Host 
//    // stack.                  
//    EVENT_RESET,                
    
//    // Device Mode: A setup packet received (data: SETUP_PKT).  This event is
//    // not used by the Host stack.
//    EVENT_SETUP,

    // Device-mode USB cable has been attached.  This event is not used by the
    // Host stack.  The client driver may provide an application event when a
    // device attaches.
    EVENT_ATTACH,

    // A user transfer was terminated by the stack.  This event will pass back
    // the value of the handle that was terminated.  Compare this value against
    // the current valid handles to determine which transfer was terminated.
    EVENT_TRANSFER_TERMINATED

} USB_DEVICE_STACK_EVENTS;

/** Function Prototypes **********************************************/

/**************************************************************************
  Function:
        void USBDeviceTasks(void)
    
  Summary:
    This function is the main state machine of the USB device side stack.
    This function should be called periodically to receive and transmit
    packets through the stack. This function should be called preferably
    once every 100us during the enumeration process. After the enumeration
    process this function still needs to be called periodically to respond
    to various situations on the bus but is more relaxed in its time
    requirements. This function should also be called at least as fast as
    the OUT data expected from the PC.

  Description:
    This function is the main state machine of the USB device side stack.
    This function should be called periodically to receive and transmit
    packets through the stack. This function should be called preferably
    once every 100us during the enumeration process. After the enumeration
    process this function still needs to be called periodically to respond
    to various situations on the bus but is more relaxed in its time
    requirements. This function should also be called at least as fast as
    the OUT data expected from the PC.

    Typical usage:
    <code>
    void main(void)
    {
        USBDeviceInit()
        while(1)
        {
            USBDeviceTasks();
            if((USBGetDeviceState() \< CONFIGURED_STATE) ||
               (USBIsDeviceSuspended() == TRUE))
            {
                //Either the device is not configured or we are suspended
                //  so we don't want to do execute any application code
                continue;   //go back to the top of the while loop
            }
            else
            {
                //Otherwise we are free to run user application code.
                UserApplication();
            }
        }
    }
    </code>

  Conditions:
    None
  Remarks:
    This function should be called preferably once every 100us during the
    enumeration process. After the enumeration process this function still
    needs to be called periodically to respond to various situations on the
    bus but is more relaxed in its time requirements.                      
  **************************************************************************/
void USBDeviceTasks(void);

/**************************************************************************
    Function:
        void USBDeviceInit(void)
    
    Description:
        This function initializes the device stack it in the default state. The
        USB module will be completely reset including all of the internal
        variables, registers, and interrupt flags.
                
    Precondition:
        This function must be called before any of the other USB Device
        functions can be called, including USBDeviceTasks().
        
    Parameters:
        None
     
    Return Values:
        None
        
    Remarks:
        None
                                                          
  **************************************************************************/
void USBDeviceInit(void);

/********************************************************************
  Function:
        bool USBGetRemoteWakeupStatus(void)
    
  Summary:
    This function indicates if remote wakeup has been enabled by the host.
    Devices that support remote wakeup should use this function to
    determine if it should send a remote wakeup.

  Description:
    This function indicates if remote wakeup has been enabled by the host.
    Devices that support remote wakeup should use this function to
    determine if it should send a remote wakeup.
    
    If a device does not support remote wakeup (the Remote wakeup bit, bit
    5, of the bmAttributes field of the Configuration descriptor is set to
    1), then it should not send a remote wakeup command to the PC and this
    function is not of any use to the device. If a device does support
    remote wakeup then it should use this function as described below.
    
    If this function returns FALSE and the device is suspended, it should
    not issue a remote wakeup (resume).
    
    If this function returns TRUE and the device is suspended, it should
    issue a remote wakeup (resume).
    
    A device can add remote wakeup support by having the _RWU symbol added
    in the configuration descriptor (located in the usb_descriptors.c file
    in the project). This done in the 8th byte of the configuration
    descriptor. For example:

    <code lang="c">
    ROM uint8_t configDescriptor1[]={
        0x09,                           // Size 
        USB_DESCRIPTOR_CONFIGURATION,   // descriptor type 
        DESC_CONFIG_uint16_t(0x0022),       // Total length 
        1,                              // Number of interfaces 
        1,                              // Index value of this cfg 
        0,                              // Configuration string index 
        _DEFAULT | _SELF | _RWU,        // Attributes, see usb_device.h 
        50,                             // Max power consumption in 2X mA(100mA)
        
        //The rest of the configuration descriptor should follow
    </code>

    For more information about remote wakeup, see the following section of
    the USB v2.0 specification available at www.usb.org: 
        * Section 9.2.5.2
        * Table 9-10 
        * Section 7.1.7.7 
        * Section 9.4.5

  Conditions:
    None

  Return Values:
    TRUE -   Remote Wakeup has been enabled by the host
    FALSE -  Remote Wakeup is not currently enabled

  Remarks:
    None
                                                                                                                                                                                                                                                                                                                       
  *******************************************************************/
bool USBGetRemoteWakeupStatus(void);
/*DOM-IGNORE-BEGIN*/
#define USBGetRemoteWakeupStatus() RemoteWakeup
/*DOM-IGNORE-END*/

/***************************************************************************
  Function:
        USB_DEVICE_STATE USBGetDeviceState(void)
    
  Summary:
    This function will return the current state of the device on the USB.
    This function should return CONFIGURED_STATE before an application
    tries to send information on the bus.
  Description:
    This function returns the current state of the device on the USB. This
    \function is used to determine when the device is ready to communicate
    on the bus. Applications should not try to send or receive data until
    this function returns CONFIGURED_STATE.
    
    It is also important that applications yield as much time as possible
    to the USBDeviceTasks() function as possible while the this function
    \returns any value between ATTACHED_STATE through CONFIGURED_STATE.
    
    For more information about the various device states, please refer to
    the USB specification section 9.1 available from www.usb.org.
    
    Typical usage:
    <code>
    void main(void)
    {
        USBDeviceInit()
        while(1)
        {
            USBDeviceTasks();
            if((USBGetDeviceState() \< CONFIGURED_STATE) ||
               (USBIsDeviceSuspended() == TRUE))
            {
                //Either the device is not configured or we are suspended
                //  so we don't want to do execute any application code
                continue;   //go back to the top of the while loop
            }
            else
            {
                //Otherwise we are free to run user application code.
                UserApplication();
            }
        }
    }
    </code>
  Conditions:
    None
  Return Values:
    USB_DEVICE_STATE - the current state of the device on the bus

  Remarks:
    None                                                                    
  ***************************************************************************/
USB_DEVICE_STATE USBGetDeviceState(void);
/*DOM-IGNORE-BEGIN*/
#define USBGetDeviceState() USBDeviceState
/*DOM-IGNORE-END*/

/***************************************************************************
  Function:
        bool USBGetSuspendState(void)
    
  Summary:
    This function indicates if this device is currently suspended. When a
    device is suspended it will not be able to transfer data over the bus.
  Description:
    This function indicates if this device is currently suspended. When a
    device is suspended it will not be able to transfer data over the bus.
    This function can be used by the application to skip over section of
    code that do not need to exectute if the device is unable to send data
    over the bus.
    
    Typical usage:
    <code>
       void main(void)
       {
           USBDeviceInit()
           while(1)
           {
               USBDeviceTasks();
               if((USBGetDeviceState() \< CONFIGURED_STATE) ||
                  (USBIsDeviceSuspended() == TRUE))
               {
                   //Either the device is not configured or we are suspended
                   //  so we don't want to do execute any application code
                   continue;   //go back to the top of the while loop
               }
               else
               {
                   //Otherwise we are free to run user application code.
                   UserApplication();
               }
           }
       }
    </code>
  Conditions:
    None
  Return Values:
    TRUE -   this device is suspended.
    FALSE -  this device is not suspended.
  Remarks:
    None                                                                    
  ***************************************************************************/
bool USBGetSuspendState(void);

/*******************************************************************************
  Function:
        void USBEnableEndpoint(uint8_t ep, uint8_t options)
    
  Summary:
    This function will enable the specified endpoint with the specified
    options
  Description:
    This function will enable the specified endpoint with the specified
    options.
    
    Typical Usage:
    <code>
    void USBCBInitEP(void)
    {
        USBEnableEndpoint(MSD_DATA_IN_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
        USBMSDInit();
    }
    </code>
    
    In the above example endpoint number MSD_DATA_IN_EP is being configured
    for both IN and OUT traffic with handshaking enabled. Also since
    MSD_DATA_IN_EP is not endpoint 0 (MSD does not allow this), then we can
    explicitly disable SETUP packets on this endpoint.
  Conditions:
    None
  Input:
    uint8_t ep -       the endpoint to be configured
    uint8_t options -  optional settings for the endpoint. The options should
                    be ORed together to form a single options string. The
                    available optional settings for the endpoint. The
                    options should be ORed together to form a single options
                    string. The available options are the following\:
                    * USB_HANDSHAKE_ENABLED enables USB handshaking (ACK,
                      NAK)
                    * USB_HANDSHAKE_DISABLED disables USB handshaking (ACK,
                      NAK)
                    * USB_OUT_ENABLED enables the out direction
                    * USB_OUT_DISABLED disables the out direction
                    * USB_IN_ENABLED enables the in direction
                    * USB_IN_DISABLED disables the in direction
                    * USB_ALLOW_SETUP enables control transfers
                    * USB_DISALLOW_SETUP disables control transfers
                    * USB_STALL_ENDPOINT STALLs this endpoint
  Return:
    None
  Remarks:
    None                                                                                                          
  *****************************************************************************/
void USBEnableEndpoint(uint8_t ep, uint8_t options);

/*******************************************************************************
  Function:
        bool USBIsDeviceSuspended(void)
    
  Summary:
    This function indicates if the USB module is in suspend mode.

  Description:
    This function indicates if the USB module is in suspend mode.  This function
    does NOT indicate that a suspend request has been received.  It only
    reflects the state of the USB module.
   
    Typical Usage:
    <code>
    if(USBIsDeviceSuspended() == TRUE)
    {
        return;
    }
    // otherwise do some application specific tasks
    </code>
    
  Conditions:
    None
  Input:
    None
  Return:
    None
  Remarks:
    None                                                                                                          
  *****************************************************************************/
bool USBIsDeviceSuspended(void);
/*DOM-IGNORE-BEGIN*/
#define USBIsDeviceSuspended() USBSuspendControl 
/*DOM-IGNORE-END*/


/*******************************************************************************
  Function:
        bool USBIsBusSuspended(void);
    
  Summary:
    This function indicates if the USB bus is in suspend mode.

  Description:
    This function indicates if the USB bus is in suspend mode.  This function
    is typically used for checking if the conditions are consistent with
    performing a USB remote wakeup sequence.
   
    Typical Usage:
    <code>
    if((USBIsBusSuspended() == TRUE) && (USBGetRemoteWakeupStatus() == TRUE))
    {
        //Check if some stimulus occured, which will be used as the wakeup source
        if(sw3 == 0)
        {
            USBCBSendResume();  //Send the remote wakeup signalling to the host
        }
    }
    // otherwise do some other application specific tasks
    </code>
    
  Conditions:
    None
  Input:
    None
  Return:
    None
  Remarks:
    The USBIsBusSuspended() function relies on the USBBusIsSuspended boolean 
    variable, which gets updated by the USBDeviceTasks() function.  Therefore,
    in order to be sure the return value is not "stale", it is suggested to make
    sure USBDeviceTasks() has executed recently (if using USB polling mode).
  *****************************************************************************/
bool USBIsBusSuspended(void);
/*DOM-IGNORE-BEGIN*/
#define USBIsBusSuspended() USBBusIsSuspended
/*DOM-IGNORE-END*/

/*******************************************************************************
  Function:
        void USBSoftDetach(void);
    
  Summary:
    This function performs a detach from the USB bus via software.

  Description:
    This function performs a detach from the USB bus via software.
    
  Conditions:
    None
  Input:
    None
  Return:
    None
  Remarks:
    Caution should be used when detaching from the bus.  Some PC drivers and 
    programs may require additional time after a detach before a device can be 
    reattached to the bus.                                                                                                          
  *****************************************************************************/
void USBSoftDetach(void);
/*DOM-IGNORE-BEGIN*/
#define USBSoftDetach()  U1CON = 0; U1IE = 0; USBDeviceState = DETACHED_STATE;
/*DOM-IGNORE-END*/

/*************************************************************************
  Function:
    USB_HANDLE USBTransferOnePacket(uint8_t ep, uint8_t dir, uint8_t* data, uint8_t len)
    
  Summary:
    Transfers a single packet (one transaction) of data on the USB bus.

  Description:
    The USBTransferOnePacket() function prepares a USB endpoint
    so that it may send data to the host (an IN transaction), or 
    receive data from the host (an OUT transaction).  The 
    USBTransferOnePacket() function can be used both to receive	and 
    send data to the host.  This function is the primary API function 
    provided by the USB stack firmware for sending or receiving application 
    data over the USB port.  

    The USBTransferOnePacket() is intended for use with all application 
    endpoints.  It is not used for sending or receiving applicaiton data 
    through endpoint 0 by using control transfers.  Separate API 
    functions, such as USBEP0Receive(), USBEP0SendRAMPtr(), and
    USBEP0SendROMPtr() are provided for this purpose.

    The	USBTransferOnePacket() writes to the Buffer Descriptor Table (BDT)
    entry associated with an endpoint buffer, and sets the UOWN bit, which 
    prepares the USB hardware to allow the transaction to complete.  The 
    application firmware can use the USBHandleBusy() macro to check the 
    status of the transaction, to see if the data has been successfully 
    transmitted yet.


    Typical Usage
    <code>
    //make sure that the we are in the configured state
    if(USBGetDeviceState() == CONFIGURED_STATE)
    {
        //make sure that the last transaction isn't busy by checking the handle
        if(!USBHandleBusy(USBInHandle))
        {
	        //Write the new data that we wish to send to the host to the INPacket[] array
	        INPacket[0] = USEFUL_APPLICATION_VALUE1;
	        INPacket[1] = USEFUL_APPLICATION_VALUE2;
	        //INPacket[2] = ... (fill in the rest of the packet data)
	      
            //Send the data contained in the INPacket[] array through endpoint "EP_NUM"
            USBInHandle = USBTransferOnePacket(EP_NUM,IN_TO_HOST,(uint8_t*)&INPacket[0],sizeof(INPacket));
        }
    }
    </code>

  Conditions:
    Before calling USBTransferOnePacket(), the following should be true.
    1.  The USB stack has already been initialized (USBDeviceInit() was called).
    2.  A transaction is not already pending on the specified endpoint.  This
        is done by checking the previous request using the USBHandleBusy() 
        macro (see the typical usage example).
    3.  The host has already sent a set configuration request and the 
        enumeration process is complete.
        This can be checked by verifying that the USBGetDeviceState() 
        macro returns "CONFIGURED_STATE", prior to calling 
        USBTransferOnePacket().
 					
  Input:
    uint8_t ep - The endpoint number that the data will be transmitted or 
	          received on
    uint8_t dir - The direction of the transfer
               This value is either OUT_FROM_HOST or IN_TO_HOST
    uint8_t* data - For IN transactions: pointer to the RAM buffer containing 
                 the data to be sent to the host.  For OUT transactions: pointer
                 to the RAM buffer that the received data should get written to.
   uint8_t len - Length of the data needing to be sent (for IN transactions).
              For OUT transactions, the len parameter should normally be set
              to the endpoint size specified in the endpoint descriptor.    

  Return Values:
    USB_HANDLE - handle to the transfer.  The handle is a pointer to 
                 the BDT entry associated with this transaction.  The
                 status of the transaction (ex: if it is complete or still
                 pending) can be checked using the USBHandleBusy() macro
                 and supplying the USB_HANDLE provided by
                 USBTransferOnePacket().

  Remarks:
    If calling the USBTransferOnePacket() function from within the USBCBInitEP()
    callback function, the set configuration is still being processed and the
    USBDeviceState may not be == CONFIGURED_STATE yet.  In this	special case, 
    the USBTransferOnePacket() may still be called, but make sure that the 
    endpoint has been enabled and initialized by the USBEnableEndpoint() 
    function first.  
    
  *************************************************************************/
USB_HANDLE USBTransferOnePacket(uint8_t ep,uint8_t dir,uint8_t* data,uint8_t len);

/*************************************************************************
  Function:
    bool USBHandleBusy(USB_HANDLE handle)
    
  Summary:
    Checks to see if the input handle is busy

  Description:
    Checks to see if the input handle is busy

    Typical Usage
    <code>
    //make sure that the last transfer isn't busy by checking the handle
    if(!USBHandleBusy(USBGenericInHandle))
    {
        //Send the data contained in the INPacket[] array out on
        //  endpoint USBGEN_EP_NUM
        USBGenericInHandle = USBGenWrite(USBGEN_EP_NUM,(uint8_t*)&INPacket[0],sizeof(INPacket));
    }
    </code>

  Conditions:
    None
  Input:
    USB_HANDLE handle -  handle of the transfer that you want to check the
                         status of
  Return Values:
    TRUE -   The specified handle is busy
    FALSE -  The specified handle is free and available for a transfer
  Remarks:
    None                                                                  
  *************************************************************************/
bool USBHandleBusy(USB_HANDLE handle);
/*DOM-IGNORE-BEGIN*/
#define USBHandleBusy(handle) (handle==0?0:((volatile BDT_ENTRY*)handle)->STAT.UOWN)
/*DOM-IGNORE-END*/

/********************************************************************
    Function:
        uint16_t USBHandleGetLength(USB_HANDLE handle)
        
    Summary:
        Retrieves the length of the destination buffer of the input
        handle
        
    Description:
        Retrieves the length of the destination buffer of the input
        handle

    PreCondition:
        None
        
    Parameters:
        USB_HANDLE handle - the handle to the transfer you want the
        address for.
        
    Return Values:
        uint16_t - length of the current buffer that the input handle
        points to.  If the transfer is complete then this is the 
        length of the data transmitted or the length of data
        actually received.
        
    Remarks:
        None
 
 *******************************************************************/
uint16_t USBHandleGetLength(USB_HANDLE handle);
/*DOM-IGNORE-BEGIN*/
#define USBHandleGetLength(handle) (((volatile BDT_ENTRY*)handle)->CNT)
/*DOM-IGNORE-END*/

/********************************************************************
    Function:
        uint16_t USBHandleGetAddr(USB_HANDLE)
        
    Summary:
        Retrieves the address of the destination buffer of the input
        handle
        
    Description:
        Retrieves the address of the destination buffer of the input
        handle

    PreCondition:
        None
        
    Parameters:
        USB_HANDLE handle - the handle to the transfer you want the
        address for.
        
    Return Values:
        uint16_t - address of the current buffer that the input handle
        points to.
       
    Remarks:
        None
 
 *******************************************************************/
uint16_t USBHandleGetAddr(USB_HANDLE);
/*DOM-IGNORE-BEGIN*/
#define USBHandleGetAddr(handle) ConvertToVirtualAddress((((volatile BDT_ENTRY*)handle)->ADR))
/*DOM-IGNORE-END*/

/********************************************************************
    Function:
        void USBEP0Transmit(uint8_t options)
        
    Summary:
        Sets the address of the data to send over the
        control endpoint
        
    PreCondition:
        None
        
    Paramters:
        options - the various options that you want
                  when sending the control data. Options are:
                       USB_EP0_ROM
                       USB_EP0_RAM
                       USB_EP0_BUSY
                       USB_EP0_INCLUDE_ZERO
                       USB_EP0_NO_DATA
                       USB_EP0_NO_OPTIONS
                       
    Return Values:
        None
    
    Remarks:
        None
 
 *******************************************************************/
void USBEP0Transmit(uint8_t options);
/*DOM-IGNORE-BEGIN*/
#define USBEP0Transmit(options) inPipes[0].info.Val = options | USB_EP0_BUSY
/*DOM-IGNORE-END*/

/*************************************************************************
  Function:
        void USBEP0SendRAMPtr(uint8_t* src, uint16_t size, uint8_t Options)
    
  Summary:
    Sets the source, size, and options of the data you wish to send from a
    RAM source
  Conditions:
    None
  Input:
    src -      address of the data to send
    size -     the size of the data needing to be transmitted
    options -  the various options that you want when sending the control
               data. Options are\:
               * USB_EP0_ROM
               * USB_EP0_RAM
               * USB_EP0_BUSY
               * USB_EP0_INCLUDE_ZERO
               * USB_EP0_NO_DATA
               * USB_EP0_NO_OPTIONS
  Remarks:
    None                                                                  
  *************************************************************************/
void USBEP0SendRAMPtr(uint8_t* src, uint16_t size, uint8_t Options);
/*DOM-IGNORE-BEGIN*/
#define USBEP0SendRAMPtr(src,size,options)  {\
            inPipes[0].pSrc.bRam = src;\
            inPipes[0].wCount.Val = size;\
            inPipes[0].info.Val = options | USB_EP0_BUSY | USB_EP0_RAM;\
            }
/*DOM-IGNORE-END*/

/**************************************************************************
  Function:
        void USBEP0SendROMPtr(uint8_t* src, uint16_t size, uint8_t Options)
    
  Summary:
    Sets the source, size, and options of the data you wish to send from a
    ROM source
  Conditions:
    None
  Input:
    src -      address of the data to send
    size -     the size of the data needing to be transmitted
    options -  the various options that you want when sending the control
               data. Options are\:
               * USB_EP0_ROM
               * USB_EP0_RAM
               * USB_EP0_BUSY
               * USB_EP0_INCLUDE_ZERO
               * USB_EP0_NO_DATA
               * USB_EP0_NO_OPTIONS
  Remarks:
    None                                                                   
  **************************************************************************/
void USBEP0SendROMPtr(uint8_t* src, uint16_t size, uint8_t Options);
/*DOM-IGNORE-BEGIN*/
#define USBEP0SendROMPtr(src,size,options)  {\
            inPipes[0].pSrc.bRom = src;\
            inPipes[0].wCount.Val = size;\
            inPipes[0].info.Val = options | USB_EP0_BUSY | USB_EP0_ROM;\
            }
/*DOM-IGNORE-END*/

/***************************************************************************
  Function:
    void USBEP0Receive(uint8_t* dest, uint16_t size, void (*function))
  Summary:
    Sets the destination, size, and a function to call on the completion of
    the next control write.
  Conditions:
    None
  Input:
    dest -        address of where the incoming data will go (make sure that this
                  address is directly accessable by the USB module for parts with
                  dedicated USB RAM this address must be in that space)
    size -        the size of the data being received (is almost always going tobe
                  presented by the preceeding setup packet SetupPkt.wLength)
    (*function) - a function that you want called once the data is received. If
                  this is specificed as NULL then no function is called.
  Remarks:
    None                                                                    
  ***************************************************************************/
void USBEP0Receive(uint8_t* dest, uint16_t size, void (*function));
/*DOM-IGNORE-BEGIN*/
#define USBEP0Receive(dest,size,function)  {outPipes[0].pDst.bRam = dest;outPipes[0].wCount.Val = size;outPipes[0].pFunc = function;outPipes[0].info.bits.busy = 1; }
/*DOM-IGNORE-END*/

/********************************************************************
    Function:
        USB_HANDLE USBTxOnePacket(uint8_t ep, uint8_t* data, uint16_t len)
        
    Summary:
        Sends the specified data out the specified endpoint
        
    PreCondition:
        None
        
    Parameters:
        ep - the endpoint you want to send the data out of
        data - the data that you wish to send
        len - the length of the data that you wish to send
        
    Return Values:
        USB_HANDLE - a handle for the transfer.  This information
        should be kept to track the status of the transfer
        
    Remarks:
        None
  
 *******************************************************************/
USB_HANDLE USBTxOnePacket(uint8_t ep, uint8_t* data, uint16_t len);
/*DOM-IGNORE-BEGIN*/
#define USBTxOnePacket(ep,data,len)     USBTransferOnePacket(ep,IN_TO_HOST,data,len)
/*DOM-IGNORE-END*/

/********************************************************************
    Function:
        USB_HANDLE USBRxOnePacket(uint8_t ep, uint8_t* data, uint16_t len)
        
    Summary:
        Receives the specified data out the specified endpoint
        
    PreCondition:
        None
        
    Parameters:
        ep - the endpoint you want to receive the data into
        data - where the data will go when it arrives
        len - the length of the data that you wish to receive
        
    Return Values:
        None
        
    Remarks:
        None
  
 *******************************************************************/
USB_HANDLE USBRxOnePacket(uint8_t ep, uint8_t* data, uint16_t len);
/*DOM-IGNORE-BEGIN*/
#define USBRxOnePacket(ep,data,len)      USBTransferOnePacket(ep,OUT_FROM_HOST,data,len)
/*DOM-IGNORE-END*/

/********************************************************************
    Function:
        void USBStallEndpoint(uint8_t ep, uint8_t dir)
        
    Summary:
         STALLs the specified endpoint
    
    PreCondition:
        None
        
    Parameters:
        uint8_t ep - the endpoint the data will be transmitted on
        uint8_t dir - the direction of the transfer
        
    Return Values:
        None
        
    Remarks:
        None

 *******************************************************************/
void USBStallEndpoint(uint8_t ep, uint8_t dir);

/**************************************************************************
    Function:
        void USBDeviceDetach(void)
   
    Summary:
        This function indicates to the USB module that the USB device has been
        detached from the bus.

    Description:
        This function indicates to the USB module that the USB device has been
        detached from the bus.  This function needs to be called in order for the
        device to start to properly prepare for the next attachment.
   
    Precondition:
        Should only be called when USB_INTERRUPT is defined.

    Parameters:
        None
     
    Return Values:
        None
        
    Remarks:
        None
                                                          
  **************************************************************************/
void USBDeviceDetach(void);

/*DOM-IGNORE-BEGIN*/
#if !defined(USB_INTERRUPT)
    #define USBDeviceDetach() 
#endif
/*DOM-IGNORE-END*/

/**************************************************************************
    Function:
        void USBDeviceAttach(void)
    
    Summary:
        Checks if VBUS is present, and that the USB module is not already 
        initalized, and if so, enables the USB module so as to signal device 
        attachment to the USB host.   

    Description:
        This function indicates to the USB host that the USB device has been
        attached to the bus.  This function needs to be called in order for the
        device to start to enumerate on the bus.
                
    Precondition:
        Should only be called when USB_INTERRUPT is defined.  Also, should only 
        be called from the main() loop context.  Do not call USBDeviceAttach()
        from within an interrupt handler, as the USBDeviceAttach() function
        may modify global interrupt enable bits and settings.

        For normal USB devices:
        Make sure that if the module was previously on, that it has been turned off 
        for a long time (ex: 100ms+) before calling this function to re-enable the module.
        If the device turns off the D+ (for full speed) or D- (for low speed) ~1.5k ohm
        pull up resistor, and then turns it back on very quickly, common hosts will sometimes 
        reject this event, since no human could ever unplug and reattach a USB device in a 
        microseconds (or nanoseconds) timescale.  The host could simply treat this as some kind 
        of glitch and ignore the event altogether.  
    Parameters:
        None
     
    Return Values:
        None                                                        
****************************************************************************/
void USBDeviceAttach(void);

/*DOM-IGNORE-BEGIN*/
#if !defined(USB_INTERRUPT)
    #define USBDeviceAttach() 
#endif
/*DOM-IGNORE-END*/

/*******************************************************************************
  Function:
    bool USB_APPLICATION_EVENT_HANDLER(uint8_t address, USB_EVENT event, void *pdata, uint16_t size);
    
  Summary:
    This function is called whenever the USB stack wants to notify the user of
    an event.

  Description:
    This function is called whenever the USB stack wants to notify the user of
    an event.  This function should be implemented by the user.
    
    Example Usage:
  Conditions:
    None

  Input:
    uint8_t address -  the address of the device when the event happened
    uint8_t event   -  The event input specifies which event happened.  The
                    possible options are listed in the USB_DEVICE_STACK_EVENTS
                    enumeration.

  Return:
    None
  Remarks:
    None                                                                                                          
  *****************************************************************************/
bool USB_APPLICATION_EVENT_HANDLER(uint8_t address, USB_EVENT event, void *pdata, uint16_t size);

/*******************************************************************************
  Function:
    ROM void *USBDeviceCBGetDescriptor (UINT16 *length, DESCRIPTOR_ID *id);
    
  Summary:
    This function is called whenever the USB stack gets a USB GET_DESCRIPTOR
    request.

  Description:
    This function is called whenever the USB stack gets a USB GET_DESCRIPTOR
    request.  This function is responsible for returning a pointer to the 
    requested descriptor and setting that the length for the that descriptor.

    This function should be implemented by the user.  This function might be 
    generated automatically by the USB configuration tool.
    
  Conditions:
    None

  Input:
    uint8_t *length -  pointer to a variable that should be set to the length of 
                    the requested descriptor.  
    uint8_t *id     -  This structure contains information about the requested
                    descriptor

  Return:
    ROM void* - pointer to the requested descriptor.
  Remarks:
    None                                                                                                          
  *****************************************************************************/
void *USBDeviceCBGetDescriptor (    uint16_t *length, 
                                    uint8_t *ptr_type,
                                    DESCRIPTOR_ID *id);


/*******************************************************************************
  Function: void USBCtrlEPAllowStatusStage(void);
    
  Summary: This function prepares the proper endpoint 0 IN or endpoint 0 OUT 
            (based on the controlTransferState) to allow the status stage packet
            of a control transfer to complete.  This function gets used 
            internally by the USB stack itself, but it may also be called from
            the application firmware, IF the application firmware called
            the USBDeferStatusStage() function during the initial processing
            of the control transfer request.  In this case, the application
            must call the USBCtrlEPAllowStatusStage() once, after it has fully
            completed processing and handling the data stage portion of the
            request.  
            
            If the application firmware has no need for delaying control 
            transfers, and therefore never calls USBDeferStatusStage(), then the
            application firmware should not call USBCtrlEPAllowStatusStage().
            
  Description:
    
  Conditions:
    None

  Input:

  Return:

  Remarks:
    None                                                                                                          
  *****************************************************************************/
void USBCtrlEPAllowStatusStage(void);

/*******************************************************************************
  Function: void USBDeferStatusStage(void);
    
  Summary: Calling this function will prevent the USB stack from automatically
            enabling the status stage for the currently pending control transfer
            from completing immediately after all data bytes have been sent or 
            received.  This is useful if a class handler or USB application 
            firmware project uses control transfers for sending/receiving data 
            over EP0, but requires time in order to finish processing and/or to 
            consume the data.
            
            For example: Consider an application which receives OUT data from the
            USB host, through EP0 using control transfers.  Now assume that this
            application wishes to do something time consuming with this data (ex:
            transmit it to and save it to an external EEPROM device, connected 
            via SPI/I2C/etc.).  In this case, it would typically be desireable to
            defer allowing the USB status stage of the control transfer to complete,
            until after the data has been fully sent to the EEPROM device and saved.
            
            If the USB class handler firmware that processes the control transfer
            SETUP packet determines that it will need extra time to complete the
            control transfer, it may optionally call USBDeferStatusStage().  If it
            does so, it is then the responsibility of the application firmware to
            eventually call USBCtrlEPAllowStatusStage(), once the firmware has
            finished processing the data associated with the control transfer.
            
            If the firmware call USBDeferStatusStage(), but never calls 
            USBCtrlEPAllowStatusStage(), then one of two possibilities will occur.
            
            1.  If the "USB_ENABLE_STATUS_STAGE_TIMEOUTS" option is commented in
                usb_config.h, then the status stage of the control transfer will
                never be able to complete.  This is an error case and should be
                avoided.
            2.  If the "USB_ENABLE_STATUS_STAGE_TIMEOUTS" option is enabled in 
                usb_config.h, then the USBDeviceTasks() function will 
                automatically call USBCtrlEPAllowStatusStage(), after the 
                "USB_STATUS_STAGE_TIMEOUT" has elapsed, since the last quanta of
                "progress" has occurred in the control transfer.  Progress is 
                defined as the last successful transaction completing on EP0 IN or
                EP0 OUT.
                Although the timeouts feature allows the status stage to
                [eventually] complete, it is still preferable to manually call 
                USBCtrlEPAllowStatusStage() after the application firmware has
                finished processing/consuming the control transfer data, as this
                will allow for much faster processing of control transfers, and
                therefore much higher data rates and better user responsiveness.
  Description:
    
  Conditions:
    None

  Input:

  Return:

  Remarks:  If this function is called, is should get called after the SETUP
            packet has arrived (the control transfer has started), but before
            the USBCtrlEPServiceComplete() function has been called by the USB
            stack.  Therefore, the normal place to call USBDeferStatusStage()
            would be from within the USBCBCheckOtherReq() handler context.  For
            example, in a HID application using control transfers, the
            USBDeferStatusStage() function would be called from within the
            USER_GET_REPORT_HANDLER or USER_SET_REPORT_HANDLER functions.                                                                                                                
  *****************************************************************************/
void USBDeferStatusStage(void);
extern volatile bool USBDeferStatusStagePacket;
/*DOM-IGNORE-BEGIN*/
#define USBDeferStatusStage()   {USBDeferStatusStagePacket = TRUE;}
/*DOM-IGNORE-END*/


/*******************************************************************************
  Function: void USBCtrlEPAllowDataStage(void);
    
  Summary: This function allows the data stage of either a host-to-device or
            device-to-host control transfer (with data stage) to complete.
            This function is meant to be used in conjunction with either the
            USBDeferOUTDataStage() or USBDeferINDataStage().  If the firmware
            does not call either USBDeferOUTDataStage() or USBDeferINDataStage(),
            then the firmware does not need to manually call 
            USBCtrlEPAllowDataStage(), as the USB stack will call this function
            instead.
     
  Description:
    
  Conditions: A control transfer (with data stage) should already be pending, 
                if the firmware calls this function.  Additionally, the firmware
                should have called either USBDeferOUTDataStage() or 
                USBDeferINDataStage() at the start of the control transfer, if
                the firmware will be calling this function manually.

  Input:

  Return:

  Remarks: 
  *****************************************************************************/
void USBCtrlEPAllowDataStage(void);


/*******************************************************************************
  Function: void USBDeferOUTDataStage(void);
    
  Summary: This function will cause the USB hardware to continuously NAK the
           OUT data packets sent from the host, during the data stage of a
           device to host control transfer.  This allows the firmware more time
           to prepare the RAM buffer that will eventually be used to receive the
           data from the host.  This is also useful, if the firmware wishes to
           receive the OUT data in a different context than what the
           USBDeviceTasks() function executes at.
           
           Calling this function (macro) will assert ownership of the currently
           pending control transfer.  Therefore, the USB stack will not STALL
           when it reaches the data stage of the control transfer, even if the
           firmware has not (yet) called the USBEP0Receive() API function.  
           However, the application firware must still (eventually, once it is 
           ready) call one of the aforementioned API function.
           
           Example Usage:
           
           1.  Host sends a SETUP packet to the device, requesting a host to 
                device control transfer, with data stage (OUT data packets).
           2.  USBDeviceTasks() executes, and then calls the USBCBCheckOtherReq()
                callback event handler.  The USBCBCheckOtherReq() calls the
                application specific/device class specific handler that detects
                the type of control transfer.
           3.  If the firmware needs more time before it wishes to receive the 
                first OUT data packet, or, if the firmware wishes to process the 
                command in a different context, then it may call 
                USBDeferOUTDataStage(), in the context of the 
                USBCBCheckOtherReq() handler function.
           4.  If the firmware called USBDeferOUTDataStage() in step #3 above,
                then the hardware will NAK the OUT data packets sent by the
                host, for the OUT data stage.
           5.  Once the firmware is ready, it should then call USBEP0Receive(), 
                to prepare the USB stack to receive the OUT data from the host,
                and to write it to the user specified buffer.
           6.  The firmware should now call USBCtrlEPAllowDataStage().  This
                will allow the data stage to complete.  Once all OUT data has
                been received, the user callback function (provided by the
                function pointer provided when calling USBEP0Receive()) will
                get called.            
           7.  Once all data has been received from the host, the status stage 
                (a 0-byte IN packet) will complete automatically (assuming the 
                firmware did not call USBDeferStatusStage() during step #3).
            
     
  Description:
    
  Conditions: Before calling USBDeferOUTDataStage(), the firmware should first
                verify that the control transfer has a data stage, and that
                it is of type host-to-device (OUT).

  Input:

  Return:

  Remarks: Section 9.2.6 of the official USB 2.0 specifications indicates that
           the USB device must be able to receive all bytes and complete the
           control transfer within a maximum of 5 seconds.
            
           If the firmware calls USBDeferOUTDataStage(), it must eventually call
           USBEP0Receive(), and then call USBCtrlEPAllowDataStage().  If it does 
           not do this, the control transfer will never be able to complete.
           This will break the USB connection, as the host needs to be able to
           communicate over EP0, in order to perform basic tasks including 
           enumeration. 
           
           The firmware should never call both USBDeferINDataStage() and 
           USBDeferOUTDataStage() during the same control transfer.  These 
           functions are mutually exclusive (a control transfer with data stage
           can never contain both IN and OUT data packets during the data stage).
  *****************************************************************************/
void USBDeferOUTDataStage(void);
extern volatile bool USBDeferOUTDataStagePackets;
/*DOM-IGNORE-BEGIN*/
#define USBDeferOUTDataStage()   {USBDeferOUTDataStagePackets = TRUE; outPipes[0].info.bits.busy = 1;}
/*DOM-IGNORE-END*/



/*******************************************************************************
  Function: bool USBOUTDataStageDeferred(void);
    
  Summary: Returns TRUE if a control transfer with OUT data stage is pending,
            and the firmware has called USBDeferOUTDataStage(), but has not
            yet called USBCtrlEPAllowDataStage().
           Returns FALSE if a control transfer with OUT data stage is either
            not pending, or the firmware did not call USBDeferOUTDataStage()
            at the start of the control transfer.
            
           This function (macro) would typically be used in the case where the
           USBDeviceTasks() function executes in the interrupt context (ex:
           USB_INTERRUPT option selected in usb_config.h), but the firmware
           wishes to take care of handling the data stage of the control transfer
           in the main loop context.  
           
           In this scenario, typical usage would be:
           1.  Host starts a control transfer with OUT data stage.
           2.  USBDeviceTasks() (in this scenario, interrupt context) executes.
           3.  USBDeviceTasks() calls USBCBCheckOtherReq(), which in turn
                determines that the control transfer is class specific, with
                OUT data stage.
           4.  The user code in USBCBCheckOtherReq() (also in interrupt context,
                since it is called from USBDeviceTasks(), and therefore executes
                at the same priority/context) calls USBDeferOUTDataStage().
           
           5.  Meanwhile, in the main loop context, a polling handler may be 
                periodically checking if(USBOUTDataStageDeferred() == TRUE).
                Ordinarily, it would evaluate false, but when a control transfer
                becomes pending, and after the USBDeferOUTDataStage() macro has
                been called (ex: in the interrupt context), the if() statement
                will evaluate true.  In this case, the main loop context can then
                take care of receiving the data, by calling USBEP0Receive() and
                USBCtrlEPAllowDataStage().
     
  Description:
    
  Conditions: 

  Input:

  Return:

  Remarks: 
  *****************************************************************************/
bool USBOUTDataStageDeferred(void);
/*DOM-IGNORE-BEGIN*/
#define USBOUTDataStageDeferred() USBDeferOUTDataStagePackets
/*DOM-IGNORE-END*/

/*******************************************************************************
  Function: void USBDeferINDataStage(void);
    
  Summary: This function will cause the USB hardware to continuously NAK the
           IN token packets sent from the host, during the data stage of a
           device to host control transfer.  This allows the firmware more time
           to process and prepare the IN data packets that will eventually be
           sent to the host.  This is also useful, if the firmware needs to
           process/prepare the IN data in a different context than what the
           USBDeviceTasks() function executes at.
           
           Calling this function (macro) will assert ownership of the currently
           pending control transfer.  Therefore, the USB stack will not STALL
           when it reaches the data stage of the control transfer, even if the
           firmware has not (yet) called the USBEP0SendRAMPtr() or 
           USBEP0SendROMPtr() API function.  However, the application firware 
           must still (eventually, once it is ready) call one of the 
           aforementioned API functions.
           
           Example Usage:
           
           1.  Host sends a SETUP packet to the device, requesting a device to 
                host control transfer, with data stage.
           2.  USBDeviceTasks() executes, and then calls the USBCBCheckOtherReq()
                callback event handler.  The USBCBCheckOtherReq() calls the
                application specific/device class specific handler that detects
                the type of control transfer.
           3.  If the firmware needs more time to prepare the first IN data packet,
                or, if the firmware wishes to process the command in a different
                context (ex: if USBDeviceTasks() executes as an interrupt handler,
                but the IN data stage data needs to be prepared in the main loop
                context), then it may call USBDeferINDataStage(), in the context
                of the USBCBCheckOtherReq() handler function.
           4.  If the firmware called USBDeferINDataStage() in step #3 above,
                then the hardware will NAK the IN token packets sent by the
                host, for the IN data stage.
           5.  Once the firmware is ready, and has successfully prepared the
                data to be sent to the host in fulfillment of the control
                transfer, it should then call USBEP0SendRAMPtr() or
                USBEP0SendROMPtr(), to prepare the USB stack to know how many
                bytes to send to the host, and from what source location.
           6.  The firmware should now call USBCtrlEPAllowDataStage().  This
                will allow the data stage to complete.  The USB stack will send
                the data buffer specified by the USBEP0SendRAMPtr() or 
                USBEP0SendROMPtr() function, when it was called.
           7.  Once all data has been sent to the host, or if the host performs
                early termination, the status stage (a 0-byte OUT packet) will
                complete automatically (assuming the firmware did not call
                USBDeferStatusStage() during step #3).
            
     
  Description:
    
  Conditions: Before calling USBDeferINDataStage(), the firmware should first
                verify that the control transfer has a data stage, and that
                it is of type device-to-host (IN).

  Input:

  Return:

  Remarks: Section 9.2.6 of the official USB 2.0 specifications indicates that
            the USB device must return the first IN data packet within 500ms
            of the start of the control transfer.  In order to meet this
            specification, the firmware must call USBEP0SendRAMPtr() or
            USBEP0SendROMPtr(), and then call USBCtrlEPAllowDataStage(), in
            less than 500ms from the start of the control transfer.    
            
           If the firmware calls USBDeferINDataStage(), it must eventually call
           USBEP0SendRAMPtr() or USBEP0SendROMPtr(), and then call 
           USBCtrlEPAllowDataStage().  If it does not do this, the control
           transfer will never be able to complete.
           
           The firmware should never call both USBDeferINDataStage() and 
           USBDeferOUTDataStage() during the same control transfer.  These 
           functions are mutually exclusive (a control transfer with data stage
           can never contain both IN and OUT data packets during the data stage).
  *****************************************************************************/
void USBDeferINDataStage(void);
extern volatile bool USBDeferINDataStagePackets;
/*DOM-IGNORE-BEGIN*/
#define USBDeferINDataStage()   {USBDeferINDataStagePackets = TRUE; inPipes[0].info.bits.busy = 1;}
/*DOM-IGNORE-END*/



/*******************************************************************************
  Function: bool USBINDataStageDeferred(void);
    
  Summary: Returns TRUE if a control transfer with IN data stage is pending,
            and the firmware has called USBDeferINDataStage(), but has not
            yet called USBCtrlEPAllowDataStage().
           Returns FALSE if a control transfer with IN data stage is either
            not pending, or the firmware did not call USBDeferINDataStage()
            at the start of the control transfer.
            
           This function (macro) would typically be used in the case where the
           USBDeviceTasks() function executes in the interrupt context (ex:
           USB_INTERRUPT option selected in usb_config.h), but the firmware
           wishes to take care of handling the data stage of the control transfer
           in the main loop context.  
           
           In this scenario, typical usage would be:
           1.  Host starts a control transfer with IN data stage.
           2.  USBDeviceTasks() (in this scenario, interrupt context) executes.
           3.  USBDeviceTasks() calls USBCBCheckOtherReq(), which in turn
                determines that the control transfer is class specific, with
                IN data stage.
           4.  The user code in USBCBCheckOtherReq() (also in interrupt context,
                since it is called from USBDeviceTasks(), and therefore executes
                at the same priority/context) calls USBDeferINDataStage().
           
           5.  Meanwhile, in the main loop context, a polling handler may be 
                periodically checking if(USBINDataStageDeferred() == TRUE).
                Ordinarily, it would evaluate false, but when a control transfer
                becomes pending, and after the USBDeferINDataStage() macro has
                been called (ex: in the interrupt context), the if() statement
                will evaluate true.  In this case, the main loop context can then
                take care of sending the data (when ready), by calling 
                USBEP0SendRAMPtr() or USBEP0SendROMPtr() and 
                USBCtrlEPAllowDataStage().
     
  Description:
    
  Conditions: 

  Input:

  Return:

  Remarks: 
  *****************************************************************************/
bool USBINDataStageDeferred(void);
/*DOM-IGNORE-BEGIN*/
#define USBINDataStageDeferred() USBDeferINDataStagePackets
/*DOM-IGNORE-END*/



/**************************************************************************
    Function:
        void USBCancelIO(uint8_t endpoint)
    
    Description:
        This function cancels the transfers pending on the specified endpoint.
        This function can only be used after a SETUP packet is received and 
        before that setup packet is handled.  This is the time period in which
        the EVENT_EP0_REQUEST is thrown, before the event handler function
        returns to the stack.

    Precondition:
  
    Parameters:
        uint8_t endpoint - the endpoint number you wish to cancel the transfers for
     
    Return Values:
        None
        
    Remarks:
        None
                                                          
  **************************************************************************/
void USBCancelIO(uint8_t endpoint);


/** Section: MACROS ******************************************************/

/* The DESC_CONFIG_uint16_t() macro is implemented for convinence.  Since the 
    configuration descriptor array is a uint8_t array, each entry needs to be a
    uint8_t in LSB format.  The DESC_CONFIG_uint16_t() macro breaks up a uint16_t into 
    the appropriate uint8_t entries in LSB.
    Typical Usage:
    <code>
        ROM uint8_t configDescriptor1[]={
            0x09,                           // Size of this descriptor in bytes
            USB_DESCRIPTOR_CONFIGURATION,   // CONFIGURATION descriptor type
            DESC_CONFIG_uint16_t(0x0022),       // Total length of data for this cfg
    </code>
*/
#define DESC_CONFIG_uint16_t(a) (a&0xFF),((a>>8)&0xFF)

/* The DESC_CONFIG_Duint16_t() macro is implemented for convinence.  Since the 
    configuration descriptor array is a uint8_t array, each entry needs to be a
    uint8_t in LSB format.  The DESC_CONFIG_Duint16_t() macro breaks up a Duint16_t into 
    the appropriate uint8_t entries in LSB.
*/
#define DESC_CONFIG_Duint16_t(a) (a&0xFF),((a>>8)&0xFF),((a>>16)&0xFF),((a>>24)&0xFF)

/* The DESC_CONFIG_uint8_t() macro is implemented for convinence.  The 
    DESC_CONFIG_uint8_t() macro provides a consistant macro for use with a byte
    when generating a configuratin descriptor when using either the 
    DESC_CONFIG_uint16_t() or DESC_CONFIG_Duint16_t() macros.
*/
#define DESC_CONFIG_uint8_t(a) (a)
















/* DOM-IGNORE-BEGIN */
/*******************************************************************************
********************************************************************************
********************************************************************************
    This section contains implementation specific information that may vary
    between releases as the implementation needs to change.  This section is
    included for compilation reasons only.
********************************************************************************
********************************************************************************
*******************************************************************************/

#if defined(USB_POLLING)
    #define USB_VOLATILE
#else
    #define USB_VOLATILE volatile
#endif

#define CTRL_TRF_RETURN void
#define CTRL_TRF_PARAMS void

// Defintion of the PIPE structure
//  This structure is used to keep track of data that is sent out
//  of the stack automatically.

/*typedef union { //Added to allow for WORD_VAL
    uint16_t word;
    struct {
        uint8_t low;
        uint8_t high;
    };
} WORD_VAL;*/

typedef struct __attribute__ ((packed))
{
    union __attribute__ ((packed))
    {
        //Various options of pointers that are available to
        // get the data from
        uint8_t *bRam;
        ROM uint8_t *bRom;
        uint16_t *wRam;
        ROM uint16_t *wRom;
    }pSrc;
    union __attribute__ ((packed))
    {
        struct __attribute__ ((packed))
        {
            //is this transfer from RAM or ROM?
            uint8_t ctrl_trf_mem          :1;
            uint8_t reserved              :5;
            //include a zero length packet after
            //data is done if data_size%ep_size = 0?
            uint8_t includeZero           :1;
            //is this PIPE currently in use
            uint8_t busy                  :1;
        }bits;
        uint8_t Val;
    }info;
    WORD_VAL __attribute__((aligned)) wCount;
}IN_PIPE;

extern USB_VOLATILE IN_PIPE inPipes[];

typedef struct __attribute__ ((packed))
{
    union __attribute__ ((packed))
    {
        //Various options of pointers that are available to
        // get the data from
        uint8_t *bRam;
        uint16_t *wRam;
    }pDst;
    union __attribute__ ((packed))
    {
        struct __attribute__ ((packed))
        {
            uint8_t reserved              :7;
            //is this PIPE currently in use
            uint8_t busy                  :1;
        }bits;
        uint8_t Val;
    }info;
    WORD_VAL wCount;
    CTRL_TRF_RETURN (*pFunc)(CTRL_TRF_PARAMS);
}OUT_PIPE;

/************* DWF - SHOULD BE REIMPLEMENTED AS AN EVENT *******************/
//#if defined(ENABLE_EP0_DATA_RECEIVED_CALLBACK)
//    void USBCBEP0DataReceived(void);
//    #define USBCB_EP0_DATA_RECEIVED() USBCBEP0DataReceived()
//#else
//    #define USBCB_EP0_DATA_RECEIVED()
//#endif

extern USB_VOLATILE bool RemoteWakeup;
extern USB_VOLATILE bool USBBusIsSuspended;
extern USB_VOLATILE USB_DEVICE_STATE USBDeviceState;
extern USB_VOLATILE uint8_t USBActiveConfiguration;
/******************************************************************************/
/* DOM-IGNORE-END */

#endif //USBD_H
