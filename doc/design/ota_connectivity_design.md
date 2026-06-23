# UniProton OTA and Connectivity Design

本文说明 UniProton `iot-ota-test` 在 sd3403 上的 OTA、MQTT、LwM2M、NB-IoT 特性设计、组件组成、socket 网络关系、宏控接入、测试逻辑和 host 环境搭建。

## 1. 目标

`iot-ota-test` 是 sd3403 单板统一 IoT/OTA 验证入口。它不按旧场景目标名拆分固件，而是由 feature 宏组合决定同一固件内编译和运行哪些测试。

测试目标按 UniProton OTA/connectivity 组件边界设计：

- OTA/SOTA 覆盖 package 解析、分块下载、更新触发、flash 写入回调，以及 `sota_init`、`sota_parse`、`sota_process` 正负向用例。
- LwM2M 使用真实 `liblwm2m` core、CoAP 层和对象模型，测试 Security、Server、Device、Firmware Update Object。
- MQTT 使用真实 `MQTTClient-C/MQTTPacket` 协议栈，经 TCP socket 连接 broker 或 host helper，测试 profile 上报、命令响应和 OTA manifest/chunk 传输。
- NB-IoT 使用公开 `los_nb_api`，当前 sd3403 无真实模组/基站时以 direct-link bearer 验证 API 边界，后续真实硬件环境替换 bearer。
- host helper 只是缺少完整 host/cloud/NB-IoT 环境时的 fallback 对端；它必须校验协议字段和响应码，不能用裸字符串或私有 channel 冒充 MQTT/LwM2M 协议栈。

## 2. 总体架构

```text
sd3403 app
  iot-ota-test
    common/
      SOTA/package test helper and fake flash adapter
    ota/
      local SOTA/package flow and negative tests
    lwm2m/
      LwM2M Device Object register/read/write/execute test client
    mqtt/
      MQTT profile bind/report/command/response test client
    nb_iot/
      NB-IoT API and bearer boundary tests
    lwm2m_ota/
      LwM2M Firmware Update Object carries OTA package and trigger
    mqtt_ota/
      MQTT manifest/chunk topics carry OTA package and trigger
```

正式可复用组件在 `src/component` 下：

```text
src/component/ota
  SOTA state machine, OTA package parser/writer, checksum abstraction, upgrade flag and CRC utility.

src/component/connectivity/lwm2m
  liblwm2m core, CoAP message layer and UniProton UDP/session port.

src/component/connectivity/mqtt
  MQTTClient-C, MQTTPacket and UniProton TCP/timer adapter.

src/component/connectivity/nb_iot
  los_nb_api public API, NB-IoT bearer abstraction and sd3403 direct-link bearer.
```

测试/示例驱动在 `testsuites/iot-ota-test` 下，不放入正式组件目录。

## 3. Socket 网络关系

MQTT、LwM2M、NB-IoT 和 OTA 的关系必须保持清晰分层。

MQTT 路径：

```text
MQTT demo / MQTT OTA test
  -> MQTTClient-C API
    -> MQTTPacket encode/decode
      -> UniProton MQTT Network/Timer adapter
        -> TCP socket
          -> Mosquitto, cloud broker or builtin MQTT fallback helper
```

LwM2M 路径：

```text
LwM2M demo / LwM2M OTA test
  -> LwM2M object model and client registration state machine
    -> CoAP message layer
      -> UDP socket
        -> Leshan, standard LwM2M server or builtin CoAP/LwM2M fallback helper
```

NB-IoT 路径：

```text
NB-IoT API test
  -> los_nb_api
    -> NB-IoT bearer implementation
      -> real modem/base-station bearer, or sd3403 direct-link fallback bearer
        -> host NB-IoT proxy service when fallback is used
```

OTA 本身不是网络协议。OTA package 可以由本地 SOTA 测试、LwM2M Firmware Update Object、MQTT topics 或未来 NB-IoT/cloud 通道承载，但 package 解析、写 flash、升级状态机仍归属 `src/component/ota`。

禁止事项：

