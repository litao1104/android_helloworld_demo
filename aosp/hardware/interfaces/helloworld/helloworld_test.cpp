# include <android/hardware/helloworld/1.0/IHelloworld.h>
# include <hidl/Status.h>
# include <hidl/LegacySupport.h>
# include <utils/misc.h>
# include <hidl/HidlSupport.h>
# include <stdio.h>

using android::hardware::helloworld::V1_0::IHelloworld;
using android::sp;
using android::hardware::hidl_string;

int main(int argc, char **argv)
{
    int fun;
    int cmd;

    for (int i = 0; i < argc; ++i) {
        printf("Argument %d is %s\n", i, argv[i]);
    }

    sscanf(argv[1], "%d", &fun);
    sscanf(argv[2], "%d", &cmd);

    android::sp<IHelloworld> service = IHelloworld::getService();
    if(service == nullptr) {
        printf("Failed to get IHelloworld service\n");
        return -1;
    }

    switch (fun) {
        case 1:  //init
            service->init();
            break;

        case 2:  //enable
            service->enable(cmd);
            break;

        case 3:  //read
            service->read(cmd);
            break;

        case 4:  //write
            service->write(cmd);
            break;

        default:
            printf("error: unknown func\n");
            break;
    }

    return 0;
}
