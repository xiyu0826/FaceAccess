#include "mqtt.h"
#include "wifi_test.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t mqtt_buf[256];

/* ---- 构造 CONNECT 报文 -------------------------------------- */
uint16_t mqtt_build_connect(uint8_t *buf, const char *client_id,
                            const char *user, const char *pass)
{
    uint16_t i = 0;
    uint16_t payload_len;
    uint8_t  flags = 0x02;          /* 默认: 干净会话 */
    int use_auth = (user && pass && strlen(user) > 0);

    if (use_auth)
        flags = 0xC2;               /* 干净会话 + 用户名 + 密码 */

    payload_len = 2 + strlen(client_id);
    if (use_auth)
        payload_len += 2 + strlen(user) + 2 + strlen(pass);

    /* 固定报头 */
    buf[i++] = 0x10;                /* CONNECT */
    buf[i++] = (uint8_t)(10 + payload_len);   /* 剩余长度 (<128, 单字节) */

    /* 可变报头 */
    buf[i++] = 0x00; buf[i++] = 0x04;         /* 协议名长度 */
    buf[i++] = 'M';  buf[i++] = 'Q';
    buf[i++] = 'T';  buf[i++] = 'T';
    buf[i++] = 0x04;                /* 协议级别 3.1.1 */
    buf[i++] = flags;               /* 连接标志 */
    buf[i++] = 0x00; buf[i++] = 0x3C;         /* 保活 60s */

    /* 载荷: Client ID */
    buf[i++] = 0x00; buf[i++] = (uint8_t)strlen(client_id);
    memcpy(&buf[i], client_id, strlen(client_id));
    i += strlen(client_id);

    if (use_auth)
    {
        buf[i++] = 0x00; buf[i++] = (uint8_t)strlen(user);
        memcpy(&buf[i], user, strlen(user));
        i += strlen(user);

        buf[i++] = 0x00; buf[i++] = (uint8_t)strlen(pass);
        memcpy(&buf[i], pass, strlen(pass));
        i += strlen(pass);
    }

    return i;
}

/* ---- 构造 PUBLISH 报文 (QoS 0) ------------------------------- */
uint16_t mqtt_build_publish(uint8_t *buf, const char *topic, const char *msg)
{
    uint16_t i = 0;
    uint16_t remaining = 2 + strlen(topic) + strlen(msg);

    buf[i++] = 0x30;                /* PUBLISH, QoS 0 */

    /* MQTT 剩余长度变长编码（>127 时多字节，防止加字段后整包错乱） */
    do {
        uint8_t d = remaining % 128;
        remaining /= 128;
        if (remaining > 0) d |= 0x80;
        buf[i++] = d;
    } while (remaining > 0);

    buf[i++] = 0x00; buf[i++] = (uint8_t)strlen(topic);
    memcpy(&buf[i], topic, strlen(topic));
    i += strlen(topic);

    memcpy(&buf[i], msg, strlen(msg));
    i += strlen(msg);

    return i;
}

/* ---- 连接 MQTT 服务器 --------------------------------------- */
void mqtt_connect_broker(const char *ip, uint16_t port, const char *client_id,
                         const char *user, const char *pass)
{
    uint16_t len;

    printf("\r\n[MQTT] Connecting to %s:%d ...\r\n", ip, port);
    wifi_tcp_connect(ip, port);

    len = mqtt_build_connect(mqtt_buf, client_id, user, pass);
    printf("[MQTT] Sending CONNECT (%d bytes)\r\n", len);
    wifi_tcp_send(mqtt_buf, len);
}

/* ---- 发布消息 ----------------------------------------------- */
void mqtt_publish_msg(const char *topic, const char *msg)
{
    uint16_t len = mqtt_build_publish(mqtt_buf, topic, msg);
    printf("[MQTT] Publish to [%s]: %s (%d bytes)\r\n", topic, msg, len);
    wifi_tcp_send(mqtt_buf, len);
}