- 不允许用私有 channel、裸 TCP/UDP 字符串协议或 NB-IoT frame 伪装 MQTT/LwM2M 协议验证。
- 不允许把 socket 收发能力当作 MQTT、CoAP 或 LwM2M 协议语义。
- 无网卡驱动时只能替换 socket 以下或 bearer 层，测试代码仍应面对真实协议栈 API。

## 4. 宏控和构建接入

feature 宏按 UniProton Kconfig/defconfig 风格配置：

```text
CONFIG_OS_SUPPORT_OTA=y
CONFIG_OS_SUPPORT_LWM2M=y
CONFIG_OS_SUPPORT_MQTT=y
CONFIG_OS_SUPPORT_NB_IOT=y
```

`src/CMakeLists.txt` 接入规则：

- `CONFIG_OS_SUPPORT_OTA=y` 时编译 `src/component/ota`。
- `CONFIG_OS_SUPPORT_LWM2M=y`、`CONFIG_OS_SUPPORT_MQTT=y` 或 `CONFIG_OS_SUPPORT_NB_IOT=y` 任一启用时编译 `src/component/connectivity`。

`src/component/ota/CMakeLists.txt` 编译 OTA 组件：

- `sota/sota.c`
- `sota/sota_hal.c`
- `package/package.c`
- `package/package_writer.c`
- `package/package_head.c`
- `package/package_checksum.c`
- `flag_operate/flag_manager.c`
- `flag_operate/upgrade_flag.c`
- `utility/ota_crc.c`

当前 sd3403 OTA 测试使用：

```text
PACK_CHECKSUM=PACK_NO_CHECKSUM
```

原因是当前仓库未补齐 `package_sha256*`、`package_sha256_rsa2048*` 实现。后续补齐 SHA256/RSA 后，才能把测试升级到真实包校验路径。

`src/component/connectivity/CMakeLists.txt` 按 feature 选择源码：

- LwM2M: `lwm2m/core/*.c`、`er-coap-13/er-coap-13.c`、`lwm2m_uniproton_port.c`。
- MQTT: `MQTTPacket/src/*.c`、`MQTTClient-C/src/MQTTClient.c`、`MQTTuniproton.c`。
- NB-IoT: `nb_iot/los_nb_api.c`、`nb_iot/nb_direct_link.c`、`nb_iot/nb_hw_direct.c`。

LwM2M 编译选项：

```text
LWM2M_CLIENT_MODE
LWM2M_LITTLE_ENDIAN
```

测试入口 `testsuites/iot-ota-test/CMakeLists.txt` 使用同一组 feature 宏决定测试源码：

| Feature 组合 | 测试源码 | 运行测试 |
|---|---|---|
| `CONFIG_OS_SUPPORT_OTA` | `ota/` + `common/` | `ota` |
| `CONFIG_OS_SUPPORT_LWM2M` | `lwm2m/` | `lwm2m` |
| `CONFIG_OS_SUPPORT_MQTT` | `mqtt/` | `mqtt` |
| `CONFIG_OS_SUPPORT_NB_IOT` | `nb_iot/` | `nb_iot` |
| `CONFIG_OS_SUPPORT_OTA` + `CONFIG_OS_SUPPORT_LWM2M` | `lwm2m_ota/` | `lwm2m_ota` |
| `CONFIG_OS_SUPPORT_OTA` + `CONFIG_OS_SUPPORT_MQTT` | `mqtt_ota/` | `mqtt_ota` |

单板统一 APP：

```bash
sg docker -c "docker exec uniproton_cmsis_verify bash -c 'cd /home/uniproton/UniProton/demos/sd3403/build && sh build_app.sh iot-ota-test'"
```

sd3403 构建必须顺序执行，不要并行跑多个 `build_app.sh`，否则共享构建目录可能产生对象文件或 `CMakeFiles` 竞争。

## 5. OTA/SOTA 特性

### 5.1 基本功能

OTA 组件提供与网络协议无关的升级公共能力：

- SOTA 状态机。
- OTA package header/TLV 解析。
- package 写入回调。
- upgrade flag 管理。
- CRC/checksum 抽象。
- flash read/write 适配接口。

### 5.2 UniProton 实现

主要目录：

