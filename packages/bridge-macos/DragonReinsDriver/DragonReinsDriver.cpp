//
//  DragonReinsDriver.cpp
//  DragonReinsDriver
//
//  Created by Jamey McElveen on 3/20/26.
//

#include <os/log.h>
#include <DriverKit/IOLib.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>
#include <USBDriverKit/IOUSBHostInterface.h>

#include "DragonReinsDriver.h"

#define Log(fmt, ...) os_log(OS_LOG_DEFAULT, "DragonReinsDriver - " fmt "\n", ##__VA_ARGS__)

struct DragonReinsDriver_IVars {
    IOUSBHostInterface* usbInterface = nullptr;
};


// MARK: - Lifecycle

bool DragonReinsDriver::init(void)
{
    Log("init()");

    if (!super::init()) {
        Log("init() - super::init failed.");
        return false;
    }

    ivars = IONewZero(DragonReinsDriver_IVars, 1);
    if (ivars == nullptr) {
        Log("init() - Failed to allocate ivars.");
        return false;
    }

    Log("init() - Finished.");
    return true;
}

kern_return_t DragonReinsDriver::Start_Impl(IOService* provider)
{
    kern_return_t ret = kIOReturnSuccess;

    ret = Start(provider, SUPERDISPATCH);
    if (ret != kIOReturnSuccess) {
        Log("Start() - super::Start failed: 0x%08x.", ret);
        goto Exit;
    }

    Log("Start() - Matched Redragon K681 (VID 9610 / PID 268), Interface 4.");

    ivars->usbInterface = OSDynamicCast(IOUSBHostInterface, provider);
    if (ivars->usbInterface == nullptr) {
        Log("Start() - Provider is not an IOUSBHostInterface.");
        ret = kIOReturnNoDevice;
        goto Exit;
    }
    ivars->usbInterface->retain();

    ret = ivars->usbInterface->Open(this, 0, NULL);
    if (ret != kIOReturnSuccess) {
        Log("Start() - Failed to open USB interface: 0x%08x.", ret);
        goto Exit;
    }

    {
        // Build 64-byte Clemson Orange packet (#F56600)
        // Header bytes at indices 0-5; RGB at 8, 9, 10; terminator at 63.
        uint8_t packet[64] = {0};
        packet[0]  = 0x04;
        packet[1]  = 0x0c;
        packet[2]  = 0x02;
        packet[3]  = 0x06;
        packet[4]  = 0x01;
        packet[5]  = 0x05;
        packet[8]  = 0xF5;  // R
        packet[9]  = 0x66;  // G
        packet[10] = 0x00;  // B
        packet[63] = 0xFF;  // Terminator

        IOBufferMemoryDescriptor* memDesc = nullptr;
        ret = IOBufferMemoryDescriptor::Create(kIOMemoryDirectionOut, 64, 0, &memDesc);
        if (ret != kIOReturnSuccess || memDesc == nullptr) {
            Log("Start() - IOBufferMemoryDescriptor::Create failed: 0x%08x.", ret);
            ivars->usbInterface->Close(this, 0);
            goto Exit;
        }

        IOAddressSegment range = {};
        kern_return_t mapRet = memDesc->GetAddressRange(&range);
        if (mapRet == kIOReturnSuccess) {
            memcpy((void*)range.address, packet, 64);

            // USB SET_REPORT (HID Class request)
            // bmRequestType = 0x21: Host→Device, Class, Interface
            // bRequest      = 0x09: SET_REPORT
            // wValue        = 0x0304: Report type 0x03 (Feature), Report ID 0x04
            // wIndex        = 4: Interface 4
            // wLength       = 64 bytes
            // DriverKit DeviceRequest takes 8 arguments (flat-field form).
            uint16_t bytesTransferred = 0;
            kern_return_t reqRet = ivars->usbInterface->DeviceRequest(
                (uint8_t)0x21,     // bmRequestType
                (uint8_t)0x09,     // bRequest
                (uint16_t)0x0304,  // wValue
                (uint16_t)4,       // wIndex
                (uint16_t)64,      // wLength
                memDesc,           // dataBuffer
                &bytesTransferred, // bytesTransferred
                (uint32_t)5000     // completionTimeout (ms)
            );

            if (reqRet == kIOReturnSuccess) {
                Log("Start() - Clemson Orange sent successfully! (%u bytes transferred)", bytesTransferred);
            } else {
                Log("Start() - DeviceRequest failed: 0x%08x. Driver will still register.", reqRet);
            }
        } else {
            Log("Start() - GetAddressRange failed: 0x%08x.", mapRet);
        }

        OSSafeReleaseNULL(memDesc);
    }

    ret = RegisterService();
    if (ret != kIOReturnSuccess) {
        Log("Start() - RegisterService failed: 0x%08x.", ret);
        goto Exit;
    }

    Log("Start() - Finished.");
    ret = kIOReturnSuccess;

Exit:
    return ret;
}

kern_return_t DragonReinsDriver::Stop_Impl(IOService* provider)
{
    kern_return_t ret = kIOReturnSuccess;

    Log("Stop()");

    if (ivars->usbInterface != nullptr) {
        ivars->usbInterface->Close(this, 0);
    }

    ret = Stop(provider, SUPERDISPATCH);
    if (ret != kIOReturnSuccess) {
        Log("Stop() - super::Stop failed: 0x%08x.", ret);
    }

    Log("Stop() - Finished.");
    return ret;
}

void DragonReinsDriver::free(void)
{
    Log("free()");

    OSSafeReleaseNULL(ivars->usbInterface);
    IOSafeDeleteNULL(ivars, DragonReinsDriver_IVars, 1);

    super::free();
}
