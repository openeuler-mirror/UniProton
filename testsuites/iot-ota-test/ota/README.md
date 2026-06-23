# OTA Test

This suite validates the local protocol-independent OTA/SOTA upgrade flow without involving LwM2M, MQTT, or NB-IoT.

```
test case
  |
  | build OTA package header + firmware image
  v
SOTA client
  |
  | MSG_NOTIFY_NEW_VER: version, block size, block count
  | MSG_GET_BLOCK: block 0..N through SOTA parser/process path
  | MSG_EXC_UPDATE: execute upgrade
  v
OTA package parser/writer
  |
  | write_flash callback
  v
test flash buffer
  |
  | compare with expected firmware image
  v
PASS/FAIL
```

What this tests:

- SOTA initialization with OTA flash callbacks.
- Version notification handling.
- Block-by-block package download handling.
- OTA package parsing and firmware write path.
- Final firmware image content written to the target flash area.

What this does not test:

- LwM2M firmware update trigger.
- MQTT network package transport.
- NB-IoT bearer reporting.

Those are covered by enabling `ota + lwm2m`, `ota + mqtt`, and `nb_iot` features in `iot-ota-test`.
