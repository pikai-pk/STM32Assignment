#ifndef __MQTT_CONFIG_H
#define __MQTT_CONFIG_H

/*
 * MQTT/Wi-Fi 配置兼容入口
 *
 * 真实配置已经集中到 app_config.h 文件顶部。
 * 保留这个文件是为了兼容旧驱动中 include "mqtt_config.h" 的写法。
 */

#include "app_config.h"

#define PRODUCT_KEY     ACCESS_TOKEN
#define DEVICE_SECRET   PROJECT_KEY

#endif
