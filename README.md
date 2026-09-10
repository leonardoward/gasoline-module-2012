# Fuel-Level Module — Baja SAE USB (2012)

A CAN-bus **fuel-level sender** for the off-road cars built by the **Baja SAE team of
Universidad Simón Bolívar** ("Baja USB"). A magnet riding on the fuel-tank float
passes a vertical row of six Hall-effect switches; a Freescale S08 microcontroller
turns that into an 11-step "how much fuel is left" reading and broadcasts it on the
car's CAN bus, where the dashboard / on-board data system picks it up.

It is one module of the wider **Baja SAE USB vehicle electronics** effort (tachometer,
brake-pressure, speedometer, wheel-alignment, all sharing one CAN bus).

- **Board silkscreen:** `BAJA USB` · `Leonardo J. Ward` · tag `40117`
- **Firmware dates:** February–May 2012 (CodeWarrior project)
- **Hardware output generated:** 21 November 2012 (Altium)

---

## How it measures fuel

The tank has a float on an arm. A small permanent magnet is fixed to the float, and
the module's PCB is mounted alongside the float's travel with **six Micronas
Hall-effect switches (HS1–HS6)** in a straight line. As the fuel level changes, the
magnet moves along the line and trips one — or two adjacent — switches.

The switches are open-drain, active-low, with pull-ups, wired to port pins
**PTA1–PTA6**. The firmware reads all six at once, inverts them (`pattern ^ 0b111111`,
so a `1` means "magnet here") and decodes the pattern to a level of **0–11**:

| Switches seeing the magnet | Level |
|---|---|
| top one only            | 1  |
| top two                 | 2  |
| next single (or top three) | 3  |
| that pair               | 4  |
| … stepping down the array … | 5 – 10 |
| bottom one only         | 11 |
| none                    | 0  |

A single switch gives an odd level, an adjacent pair gives the even level between —
so six switches resolve **11 discrete steps**. Three-in-a-row patterns are folded onto
the middle step so a wide magnet still reads cleanly.

## Sampling and averaging

A **TPM1 timer interrupt runs every 200 ms (5 Hz)**. On each tick the firmware:

1. reads and decodes the Hall pattern, adding the level to an accumulator;
2. toggles the heartbeat LED (DM1).

Every **10 ticks (2 s)** the main loop divides the accumulator by 10 — a simple
running average that rejects float slosh — and transmits the result.

## CAN output

| Field | Value |
|---|---|
| Transceiver | NXP **TJA1040** (U3), high-speed CAN |
| Frame | standard (11-bit) data frame |
| ID | `0x222` |
| Length | 1 byte |
| Payload | averaged fuel level, 0–11 |
| Period | ~2 s |

Frames are sent round-robin through the three MSCAN transmit buffers
(`CAN1_SendFrame(buffer, 0x222, DATA_FRAME, 1, &level)` with `buffer` cycling 0→1→2).
Bit timing: SJW 1, TSEG1 6, TSEG2 7 (≈50 % sample point).

---

## Hardware

Long, narrow **2-layer board** (Altium Designer) so it fits along the tank next to the
float.

| Ref | Part | Function |
|---|---|---|
| U1 | Freescale **MC9S08DZ60** (32-pin LQFP) | 8-bit S08 MCU with on-chip MSCAN; 16 MHz bus |
| U2 | Micrel **MIC2954** | low-dropout regulator, car rail → 5 V |
| U3 | NXP **TJA1040** | high-speed CAN transceiver |
| HS1–HS6 | **Micronas** Hall-effect switch (SOT89) | fuel-level pickup |
| DM2 / DM1 | LEDs | power / heartbeat |
| P1 | 4-pin header | +V, GND, CANH, CANL to the car harness |
| P2 | 6-pin (2×3) header | BDM programming/debug (P&E Multilink) |
| S1 | push-button | reset |
| R1–R19, C1–C24 | passives | Hall pull-ups, LED limiting, decoupling |

The Altium project also carries custom library parts for the Micronas sensor
(`senssorHALLMicronas_MS.SchLib`), a TIP31C power transistor and a `PCB_DisplayGas`
footprint library — leftovers from an earlier fuel-gauge **display** board that are not
populated on this design.

## Firmware

Freescale **CodeWarrior for HCS08** with **Processor Expert 3.07**. The generated
components are:

| Component | Bean | Role |
|---|---|---|
| `Sensor` | BitsIO (input) | 6-bit read of PTA1–PTA6 |
| `TI1`    | TimerInt | 200 ms periodic interrupt on TPM1 |
| `CAN1`   | FreescaleCAN | MSCAN driver |
| `LED`    | BitIO (output) | heartbeat LED on PTD5 |
| `Cpu`    | MC9S08DZ60_32 | clock / init |

The hand-written code is small: `Sources/Gasolina.c` (the 2 s average + transmit loop)
and the `TI1_OnInterrupt` handler in `Sources/Events.c` (sample + decode).

---

## Repository layout

```
firmware/Gasolina 2011-2012/
    CODE/                     working copy of the sources + Processor Expert output
        Gasolina.c            main loop: 2 s average, CAN transmit
        Events.c              TI1_OnInterrupt: sample + decode the Hall pattern
        Sensor.c / .h         PTA1–PTA6 input
        CAN1.c / .h           MSCAN driver
        TI1.c / .h            200 ms timer
        LED.c / .h            heartbeat LED
        Cpu.c / .h, IO_Map.*, Vectors.c, PE_*   generated support
    Gasolina*/                CodeWarrior / Processor Expert project variants
    bin/                      built Project.abs, .s19, .map
    DOC/                      Processor Expert reports

hardware/
    Esquemático Módulo Gasolina.SchDoc     schematic
    PCB Módulo Gasolina.PcbDoc             layout
    Proyecto Módulo Gasolina.PrjPCB        Altium project
    Librerías/                             custom symbol / footprint libraries
    Project Outputs .../BOM (Gasolina).xls bill of materials

IMG_2195.JPG                 assembled board on the bench, powered up
```

## Building the firmware

Open `firmware/Gasolina 2011-2012/Gasolina.mcp` (or one of the `Gasolina-*` Eclipse
projects) in CodeWarrior for HCS08 / MCU 10.x, build, and flash `Project.abs` /
`Project.abs.s19` to the MC9S08DZ60 through the P2 BDM header with a P&E Multilink.

## License

Released under the [MIT License](LICENSE).
