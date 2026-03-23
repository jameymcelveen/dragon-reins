//
//  DragonReinsDriver.cpp
//  DragonReinsDriver
//
//  Created by Jamey McElveen on 3/20/26.
//

#include <os/log.h>
#include <DriverKit/IOUserServer.h>
#include <USBDriverKit/IOUSBHostInterface.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>
#include <DriverKit/IOLib.h>

#include "DragonReinsDriver.h"

kern_return_t
IMPL(DragonReinsDriver, Start)
{
    kern_return_t ret = Start(provider, SUPERDISPATCH);
    if (ret != kIOReturnSuccess) return ret;
    
    os_log(OS_LOG_DEFAULT, "🐅 DragonReins: Driver Matched!");
    
    // 1. Cast the provider
    auto interface = OSDynamicCast(IOUSBHostInterface, provider);
    if (!interface) return kIOReturnNoDevice;

    // 2. Open the interface session
    ret = interface->Open(this, 0, NULL);
    if (ret != kIOReturnSuccess) return ret;

    // 3. Prepare Clemson Orange Packet (#F56600)
    uint8_t orangeBytes[64] = {0};
    uint8_t header[] = {0x04, 0x0c, 0x02, 0x06, 0x01, 0x05};
    memcpy(orangeBytes, header, 6);
    orangeBytes[8]  = 0xF5;
    orangeBytes[9]  = 0x66;
    orangeBytes[10] = 0x00;
    orangeBytes[63] = 0xFF;

    // 4. Create a Memory Descriptor for the USB transfer
    IOBufferMemoryDescriptor* memDesc = nullptr;
    ret = IOBufferMemoryDescriptor::Create(kIOMemoryDirectionOut, 64, 0, &memDesc);

    if (ret == kIOReturnSuccess && memDesc) {
        // DRIVERKIT WAY: Get the memory address to write into
        IOAddressSegment range;
        ret = memDesc->GetAddressRange(&range);
        
        if (ret == kIOReturnSuccess) {
            // Copy the orangeBytes into the memory descriptor's address
            memcpy((void*)range.address, orangeBytes, 64);

            // 6. Blast the "Set Report" request
            // NOTE: DriverKit DeviceRequest signature often needs 'this' as the first arg
            // depending on your SDK version. Try this signature:
            uint16_t bytesTransferred = 0;
            ret = interface->DeviceRequest(0x21, 0x09, 0x0304, 4, 64, memDesc, &bytesTransferred, 5000);
            
            if (ret == kIOReturnSuccess) {
                IOLog("✅ Clemson Orange Sent! (%u bytes)", bytesTransferred);
            } else {
                IOLog("❌ Failed to send Orange: 0x%08x", ret);
            }
        }
        
        OSSafeReleaseNULL(memDesc);
    }
    
    return kIOReturnSuccess;
}
