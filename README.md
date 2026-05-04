# Curiosity Mars Time

A Pebble watchface displaying Mars Local Mean Solar Time at the Curiosity rover's landing site in Gale Crater, along with the spacecraft clock (SCET), UTC day-of-year, and your local time.

## What it shows

| Row | Content | Example |
|-----|---------|---------|
| Sol | Mission sol since landing | `MSL sol 4543` |
| Mars time | Mars HH:MM:SS (LMST) | `14:22:07` |
| SCET | Spacecraft event clock | `434862910` |
| UTC | UTC day-of-year and time | `utc 123T18:44` |
| Local time | Your timezone + 12-hour time | `EST 2:44pm` |
| Date | Local calendar date | `Sat 05/03` |

Tapping the watch activates seconds mode for a configurable duration (default 10 s), showing true seconds on the Mars time and SCET rows. At all other times both rows show `00` for the sub-minute digits.

## Platforms

All seven Pebble platforms are supported. Font sizes scale with display size.

| Platform | Display | Notes |
|----------|---------|-------|
| aplite, basalt, diorite, flint | 144×168 rect | Mulish Bold 30/18, Regular 14 |
| chalk | 180×180 round | Mulish Bold 36/22, Regular 12 |
| emery, gabbro | 200×228 / 260×260 | Mulish Bold 42/24, Regular 18 |

## Configuration

Open the watchface settings from the Pebble app:

- **Seconds duration** — how many seconds the seconds display stays active after a wrist tap (3–60 s)
- **Timezone label** — short label shown next to local time (e.g. `EST`, `PDT`, up to 7 characters)

## Building

```bash
pebble build
pebble install --emulator basalt    # small rect
pebble install --emulator chalk     # round
pebble install --emulator emery     # large rect
```

Requires the [Rebble Pebble SDK](https://developer.repebble.com) and pebble-tool:

```bash
uv tool install pebble-tool --python 3.13
```

## Mars time math

Mars Local Mean Solar Time is computed as Earth seconds elapsed since the MSL landing epoch (2012-08-05 05:17:57 UTC), scaled by the Mars/Earth sol ratio (1,000,000 / 1,027,491 ≈ 1/1.027491). All arithmetic uses `int64_t` — there is no floating point anywhere, as the ARM Cortex-M processors in Pebble hardware have no hardware FPU.

The epoch (`MARS_EPOCH_UNIX`) is anchored to local midnight at Gale Crater (137.42°E), so the displayed time is true LMST at Curiosity's landing site — not Mars prime-meridian time.