```text
src/component/ota/include
src/component/ota/sota
src/component/ota/package
src/component/ota/flag_operate
src/component/ota/utility
```

测试侧 fake flash buffer 在：

```text
testsuites/iot-ota-test/common/iot_ota_test_common.c
```

`iot_ota_test_init_sota()` 注册 SOTA 回调：

- `get_ver`
- `sota_send`
- `sota_malloc`
- `sota_free`
- `sota_printf`
- `ota_info.read_flash`
- `ota_info.write_flash`
- `ota_info.flash_block_size`

### 5.3 测试逻辑

`ota` 测试包含两部分。

SOTA 负向/状态机测试：

- `sota_init(NULL)` 必须失败。
- `sota_parse()` 覆盖空输入、错误前缀、空输出、错误 message code、错误 packet type、短帧等。
- `sota_process()` 覆盖无 payload notify、unexpected version response、合法 new-version notify。

完整升级 happy path：

```text
build OTA package
  -> sota_init callbacks
    -> MSG_NOTIFY_NEW_VER
      -> MSG_GET_BLOCK for each package block
        -> MSG_EXC_UPDATE
          -> verify firmware bytes in fake flash
```

当前 package 使用 `PACK_NO_CHECKSUM`。文档和代码均不能把它描述为最终安全校验目标；后续 SHA256/RSA package 校验补齐后应扩展正负向用例。

## 6. MQTT 特性

### 6.1 基本功能

MQTT 是发布/订阅应用层协议，运行在 TCP 之上。UniProton 当前 MQTT 测试覆盖：

- MQTT CONNECT/CONNACK。
- SUBSCRIBE/SUBACK。
- PUBLISH。
- JSON profile report。
- cloud command receive。
- command response publish。
- OTA manifest request。
- OTA chunk request and receive。

### 6.2 UniProton 实现

主要目录：

```text
src/component/connectivity/mqtt/MQTTPacket
src/component/connectivity/mqtt/MQTTClient-C
src/component/connectivity/mqtt/MQTTClient-C/src/uniproton
```

`MQTTuniproton.c` 是 UniProton 适配层：

- TCP socket connect/read/write/disconnect。
- timer/tick 适配。
- MQTTClient-C `Network` 接口适配。

测试侧目录：

```text
testsuites/iot-ota-test/mqtt
testsuites/iot-ota-test/mqtt_ota
```

### 6.3 MQTT Demo 测试逻辑

板端 client：

```text
connect broker/helper
  -> subscribe demo/command
    -> publish demo/report JSON
      -> receive demo/command
        -> publish demo/response JSON
```

host helper 校验 JSON：

- report `msgType=deviceReq`。
- `serviceId=Battery`。
- `batteryLevel` 为整数。
- response `msgType=deviceRsp`。
- response `mid=1001`。
- `errcode=0`。
- `body.result=ok`。

### 6.4 MQTT OTA 测试逻辑

当前 topics：

```text
ota/manifest/request  board -> host, payload: GET_MANIFEST
ota/manifest          host  -> board, payload: MANIFEST <version> <size> <crc>
ota/request           board -> host, payload: GET_CHUNK <offset> <size>
ota/chunk             host  -> board, payload: CHUNK <offset> <size> <crc>\n<OTA package bytes>
```

板端校验：

- manifest version 必须匹配当前测试版本。
- manifest size 必须非零且不超过下载 buffer。
- manifest CRC 必须匹配完整 package。
- chunk offset 必须等于请求 offset。
- chunk size 必须等于请求 size。
- chunk CRC 必须匹配 chunk payload。

chunk 收齐后调用 SOTA 升级流程并验证 fake flash 中的 firmware payload。

### 6.5 Host 环境

推荐使用 Mosquitto：

Debian/Ubuntu/openEuler 系 host 可先安装并启动 broker：

```bash
sudo apt-get update
sudo apt-get install -y mosquitto mosquitto-clients
sudo systemctl enable --now mosquitto
mosquitto_sub -h 127.0.0.1 -t uniproton/smoke -C 1 &
mosquitto_pub -h 127.0.0.1 -t uniproton/smoke -m ok
```

