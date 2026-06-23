# OTA-MQTT Test

This suite validates MQTT-driven OTA without LwM2M.

```
UniProton MQTT client                 MQTT broker / host helper
        |                                      |
        | CONNECT/SUBSCRIBE                    |
        |------------------------------------->|
        |<-------------------------------------| CONNACK/SUBACK
        |                                      |
        | PUBLISH ota/manifest/request         |
        |------------------------------------->|
        |<-------------------------------------| PUBLISH ota/manifest
        |                                      |
        | PUBLISH ota/request GET_CHUNK ...    |
        |------------------------------------->|
        |<-------------------------------------| PUBLISH ota/chunk metadata + bytes
        |                                      |
        v                                      v
                          SOTA/package parse and flash write
```

What this tests:

- MQTT connect/subscribe/publish using the real MQTT client stack.
- OTA manifest request and response over MQTT topics, including version, size and CRC validation.
- OTA package chunk download over MQTT topics, including offset, size and per-chunk CRC validation.
- SOTA/package parse and firmware write callback.

What this does not test:

- LwM2M registration or Firmware Update Object handling.
- LwM2M Device Object demo behavior.
- Real NB-IoT base station or modem hardware.
