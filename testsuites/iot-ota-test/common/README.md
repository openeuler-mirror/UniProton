# Common Test Support

This directory contains shared configuration and helper code for the feature suites.

```
feature test
    |
    | include iot_ota_test_config.h
    | include iot_ota_test_common.h
    v
common helpers
    |
    | log/check/reset/build package/SOTA frame helpers
    v
component under test
```

Shared configuration:

- Host server: `192.168.7.10`
- LwM2M port: `5683`
- MQTT port: `1883`
- NB-IoT direct-link port: `56830`
- Endpoint: `uniproton-sd3403`
- MQTT client ID: `sd3403-client`

The board CMake does not inject these network values. Test-side configuration owns them.