openEuler/RHEL 系机器可使用系统包管理器安装同名 `mosquitto` 和 `mosquitto-clients` 包，或者从发行版仓库提供的 Mosquitto 包安装。确认 `mosquitto` 监听 `1883` 后启动 host helper：

```bash
python3 testsuites/iot-ota-test/host_iot_ota_server.py --target mqtt --mqtt-backend auto
```

`--mqtt-backend auto` 优先复用或启动 Mosquitto；不可用时退回 builtin broker。builtin broker 仅用于 fallback 验证，仍会解析 MQTT packet 并校验 payload，不作为完整云平台替代。

后续有真实云平台环境时，应继续补齐：

- 平台 topic。
- 鉴权和动态连接。
- QoS1。
- 断链重连。
- retained/session 行为。

## 7. LwM2M 特性

### 7.1 基本功能

LwM2M 是设备管理协议，运行在 CoAP 之上，典型 UDP 端口为 `5683`。当前 UniProton LwM2M 测试覆盖：

- client registration。
- Security Object。
- Server Object。
- Device Object read/write/execute。
- Firmware Update Object package write/update execute。

### 7.2 UniProton 实现

主要目录：

```text
src/component/connectivity/lwm2m/core
src/component/connectivity/lwm2m/core/er-coap-13
```

迁入的真实 core 源包括：

- `liblwm2m.c`
- `registration.c`
- `management.c`
- `objects.c`
- `observe.c`
- `transaction.c`
- `packet.c`
- `data.c`
- `tlv.c`
- `uri.c`
- `block1.c`
- `er-coap-13/er-coap-13.c`

UniProton port/shim：

```text
lwm2m_uniproton_port.c
lwm2m_uniproton_port.h
osdepends/atiny_osdep.h
connection.h
commandline.h
object_comm.h
```

port 层提供：

- UDP socket session/connection。
- `lwm2m_malloc/free/strdup/gettime/rand/delay`。
- `atiny_*` 内存、时间、互斥、printf 适配。
- `lwm2m_connect_server()`。
- `lwm2m_close_connection()`。
- `lwm2m_buffer_send()`。
- `lwm2m_buffer_recv()`。
- `lwm2m_session_is_equal()`。

测试侧 LwM2M client helper：

```text
testsuites/iot-ota-test/lwm2m/iot_lwm2m_core_client.c
```

它构造最小对象模型：

- Security Object: no-sec server URI、bootstrap=false、short server ID。
- Server Object: short server ID、lifetime=20、binding=U。
- Device Object: manufacturer/model/serial/version/current time/UTC offset/timezone/reboot。
- Firmware Update Object: Package、Package URI、Update、State、Update Result。

Security Object URI 由传入 host/port 动态生成，例如：

```text
coap://192.168.7.10:5683
```

### 7.3 LwM2M Demo 测试逻辑

```text
board client
  -> lwm2m_init
    -> lwm2m_configure(endpoint=uniproton-sd3403, objects=Security/Server/Device)
      -> lwm2m_step sends Register /rd
        -> host ACK 2.01 Created
          -> host READ /3/0/0
          -> host WRITE /3/0/14
          -> host EXECUTE /3/0/4
            -> board marks read/write/execute success
```

成功条件：

```text
read_ok && write_ok && execute_ok
```

### 7.4 LwM2M OTA 测试逻辑

```text
board client
  -> lwm2m_init
    -> lwm2m_configure(endpoint=uniproton-sd3403, objects=Security/Server/Device/Firmware)
      -> Register /rd with </5/0>
        -> host WRITE /5/0/0 package bytes
          -> board stores package
        -> host EXECUTE /5/0/2 update
          -> board marks fw_executed
            -> SOTA upgrade on received package
```

成功条件：

```text
fw_executed && package_len > 0
SOTA upgrade succeeds
firmware payload matches fake flash
```

### 7.5 Host 环境

builtin LwM2M fallback：

```bash
python3 testsuites/iot-ota-test/host_iot_ota_server.py --target lwm2m --lwm2m-backend builtin
python3 testsuites/iot-ota-test/host_iot_ota_server.py --target ota --target lwm2m --lwm2m-backend builtin
```

builtin helper 会校验：

