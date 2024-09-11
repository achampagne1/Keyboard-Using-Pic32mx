#ifndef __USB_DESCRIPTORS_C
#define __USB_DESCRIPTORS_C

/** INCLUDES *******************************************************/
#include "usb.h"
#include "usb_function_hid.h"

/** CONSTANTS ******************************************************/
/* Device Descriptor */
ROM USB_DEVICE_DESCRIPTOR device_dsc=
{
    0x12,                   // Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,  // DEVICE descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
    0x00,                   // Class Code
    0x00,                   // Subclass code
    0x00,                   // Protocol code
    USB_EP0_BUFF_SIZE,      // Max packet size for EP0, see usb_config.h
    MY_VID,                 // Vendor ID
    MY_PID,                 // Product ID: Keyboard demo
    0x0003,                 // Device release number in BCD format
    0x01,                   // Manufacturer string index
    0x02,                   // Product string index
    0x00,                   // Device serial number string index
    0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */
ROM uint8_t configDescriptor1[] = {
    /* Configuration Descriptor */
    0x09,                       // Size of this descriptor in bytes
    USB_DESCRIPTOR_CONFIGURATION,                // CONFIGURATION descriptor type
    DESC_CONFIG_uint16_t(0x0022),   // Total length of data for this cfg (34 bytes)
    1,                            // Number of interfaces in this cfg
    1,                            // Index value of this configuration
    0,                            // Configuration string index
    _DEFAULT | _SELF,             // Attributes, see usb_device.h
    50,                           // Max power consumption (2X mA)

    /* Interface Descriptor */
    0x09,                         // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,     // INTERFACE descriptor type
    0,                            // Interface Number
    0,                            // Alternate Setting Number
    1,                            // Number of endpoints in this intf
    HID_INTF,                     // Class code
    BOOT_INTF_SUBCLASS,           // Subclass code
    HID_PROTOCOL_KEYBOARD,        // Protocol code
    0,                            // Interface string index

    /* HID Class-Specific Descriptor */
    0x09,                         // Size of this descriptor in bytes
    DSC_HID,                      // HID descriptor type
    DESC_CONFIG_uint16_t(0x0111),  // HID Spec Release Number in BCD format (1.11)
    0x00,                         // Country Code (0x00 for Not supported)
    HID_NUM_OF_DSC,               // Number of class descriptors, see usbcfg.h
    DSC_RPT,                      // Report descriptor type
    DESC_CONFIG_uint16_t(45),      // Size of the report descriptor (45 bytes)

    /* Endpoint Descriptor */
    0x07,                         // Size of this descriptor in bytes
    USB_DESCRIPTOR_ENDPOINT,      // Endpoint Descriptor
    HID_EP | _EP_IN,              // Endpoint Address
    _INTERRUPT,                   // Attributes
    DESC_CONFIG_uint16_t(0x08),   // Size of the endpoint (8 bytes)
    0x0A                          // Interval (10 ms)
};

/* HID Report Descriptor (Keyboard) */
ROM struct{uint8_t report[HID_RPT01_SIZE];} hid_rpt01 = {
    {0x05, 0x01,        /* Usage Page (Generic Desktop)             */
    0x09, 0x06,        /* Usage (Keyboard)                         */
    0xA1, 0x01,        /* Collection (Application)                 */
    
    0x05, 0x07,        /*   Usage Page (Key Codes)                 */
    0x19, 0xE0,        /*   Usage Minimum (Left Control)           */
    0x29, 0xE7,        /*   Usage Maximum (Right GUI)              */
    0x15, 0x00,        /*   Logical Minimum (0)                    */
    0x25, 0x01,        /*   Logical Maximum (1)                    */
    0x75, 0x01,        /*   Report Size (1)                        */
    0x95, 0x08,        /*   Report Count (8)                       */
    0x81, 0x02,        /*   Input (Data, Variable, Absolute)       */
    
    0x95, 0x01,        /*   Report Count (1)                       */
    0x75, 0x08,        /*   Report Size (8)                        */
    0x81, 0x01,        /*   Input (Constant) ; Reserved byte       */
    
    0x95, 0x06,        /*   Report Count (6)                       */
    0x75, 0x08,        /*   Report Size (8)                        */
    0x15, 0x00,        /*   Logical Minimum (0)                    */
    0x25, 0x65,        /*   Logical Maximum (101)                  */
    0x05, 0x07,        /*   Usage Page (Key Codes)                 */
    0x19, 0x00,        /*   Usage Minimum (0)                      */
    0x29, 0x65,        /*   Usage Maximum (101)                    */
    0x81, 0x00,        /*   Input (Data, Array)                    */
    
    0xC0               /* End Collection                           */
    }
};

//Language code string descriptor
ROM struct{uint8_t bLength; uint8_t bDscType; uint16_t string[1];} sd000 = {
    sizeof(sd000), USB_DESCRIPTOR_STRING, {0x0409}  // English (United States)
};

//Manufacturer string descriptor
ROM struct{uint8_t bLength; uint8_t bDscType; uint16_t string[10];} sd001 = {
    sizeof(sd001), USB_DESCRIPTOR_STRING,
    {'A','C',' ','D','e','s','i','g','n','s'}
};

//Product string descriptor
ROM struct{uint8_t bLength; uint8_t bDscType; uint16_t string[12];} sd002 = {
    sizeof(sd002), USB_DESCRIPTOR_STRING,
    {'O','L','I',' ','K','e','y','b','o','a','r','d'}
};

//Array of configuration descriptors
ROM uint8_t *ROM USB_CD_Ptr[] = {
    (ROM uint8_t *ROM)&configDescriptor1
};

//Array of string descriptors
ROM uint8_t *ROM USB_SD_Ptr[] = {
    (ROM uint8_t *ROM)&sd000,
    (ROM uint8_t *ROM)&sd001,
    (ROM uint8_t *ROM)&sd002
};

/** EOF usb_descriptors.c ***************************************************/

#endif