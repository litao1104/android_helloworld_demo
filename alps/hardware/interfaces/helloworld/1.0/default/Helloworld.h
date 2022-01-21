// FIXME: your file license if you have one

#pragma once

#include <android/hardware/helloworld/1.0/IHelloworld.h>
#include <hardware/helloworld.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#define UNUSED(x) (void)(x)

namespace android {
namespace hardware {
namespace helloworld {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct Helloworld : public IHelloworld {
    Helloworld(helloworld_device_t *device);
    // Methods from ::android::hardware::helloworld::V1_0::IHelloworld follow.
    Return<int32_t> init() override;
    Return<int32_t> enable(uint32_t onoff) override;
    Return<int32_t> read(uint32_t cmd) override;
    Return<int32_t> write(uint32_t cmd) override;

private:
    helloworld_device_t *mDevice;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
 extern "C" IHelloworld* HIDL_FETCH_IHelloworld(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace helloworld
}  // namespace hardware
}  // namespace android