- Register `Uri-Path=rd`。
- Register query `ep=uniproton-sd3403`。
- Register query `lt=20`。
- Register query `lwm2m=1.0`。
- Register query `b=U`。
- object link payload 包含 `/3/0`，OTA 场景还包含 `/5/0`。
- READ/WRITE/EXECUTE 的 ACK、CoAP code 和关键 payload。

标准 LwM2M server 路径使用 Leshan：

先准备 Java。若需要从源码构建 fallback，再安装 Maven 和 Git：

```bash
sudo apt-get update
sudo apt-get install -y openjdk-17-jdk maven git
java -version
mvn -version
git --version
```

`tools/leshan-server-demo.jar` 不随仓库提供。可从 Maven Central 下载 Leshan demo fat jar：

```bash
mkdir -p tools
wget -O tools/leshan-server-demo.jar \
  https://repo1.maven.org/maven2/org/eclipse/leshan/leshan-server-demo/2.0.0-M15/leshan-server-demo-2.0.0-M15-jar-with-dependencies.jar
wget -O /tmp/leshan-server-demo.jar.sha1 \
  https://repo1.maven.org/maven2/org/eclipse/leshan/leshan-server-demo/2.0.0-M15/leshan-server-demo-2.0.0-M15-jar-with-dependencies.jar.sha1
printf '%s  %s\n' "$(cat /tmp/leshan-server-demo.jar.sha1)" tools/leshan-server-demo.jar | sha1sum -c -
```

如果 Maven Central 不可访问，可从源码构建 demo jar 后复制：

```bash
mkdir -p tools
git clone https://github.com/eclipse-leshan/leshan.git /tmp/leshan
cd /tmp/leshan
mvn -pl leshan-server-demo -am package -DskipTests
cp leshan-server-demo/target/*jar-with-dependencies.jar \
  /home/liuxi/code/UniProton/tools/leshan-server-demo.jar
```

随后启动 host helper：

```bash
python3 testsuites/iot-ota-test/host_iot_ota_server.py \
  --target ota \
  --target lwm2m \
  --lwm2m-backend leshan \
  --leshan-jar tools/leshan-server-demo.jar
```

也可以通过环境变量或参数指定：

```bash
LESHAN_SERVER_JAR=/path/to/leshan-server-demo.jar \
python3 testsuites/iot-ota-test/host_iot_ota_server.py --target lwm2m --lwm2m-backend leshan
```

## 8. NB-IoT 特性

### 8.1 基本功能

NB-IoT 是低功耗广域蜂窝承载，不是 OTA 协议本身。UniProton 当前 NB-IoT 测试目标是验证公开 API 和 bearer 边界：

- `nb_init()`。
- `nb_report()`。
- `nb_deinit()`。
- direct-link bearer 与 host proxy 的连接和上报。

### 8.2 UniProton 实现

主要目录：

```text
src/component/connectivity/nb_iot
```

关键文件：

- `los_nb_api.h`
- `los_nb_api.c`
- `nb_direct_link.c`
- `nb_direct_link.h`
- `nb_hw_direct.c`

当前 sd3403 无真实 NB-IoT 模组和基站环境，所以 direct-link bearer 通过 host TCP proxy 验证 API 行为。未来真实模组环境应替换 bearer 实现，不改变上层 `los_nb_api` 调用方式。

### 8.3 测试逻辑

```text
board nb_iot test
  -> nb_init(host, port)
    -> direct-link CONNECT frame to host proxy
      -> nb_report("feature:nb_iot")
        -> host proxy ACK
          -> nb_deinit
```

host proxy 端口默认：

```text
56830
```

启动：

```bash
python3 testsuites/iot-ota-test/host_iot_ota_server.py --target nb_iot
```

当前 direct-link 只是 bearer fallback，不代表完整 NB-IoT 模组认证、AT 命令、网络附着、基站链路和运营商平台行为。

## 9. 组合测试

### 9.1 OTA + LwM2M

启用宏：

```text
CONFIG_OS_SUPPORT_OTA=y
CONFIG_OS_SUPPORT_LWM2M=y
```

同一固件内运行：

```text
ota
lwm2m
lwm2m_ota
```

host 启动：

