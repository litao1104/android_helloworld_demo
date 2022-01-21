// FIXME: your file license if you have one

#include <log/log.h>

#include <hardware/hardware.h>
#include <hardware/helloworld.h>

#include "Helloworld.h"

namespace android {
namespace hardware {
namespace helloworld {
namespace V1_0 {
namespace implementation {

Helloworld::Helloworld(helloworld_device_t *device) {
    ALOGI("Helloworld::Helloworld\n");
    mDevice = device;
}

// Methods from ::android::hardware::helloworld::V1_0::IHelloworld follow.
Return<int32_t> Helloworld::init() {
    // TODO implement
    ALOGI("Helloworld::init\n");

    int32_t ret;

    ret = mDevice->init(mDevice);
    if (ret != 0) {
        ALOGE("init failed, ret = %d", ret);
    }

    return ret;
}

Return<int32_t> Helloworld::enable(uint32_t onoff) {
    // TODO implement
    UNUSED(onoff);

    ALOGI("Helloworld::enable onoff = %d\n", onoff);

    int32_t ret;

    ret = mDevice->enable(mDevice, onoff);
    if (ret != 0) {
        ALOGE("enable failed, ret = %d", ret);
    }

    return ret;
}

Return<int32_t> Helloworld::read(uint32_t cmd) {
    // TODO implement
    UNUSED(cmd);

    ALOGI("Helloworld::read cmd = %d\n", cmd);

    int32_t ret;

    ret = mDevice->read(mDevice, cmd);
    if (ret != 0) {
        ALOGE("read failed, ret = %d", ret);
    }

    return ret;
}

Return<int32_t> Helloworld::write(uint32_t cmd) {
    // TODO implement
    UNUSED(cmd);

    ALOGI("Helloworld::write cmd = %d\n", cmd);

    int32_t ret;

    ret = mDevice->write(mDevice, cmd);
    if (ret != 0) {
        ALOGE("write failed, ret = %d", ret);
    }

    return ret;
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

IHelloworld* HIDL_FETCH_IHelloworld(const char* /* name */) {
    helloworld_device_t *dev;
    const hw_module_t *hw_module = NULL;

    ALOGI("HIDL_FETCH_IHelloworld\n");

    int ret = hw_get_module(HELLOWORLD_HARDWARE_MODULE_ID, &hw_module);
    if (ret != 0) {
        ALOGE("hw_get_module %s failed: %d", HELLOWORLD_HARDWARE_MODULE_ID, ret);
        return nullptr;
    }
    ret = hw_module->methods->open(hw_module, HELLOWORLD_DEVICE_ID, (hw_device_t **) &dev);
    if (ret < 0) {
        ALOGE("Can't open helloworld device, error: %d", ret);
        return nullptr;
    }

    return new Helloworld(dev);
}
//
}  // namespace implementation
}  // namespace V1_0
}  // namespace helloworld
}  // namespace hardware
}  // namespace android
