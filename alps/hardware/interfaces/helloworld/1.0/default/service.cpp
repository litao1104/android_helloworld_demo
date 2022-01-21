# define LOG_TAG "android.hardware.helloworld@1.0-service"

# include <android/hardware/helloworld/1.0/IHelloworld.h>

# include <hidl/LegacySupport.h>

#include <log/log.h>

using android::hardware::helloworld::V1_0::IHelloworld;
using android::hardware::defaultPassthroughServiceImplementation;

int main() {
    ALOGI("before defaultPassthroughServiceImplementation<IHelloworld>()\n");
    return defaultPassthroughServiceImplementation<IHelloworld>();
}
