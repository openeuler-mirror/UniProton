# MQTT Test

This suite validates MQTT bind, JSON uplink report, JSON downlink command, and JSON response behavior.

```
UniProton MQTT client                 MQTT broker / host helper
        |                                      |
        | CONNECT client_id=sd3403-client      |
        |------------------------------------->|
        |<-------------------------------------| CONNACK
        |                                      |
        | SUBSCRIBE demo/command               |
        |------------------------------------->|
        |<-------------------------------------| SUBACK
        |                                      |
        | PUBLISH demo/report                  |
        | {msgType:deviceReq, Battery data}    |
        |------------------------------------->|
        |                                      |
        |<-------------------------------------| PUBLISH demo/command
        |                                      | {msgType:cloudReq, cmd, mid}
        | PUBLISH demo/response                |
        | {msgType:deviceRsp, mid, errcode}    |
        |------------------------------------->|
```

What this tests:

- MQTT connect using the real MQTT client stack.
- Subscribe to a downlink command topic.
- Publish a JSON data report and have the host validate its fields.
- Receive and parse a cloud command JSON message.
- Publish a command response JSON message and have the host validate its fields.

What this does not test:

- OTA firmware chunk download.
- LwM2M registration.
- NB-IoT bearer reporting.

Firmware chunk download over MQTT is covered when both OTA and MQTT features are enabled.