/* ---- 构造 SUBSCRIBE 报文 ------------------------------------ */
static uint16_t mqtt_build_subscribe(uint8_t *buf, const char *topic)
{
    uint16_t i = 0;
    uint16_t remaining = 2 + 2 + strlen(topic) + 1;  /* 报文ID + topic长度 + topic + QoS */

    buf[i++] = 0x82;              /* SUBSCRIBE */
    buf[i++] = (uint8_t)remaining;
    buf[i++] = 0x00; buf[i++] = 0x01;         /* 报文ID */
    buf[i++] = 0x00; buf[i++] = (uint8_t)strlen(topic);  /* topic长度 */
    memcpy(&buf[i], topic, strlen(topic));
    i += strlen(topic);
    buf[i++] = 0x00;              /* QoS 0 */

    return i;
}

/* ---- 订阅主题 ----------------------------------------------- */
void mqtt_subscribe(const char *topic)
{
    uint16_t len = mqtt_build_subscribe(mqtt_buf, topic);
    printf("[MQTT] Subscribe to [%s] (%d bytes)\r\n", topic, len);
    wifi_tcp_send(mqtt_buf, len);
}

/* ---- 检查下发的命令 (非阻塞) -------------------------------- */
/* ---- 下行 +IPD 跨轮询累积缓冲（解决 +IPD 被分片导致 strstr 找不到） ---- */
static uint8_t  mqtt_rx_acc[256];
static uint16_t mqtt_rx_acc_len = 0;

/* 累积收到的下行数据（把多次 wifi_poll/wifi_recv 读到的字节拼起来） */
void mqtt_rx_accumulate(const uint8_t *data, uint16_t len)
{
    if (mqtt_rx_acc_len + len < sizeof(mqtt_rx_acc))
    {
        memcpy(mqtt_rx_acc + mqtt_rx_acc_len, data, len);
        mqtt_rx_acc_len += len;
        mqtt_rx_acc[mqtt_rx_acc_len] = '\0';
    }
    else
    {
        mqtt_rx_acc_len = 0;   /* 溢出保护 */
    }
}

/* 从累积缓冲提取一条完整 +IPD 数据，返回数据长度；未到齐返回 0 */
static uint16_t mqtt_extract_ipd(uint8_t *ipd, uint16_t maxlen)
{
    char *p = strstr((char *)mqtt_rx_acc, "+IPD,");
    int len, total;
    if (!p) return 0;
    p += 5;
    len = atoi(p);
    p = strchr(p, ':');
    if (!p) return 0;
    p++;
    total = (int)(p - (char *)mqtt_rx_acc) + len;   /* 已消费的字节数 */
    if (total > mqtt_rx_acc_len) return 0;          /* 数据还没到齐，等下一轮 */
    if (len <= 0 || len >= (int)maxlen) return 0;

    memcpy(ipd, p, len);
    memmove(mqtt_rx_acc, mqtt_rx_acc + total, mqtt_rx_acc_len - total);
    mqtt_rx_acc_len -= total;
    mqtt_rx_acc[mqtt_rx_acc_len] = '\0';
    return (uint16_t)len;
}

int mqtt_check_command(char *cmd, uint16_t maxlen)
{
    uint8_t rx[64];
    uint8_t ipd[128];
    uint16_t np, n;
    uint16_t topic_len;
    int payload_len;

    /* 累积新到的数据 */
    np = wifi_poll(rx, sizeof(rx));
    if (np > 0)
        mqtt_rx_accumulate(rx, np);

    /* 提取完整 +IPD */
    n = mqtt_extract_ipd(ipd, sizeof(ipd));
    if (n == 0)
    {
        /* 没有完整 +IPD：若连 "+IPD," 都没有，说明是 AT 响应杂数据，清空 */
        if (strstr((char *)mqtt_rx_acc, "+IPD,") == NULL)
            mqtt_rx_acc_len = 0;
        return 0;
    }

    /* ipd 是 MQTT PUBLISH 报文: [0x30][剩余长度][topic长度2字节][topic][payload] */
    if (n < 5) return 0;

    topic_len = ((uint16_t)ipd[2] << 8) | ipd[3];
    payload_len = n - 4 - topic_len;

    if (payload_len <= 0 || payload_len >= maxlen) return 0;

    memcpy(cmd, ipd + 4 + topic_len, payload_len);
    cmd[payload_len] = '\0';

    return 1;
}
