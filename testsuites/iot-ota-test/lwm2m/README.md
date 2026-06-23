# LwM2M Test

This suite validates UniProton LwM2M client behavior for the Device Object.

```
UniProton LwM2M client                 LwM2M server
        |                                   |
        | POST /rd ep=uniproton-sd3403     |
        | payload: </3/0>                  |
        |---------------------------------->| register endpoint
        |<----------------------------------| 2.01 Created
        |                                   |
        |<----------------------------------| READ /3/0/0 manufacturer
        |---------------------------------->| "Open Mobile Alliance"
        |                                   |
        |<----------------------------------| WRITE /3/0/14 UTC offset
        |---------------------------------->| 2.04 Changed
        |                                   |
        |<----------------------------------| EXECUTE /3/0/4 reboot
        |---------------------------------->| 2.04 Changed
```

What this tests:

- LwM2M endpoint registration.
- Device Object `/3/0` resource read semantics.
- Writable resource handling such as UTC offset.
- Executable resource handling such as reboot/factory-reset style commands.

What this does not test:

- Firmware Update Object `/5/0`.
- OTA package URI delivery.
- MQTT firmware download.

Firmware update over LwM2M is covered when both OTA and LwM2M features are enabled.
