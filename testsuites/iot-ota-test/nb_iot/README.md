# NB-IoT Test

This suite validates the NB-IoT API and bearer adapter boundary used on sd3403 without a real base station.

```
UniProton test
    |
    | los_nb_init(host, port)
    v
NB-IoT API
    |
    | nb_hw_adapter_init/detect/wait_network/query_ip/set_cdpserver
    v
NB hardware adapter
    |
    | direct-link frame, channel=nb_iot
    v
host-side bearer service
    |
    | OK / ERR
    v
UniProton test
    |
    | los_nb_report("feature:nb_iot")
    v
host-side bearer service
```

What this tests:

- Public NB-IoT initialization path.
- Hardware adapter abstraction calls.
- CDP server setup through the current no-base-station bearer.
- Payload report path through the current direct-link bearer.

What this does not test:

- Real NB module AT command transport.
- Real operator network attach.
- LwM2M or MQTT protocol behavior.

Real module integration should replace `nb_hw_adapter` while keeping the same public NB-IoT API. If a validation setup needs network transport, the direct-link bearer can be carried by a host-side proxy service.