```bash
python3 testsuites/iot-ota-test/host_iot_ota_server.py --target ota --target lwm2m --lwm2m-backend builtin
```

### 9.2 OTA + MQTT

启用宏：

```text
CONFIG_OS_SUPPORT_OTA=y
CONFIG_OS_SUPPORT_MQTT=y
```

同一固件内运行：

```text
ota
mqtt
mqtt_ota
```

host 启动：

```bash
python3 testsuites/iot-ota-test/host_iot_ota_server.py --target ota --target mqtt --mqtt-backend auto
```

### 9.3 全功能

启用宏：

```text
CONFIG_OS_SUPPORT_OTA=y
CONFIG_OS_SUPPORT_LWM2M=y
CONFIG_OS_SUPPORT_MQTT=y
CONFIG_OS_SUPPORT_NB_IOT=y
```

同一固件内运行：

```text
nb_iot
ota
lwm2m
mqtt
lwm2m_ota
mqtt_ota
```

host 启动：

```bash
python3 testsuites/iot-ota-test/host_iot_ota_server.py \
  --target all \
  --mqtt-backend auto \
  --lwm2m-backend builtin
```

## 10. Host Helper

脚本：

```text
testsuites/iot-ota-test/host_iot_ota_server.py
```

通用参数：

```text
--host                  default: 0.0.0.0
--target                all, ota, lwm2m, mqtt, nb_iot; can repeat
--nb-iot-port           default: 56830
--lwm2m-port            default: 5683
--leshan-web-port       default: 8080
--leshan-jar            default: tools/leshan-server-demo.jar or LESHAN_SERVER_JAR
--mqtt-port             default: 1883
--mqtt-backend          auto, mosquitto, builtin
--lwm2m-backend         builtin, leshan
```

默认 sd3403 网络：

```text
host IP:  192.168.7.10
board IP: 192.168.7.2
```

host helper 职责：

- NB-IoT proxy: 接收 direct-link frame，校验 CONNECT/REPORT 状态。
- MQTT backend: 优先使用 Mosquitto，fallback 使用 builtin MQTT packet handler。
- LwM2M backend: fallback 使用 builtin CoAP/LwM2M helper，标准路径使用 Leshan。
- OTA package source: 生成测试 package，并提供给 LwM2M Firmware Update 或 MQTT OTA chunk。
- 关键 helper 线程异常时主进程退出非零，避免 host 侧失败被误判为板端通过。

## 11. 当前边界和后续补齐

当前已完成：

- OTA/SOTA happy path 和负向 parser/state-machine 用例。
- LwM2M 真实 core、CoAP、UDP socket、对象模型路径。
- LwM2M builtin host helper 的注册参数和 CoAP ACK/code/payload 校验。
- MQTTClient-C/MQTTPacket 经 TCP socket 的 demo 和 OTA 路径。
- MQTT OTA manifest version/size/CRC 和 chunk offset/size/CRC 校验。
- NB-IoT `los_nb_api` direct-link bearer API 边界验证。
- sd3403 全功能测试已覆盖 `ota`、`lwm2m`、`mqtt`、`nb_iot`、`lwm2m_ota`、`mqtt_ota`。

依赖外部环境或缺失实现的补齐项：

- OTA SHA256/RSA package 校验。需要补齐 `package_sha256*` 和 `package_sha256_rsa2048*`。
- MQTT 平台 topic、鉴权、动态连接、QoS1、断链重连。
- 真实 NB-IoT 模组、AT/bearer、基站和运营商平台验证。

当前环境可继续增强的测试项：

- 标准 Leshan 环境下的 LwM2M server 回归矩阵。
- LwM2M Firmware Update Package URI `/5/0/1` 标准下载路径。
- Device Object WRITE 后 READ 回读校验。
- NB-IoT 未 init、deinit 后 report、host 不可达、空 payload、长 payload 等负向用例。
- MQTT QoS1、断链后失败路径、异常 manifest/chunk 响应等负向用例。
- 按 feature 组合拆分独立回归：`OTA`、`LWM2M`、`MQTT`、`NB_IOT`、`OTA+LWM2M`、`OTA+MQTT`、`ALL`。
