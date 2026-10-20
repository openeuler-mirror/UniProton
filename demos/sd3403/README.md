# SD3403 UniProton board support

UniProton runs on CPU3 (Cortex-A55) of the SD3403 (SS928) board; Linux
starts and stops it through MICA.

## Board support

- `bsp/sd3403/cpu_config.h`, `build/sd3403.ld`: image at 0x4b000000, as
  in the board vendor's firmware; OpenAMP shared memory at 0x48000000,
  followed by the log memory and the WOW region, inside the board's
  `mcs_baremetal_mem` reservation (0x48000000-0x4affffff).

## Build

In the UniProton v004 build container:

```bash
cd demos/sd3403/build
bash build_app.sh sd3403
```

## Deploy

Strip `sd3403.elf` and install it at the ClientPath of the `uniproton`
MICA instance (`/root/mica/sd3403.elf` on the board), then start the
instance.
