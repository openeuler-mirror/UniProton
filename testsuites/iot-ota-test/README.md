# IoT OTA Test Suites

The board-side app is `iot-ota-test`. Test cases are selected by `CONFIG_OS_SUPPORT_OTA`, `CONFIG_OS_SUPPORT_LWM2M`, `CONFIG_OS_SUPPORT_MQTT`, and `CONFIG_OS_SUPPORT_NB_IOT`.

Feature overview:

```text
ota
  Local SOTA/package validation path. It builds a test package, parses and processes SOTA
  notify/response frames, writes payload into fake flash, and covers parser/state-machine
  negative cases. The current package mode is PACK_NO_CHECKSUM until SHA256/RSA package
  verification code is available in the repository.

mqtt
  MQTT connectivity path through MQTTClient-C/MQTTPacket over TCP sockets. The demo covers
  connect, subscribe, publish, command receive, and response publish. The MQTT OTA case uses
  manifest and chunk topics, with version, size, offset, and CRC checks before invoking SOTA.

lwm2m
  LwM2M device-management path through liblwm2m core, CoAP, and UDP sockets. The client
  builds Security, Server, Device, and Firmware Update objects, registers endpoint
  uniproton-sd3403, and handles Device Object read/write/execute operations plus Firmware
  Update package write and update execute.

nb_iot
  NB-IoT public API and bearer boundary validation. sd3403 currently has no real NB-IoT
  module/base-station environment in this test, so nb_init/nb_report/nb_deinit are verified
  through the direct-link bearer and host proxy. A real module should replace only the bearer
  layer while keeping the upper los_nb_api path.
```

```
feature-selected tests
  ota    -> protocol-independent local SOTA/package flow
  lwm2m  -> LwM2M Device Object register/read/write/execute
  mqtt   -> MQTT bind/report/command/response
  nb_iot -> NB-IoT API and direct-link bearer

feature-combination tests
  ota + lwm2m -> LwM2M Firmware Update Object carries OTA package and trigger
  ota + mqtt  -> MQTT manifest/chunk topics carry OTA package and trigger
```

Host-side helper examples:

```bash
python3 testsuites/iot-ota-test/host_iot_ota_server.py --target lwm2m --lwm2m-backend builtin
python3 testsuites/iot-ota-test/host_iot_ota_server.py --target mqtt --mqtt-backend auto
python3 testsuites/iot-ota-test/host_iot_ota_server.py --target nb_iot
python3 testsuites/iot-ota-test/host_iot_ota_server.py --target ota --target lwm2m --lwm2m-backend builtin
python3 testsuites/iot-ota-test/host_iot_ota_server.py --target ota --target mqtt --mqtt-backend auto
```

Multiple `--target` options can be combined when a run needs more than one host service.

Host setup for MQTT:

```bash
sudo apt-get update
sudo apt-get install -y mosquitto mosquitto-clients
sudo systemctl enable --now mosquitto
mosquitto_sub -h 127.0.0.1 -t uniproton/smoke -C 1 &
mosquitto_pub -h 127.0.0.1 -t uniproton/smoke -m ok
```

Host setup for Leshan LwM2M server:

```bash
sudo apt-get update
sudo apt-get install -y openjdk-17-jdk maven git wget
mkdir -p tools
wget -O tools/leshan-server-demo.jar \
  https://repo1.maven.org/maven2/org/eclipse/leshan/leshan-server-demo/2.0.0-M15/leshan-server-demo-2.0.0-M15-jar-with-dependencies.jar
wget -O /tmp/leshan-server-demo.jar.sha1 \
  https://repo1.maven.org/maven2/org/eclipse/leshan/leshan-server-demo/2.0.0-M15/leshan-server-demo-2.0.0-M15-jar-with-dependencies.jar.sha1
printf '%s  %s\n' "$(cat /tmp/leshan-server-demo.jar.sha1)" tools/leshan-server-demo.jar | sha1sum -c -
python3 testsuites/iot-ota-test/host_iot_ota_server.py --target lwm2m --lwm2m-backend leshan --leshan-jar tools/leshan-server-demo.jar
```

If Maven Central is unavailable, build the same jar from source:

```bash
git clone https://github.com/eclipse-leshan/leshan.git /tmp/leshan
cd /tmp/leshan
mvn -pl leshan-server-demo -am package -DskipTests
cp leshan-server-demo/target/*jar-with-dependencies.jar \
  /home/liuxi/code/UniProton/tools/leshan-server-demo.jar
```

Source structure:

```text
common/iot_ota_test_common.c
  SOTA package flow plus sota_init/sota_parse/sota_process negative cases.

lwm2m/iot_lwm2m_core_client.c
  liblwm2m core client driver with Security, Server, Device and Firmware Update objects.

mqtt/mqtt_demo_client.c
  MQTTClient-C based report/command/response profile path.

mqtt_ota/mqtt_ota_client.c
  MQTT OTA manifest and chunk transport with version, size, offset and CRC validation.

host_iot_ota_server.py
  Host fallback services. Builtin LwM2M validates registration query and CoAP ACK/code/payload;
  MQTT fallback validates manifest/chunk metadata. Leshan and Mosquitto remain the standard host paths.
```

Current fallback code is intentionally stricter than a raw socket proxy. It validates protocol framing and payloads while sd3403 lacks some cloud, NB-IoT module, and checksum/signature environments.
