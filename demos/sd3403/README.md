# SD3403 UniProton board support

UniProton runs on CPU3 (Cortex-A55) of the SD3403 (SS928) board; Linux
starts and stops it through MICA.

## Board support

- `bsp/sd3403/cpu_config.h`, `build/sd3403.ld`: image at 0x4b000000, as
  in the board vendor's firmware; OpenAMP shared memory at 0x48000000,
  followed by the log memory and the WOW region, inside the board's
  `mcs_baremetal_mem` reservation (0x48000000-0x4affffff).
- `apps/openamp/main.c`: the wait for the tty endpoint at startup is
  bounded to 5 s, so the application also starts when nobody opens the
  console.
- `bsp/timer.c`, `config/prt_config.h`: 1 kHz OS tick on an absolute
  deadline grid; the counter runs at 24 MHz (`SYS_COUNTER_FREQ`), not the
  25 MHz the firmware leaves in CNTFRQ_EL0. Tickless idle
  (`src/extended/lowpower/tickless/prt_tickless.c`) still takes its reload
  from CNTFRQ_EL0, so the tick runs about 4% slow whenever it sleeps; the
  sd3403 defconfig leaves tickless off.

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
