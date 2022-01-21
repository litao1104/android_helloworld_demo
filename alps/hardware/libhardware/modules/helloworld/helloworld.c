/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hardware/helloworld.h>
#include <hardware/hardware.h>

#include <errno.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <log/log.h>

static int helloworld_close(hw_device_t *dev)
{
    free(dev);
    return 0;
}

static int helloworld_init(struct helloworld_device *dev __unused)
{
    ALOGI("helloworld_init\n");

    return 0;
}

static int helloworld_enable(struct helloworld_device *dev __unused,  uint32_t onoff)
{
    ALOGI("helloworld_enable, onoff = %d\n", onoff);

    return 0;
}

static int helloworld_read(struct helloworld_device *dev __unused,  uint32_t cmd)
{
    ALOGI("helloworld_read, cmd = %d\n", cmd);

    return 0;
}

static int helloworld_write(struct helloworld_device *dev __unused,  uint32_t cmd)
{
    ALOGI("helloworld_write, cmd = %d\n", cmd);

    return 0;
}

static int helloworld_open(const hw_module_t* module, const char* name,
        hw_device_t** device)
{
    ALOGI("helloworld_open start\n");

    if (strcmp(name, HELLOWORLD_DEVICE_ID) != 0) {
        return -EINVAL;
    }
    if (device == NULL) {
        ALOGE("NULL device on open");
        return -EINVAL;
    }

    helloworld_device_t *dev = malloc(sizeof(helloworld_device_t));
    memset(dev, 0, sizeof(helloworld_device_t));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*) module;
    dev->common.close = helloworld_close;

    dev->init = helloworld_init;
    dev->enable = helloworld_enable;
    dev->read = helloworld_read;
    dev->write = helloworld_write;

    *device = (hw_device_t*) dev;

    ALOGI("helloworld_open end\n");

    return 0;
}

static struct hw_module_methods_t helloworld_module_methods = {
    .open = helloworld_open,
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag                = HARDWARE_MODULE_TAG,
    .module_api_version = HELLOWORLD_MODULE_API_VERSION_1_0,
    .hal_api_version    = HARDWARE_HAL_API_VERSION,
    .id                 = HELLOWORLD_HARDWARE_MODULE_ID,
    .name               = "Helloworld Demo HAL",
    .author             = "The Android Open Source Project",
    .methods            = &helloworld_module_methods,
};
