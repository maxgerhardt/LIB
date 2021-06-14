#include "usb_descriptors.h"

const alignas(4) usb_device_descriptor_t usb_device_descriptor =
    {
        .bLength = sizeof(usb_device_descriptor_t), // 0x12
        .bDescriptorType = USB_DEVICE_DESCRIPTOR,   // 0x01
        .bcdUSB = 0x0100,                           //
        .bDeviceClass = 0xFF,                       // 0 = Defined in interface or Vendor Specific
        .bDeviceSubClass = 0x00,                    // 0 = Defined in interface
        .bDeviceProtocol = 0x00,                    // 0 = Defined in interface
        .bMaxPacketSize = 64,                       //
        .idVendor = 0x6666,                         //
        .idProduct = 0x6666,                        //
        .bcdDevice = 0x0100,                        // 
        .iManufacturer = USB_STR_MANUFACTURER,      // 1
        .iProduct = USB_STR_PRODUCT,                // 2
        .iSerialNumber = USB_STR_SERIAL_NUMBER,     // 3
        .bNumConfigurations = 1,                    // 1
};

const alignas(4) uint8_t usb_hid_report_descriptor[28] =
    {
        0x05, 0x01,       // Usage Page (Generic Desktop Ctrls)
        0x09, 0x00,       // Usage (Undefined)
        0xa1, 0x01,       // Collection (Application)
        0x15, 0x00,       //   Logical Minimum (0)
        0x26, 0xff, 0x00, //   Logical Maximum (255)
        0x75, 0x08,       //   Report Size (8)
        0x95, 0x40,       //   Report Count (64)
        0x09, 0x00,       //   Usage (Undefined)
        0x81, 0x82,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
        0x75, 0x08,       //   Report Size (8)
        0x95, 0x40,       //   Report Count (64)
        0x09, 0x00,       //   Usage (Undefined)
        0x91, 0x82,       //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)
        0xc0,             // End Collection
};

const alignas(4) usb_configuration_hierarchy_t usb_configuration_hierarchy =
    {
        .configuration = /* Configuration 1 descriptor */
        {
            .bLength = sizeof(usb_configuration_descriptor_t),     // 9
            .bDescriptorType = USB_CONFIGURATION_DESCRIPTOR,       // 2
            .wTotalLength = sizeof(usb_configuration_hierarchy_t), // 64 
            .bNumInterfaces = USE_HID + USE_MSC,                   // 2 
            .bConfigurationValue = 1,                              // 1
            .iConfiguration = USB_STR_CONFIGURATION,               // 4
            .bmAttributes = 0x80,                                  // bus-powered
            .bMaxPower = 200,                                      // 400 mA 0xC8
        },

#if USE_HID
        /********** Interface #0 : HID  **********/
        .hid_interface =
            {
                .bLength = sizeof(usb_interface_descriptor_t), // 9
                .bDescriptorType = USB_INTERFACE_DESCRIPTOR,   // 4
                .bInterfaceNumber = 0,                         // 0 interface number
                .bAlternateSetting = 0,                        // 0
                .bNumEndpoints = 2,                            // 2
                .bInterfaceClass = 0x03,                       // 3 HID
                .bInterfaceSubClass = 0x00,                    // 0
                .bInterfaceProtocol = 0x00,                    // 0
                .iInterface = USB_STR_INTERFACE_HID,           // 5
            },
        .hid_descriptor =
            {
                .bLength = sizeof(usb_hid_descriptor_t),                // 9
                .bDescriptorType = USB_HID_DESCRIPTOR,                  // x21
                .bcdHID = 0x0111,                                       //
                .bCountryCode = 0,                                      //
                .bNumDescriptors = 1,                                   //
                .bDescriptorType1 = USB_HID_REPORT_DESCRIPTOR,          // x22
                .wDescriptorLength = sizeof(usb_hid_report_descriptor), //
            },
        .hid_in =
            {
                .bLength = sizeof(usb_endpoint_descriptor_t),    // 7
                .bDescriptorType = USB_ENDPOINT_DESCRIPTOR,      // 5
                .bEndpointAddress = USB_IN_ENDPOINT | EP_HID_IN, // 0x81
                .bmAttributes = USB_INTERRUPT_ENDPOINT,          // 3
                .wMaxPacketSize = 64,                            // 64
                .bInterval = 1,                                  // 1
            },
        .hid_out =
            {
                .bLength = sizeof(usb_endpoint_descriptor_t),      // 7
                .bDescriptorType = USB_ENDPOINT_DESCRIPTOR,        // 5
                .bEndpointAddress = USB_OUT_ENDPOINT | EP_HID_OUT, // 0x02
                .bmAttributes = USB_INTERRUPT_ENDPOINT,            // 3
                .wMaxPacketSize = 64,                              // 64
                .bInterval = 1,                                    // 1
            },
#endif

#if USE_MSC
        /********** Interface #1 : Mass Storage Device **********/
        .msc_interface = {
            .bLength = sizeof(usb_interface_descriptor_t), // 9
            .bDescriptorType = USB_INTERFACE_DESCRIPTOR,   // 4
            .bInterfaceNumber = USE_HID,                   // 1 interface number
            .bAlternateSetting = 0,                        // 0
            .bNumEndpoints = 2,                            // 2 IN+OUT
            .bInterfaceClass = 0x08,                       // 8 Mass Storage
            .bInterfaceSubClass = 0x06,                    // 6 SCSI Transparent Command Set
            .bInterfaceProtocol = 80,                      // Bulk-Only Transport
            .iInterface = USB_STR_INTERFACE_MSC,           //
        },
        .msc_in = {
            .bLength = sizeof(usb_endpoint_descriptor_t),    // 7
            .bDescriptorType = USB_ENDPOINT_DESCRIPTOR,      // 5
            .bEndpointAddress = USB_IN_ENDPOINT | EP_MSC_IN, // 0x83
            .bmAttributes = USB_BULK_ENDPOINT,               // 2 transfer type - bulk
            .wMaxPacketSize = 64,                            // 64
            .bInterval = 0,                                  // 0
        },
        .msc_out = {
            .bLength = sizeof(usb_endpoint_descriptor_t),      // 7
            .bDescriptorType = USB_ENDPOINT_DESCRIPTOR,        // 5
            .bEndpointAddress = USB_OUT_ENDPOINT | EP_MSC_OUT, // 0x04
            .bmAttributes = USB_BULK_ENDPOINT,                 // 2 transfer type - bulk
            .wMaxPacketSize = 64,                              // 64
            .bInterval = 0,                                    // 0
        },
#endif

};

const alignas(4) usb_string_descriptor_zero_t usb_string_descriptor_zero =
    {
        .bLength = sizeof(usb_string_descriptor_zero_t),
        .bDescriptorType = USB_STRING_DESCRIPTOR,
        .wLANGID = 0x0409, // English (United States)
};

const char *const usb_strings[] =
    {
        [USB_STR_MANUFACTURER] = "WizIO",
        [USB_STR_PRODUCT] = "WizIO Product",
        [USB_STR_SERIAL_NUMBER] = "12345678",
        [USB_STR_CONFIGURATION] = "WizIO Configuration",
        [USB_STR_INTERFACE_HID] = "WizIO HID",
        [USB_STR_INTERFACE_MSC] = "WizIO MSC",
};