# OTA-LwM2M Test

This suite validates LwM2M-driven OTA without MQTT.

```
LwM2M server / host helper             UniProton LwM2M client
        |                                      |
        |<-------------------------------------| POST /rd payload: </5/0>,</3/0>
        |------------------------------------->| 2.01 Created
        |                                      |
        | WRITE /5/0/0 Package bytes           |
        |------------------------------------->|
        |<-------------------------------------| 2.04 Changed
        |                                      |
        | EXECUTE /5/0/2 Update                |
        |------------------------------------->|
        |<-------------------------------------| 2.04 Changed
        |                                      |
        v                                      v
                                  SOTA/package parse and flash write
```

What this tests:

- LwM2M registration with Firmware Update Object `/5/0`.
- OTA package delivery through LwM2M Package resource `/5/0/0`.
- Update execution through `/5/0/2`.
- SOTA/package parse and firmware write callback.

What this does not test:

- MQTT manifest or chunk transport.
- MQTT demo bind/report/command/response behavior.
- Real NB-IoT base station or modem hardware.
