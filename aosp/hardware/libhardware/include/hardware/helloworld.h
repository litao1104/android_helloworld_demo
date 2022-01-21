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

#ifndef _HARDWARE_HELLOWORLD_H
#define _HARDWARE_HELLOWORLD_H

#include <hardware/hardware.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UNUSED(x) (void)(x)

#define HELLOWORLD_MODULE_API_VERSION_1_0 HARDWARE_MODULE_API_VERSION(1, 0)

/* define module ID */
#define HELLOWORLD_HARDWARE_MODULE_ID "helloworld"
#define HELLOWORLD_DEVICE_ID          "helloworld_dev"

typedef struct helloworld_device {
    struct hw_device_t common;

    int (*init)(struct helloworld_device* dev);
    int (*enable)(struct helloworld_device* dev, uint32_t onoff);
    int (*read)(struct helloworld_device* dev, uint32_t cmd);
    int (*write)(struct helloworld_device* dev, uint32_t cmd);
} helloworld_device_t;

#ifdef __cplusplus
}
#endif

#endif  // _HARDWARE_HELLOWORLD_H
