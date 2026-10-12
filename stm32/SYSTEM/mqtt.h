#ifndef __MQTT_H__
#define __MQTT_H__

#include <stdint.h>

/*
 * MQTT 客户端 (MQTT 3.1.1, QoS 0)
 * 基于 ESP8266 TCP (wifi_test.c 提供 TCP 通道)
 *
 * 用于 OneNet / 公共 MQTT 服务器数据上报
 */

/* 构造 CONNECT 报文, 返回报文长度 */
uint16_t mqtt_build_connect(uint8_t *buf, const char *client_id,
                            const char *user, const char *pass);

/* 构造 PUBLISH 报文 (QoS 0), 返回报文长度 */
uint16_t mqtt_build_publish(uint8_t *buf, const char *topic, const char *msg);

/* 连接 MQTT 服务器 (TCP + CONNECT) */
void mqtt_connect_broker(const char *ip, uint16_t port, const char *client_id,
                         const char *user, const char *pass);

/* 发布消息 */
void mqtt_publish_msg(const char *topic, const char *msg);

/* 订阅主题 */
void mqtt_subscribe(const char *topic);

/* 检查是否有下发的命令 (非阻塞), 返回1=有命令(存入cmd), 0=无 */
int mqtt_check_command(char *cmd, uint16_t maxlen);

/* 累积收到的下行数据（供 wifi_test.c 把读到的字节拼起来，解决 +IPD 分片） */
void mqtt_rx_accumulate(const uint8_t *data, uint16_t len);

#endif
