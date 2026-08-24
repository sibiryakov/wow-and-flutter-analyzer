# Knowledge: practical setup, workflow and troubleshooting

Sources: tapeheads thread "Version 8 of the WFGUI" (100 posts, 2017-03-03 to
2024-04-04, post numbers cited); `docs/` images; Metzler where noted.

## What you need

| Item | Notes |
|---|---|
| A test tape with a **3.00 kHz or 3.15 kHz** tone | Non-negotiable. A 1 kHz tone will not work (posts #15/#16). Sources: eBay, or thread member A.N.T. (post #16). |
| A **line input** on the PC | Line in strongly preferred; mic in works but is noisier. Many modern desktops/laptops have no line input at all (post #92: the Dell Precision 3650 has only a headset jack and line *out*) — a cheap USB interface solves it (post #93). |
| **RCA → 3.5 mm stereo cable** | Deck line out → PC line in. This is exactly the cable photographed in `docs/IMG_3288.jpeg`. |
| Windows | MFC app. Runs under Wine on Linux/macOS — see below. |

Connection is simply **tape deck line out → computer line/mic in** (post #14).

## Basic workflow

1. **Plug the cable in before launching.** Many sound cards only expose the input
   once a jack is inserted; with no input device WFGUI shows
   "NO Input Devices Found.." and exits (posts #88, #89, #91).
2. Select the input device in the top-left dropdown.
3. Choose **3000 Hz or 3150 Hz** to match your test tape.
4. Choose the weighting in the dropdown above the meter — **set it before
   pressing Start**, it locks once running (posts #96, #98).
5. Tick **Save wave** / **Save log** before Start if you want files (post #47).
6. Press **Start**, play the tape, adjust level until the LED stays green.
7. Read the numbers; press **Stop** when done.

## Reading the display

Referring to `docs/wf-meter-ui.png`:

| Element | Meaning |
|---|---|
| Device dropdown (top left) | Input selection — see the index bug below |
| Green LED | Lit only while the signal passes both validity gates |
| Level bar | Peak input level of the current 0.1 s buffer |
| **Frequency** | Measured carrier, 0.1 Hz resolution, updated 1×/s → **tape speed** |
| **Range** 0.1/0.4/1/4% | Full-scale for scope and needle; display only |
| **RMS (%)** | RMS over the last 1 second |
| Weighting dropdown | Unweighted / DIN / Wow / Flutter |
| Needle + **Peak** | Live quasi-peak (DIN-style, ~1 s decay) |
| "Max in 10 sec … RMS … quasi-peak" | Maxima of both over a rolling 10 s window |
| Deviation graph | Instantaneous weighted deviation, % |

Worked example from the screenshot: 3150 Hz selected, Frequency **3105.4** ⇒
speed error **−1.42%** (running slow); RMS **0.1395%**; live quasi-peak
**0.199%**; 10 s maxima 0.1489% RMS / 0.2724% quasi-peak.

## Which reading do I compare to the manufacturer's spec?

WFGUI computes both detectors at once, so the *weighting selection* plus *which
number you read* determines the standard:

| Spec quoted as | Set weighting to | Read |
|---|---|---|
| **DIN / IEC / CCIR** (weighted peak) | `DIN` | the needle / quasi-peak figure |
| **WRMS / JIS** (weighted rms) | `DIN` | the **RMS (%)** box |
| **NAB / unweighted RMS** | `Unweighted` | the **RMS (%)** box |
| wow only / flutter only (diagnostic) | `Wow` / `Flutter` | either |

This is the mapping the thread eventually converged on — post #74: *"DIN RMS =
WRMS, DIN PEAK = DIN, and JIS = WRMS when a 3 kHz reference tape is used"* — and
it is consistent with the code (`02-implementation.md`).

The `DIN` label is genuinely misleading: it names the **weighting curve**, not
the detector. Posts #78, #80 and #82 all argue it should read "Weighted Wow &
Flutter". Worth saying plainly in the README.

**Caveats before trusting a spec-sheet comparison:**

- Manufacturers quote hand-picked, carefully adjusted samples (post #70).
- The **reference tape dominates the result.** Absolute numbers mean little
  unless the same tape is used across comparisons (post #70). Metzler p.48 adds
  that on very good machines the tape may be worse than the machine.
- WFGUI has **no 2-sigma mode**; many specs are 2-sigma. Its 10-second *maximum*
  reads higher than a 2-sigma value.
- JIS's "slow" detector is not literally implemented — WFGUI substitutes a
  1-second RMS.
- The peak-to-peak vs centre-to-peak factor-of-two ambiguity
  (`01-theory-and-standards.md`) applies to the spec, not to WFGUI.

Sanity reference point from post #70: a fully serviced Nakamichi Dragon
consistently measures at or near its 0.018% spec, cross-checked against a
National VP-7750A, a Leader LFM-3610 and five Nakamichi T-100s.

## Establishing your noise floor (highly recommended)

Post #17 measured a **PC loopback of an Audacity-generated 3 kHz tone at
0.0025% RMS**. Doing the same on your own rig tells you how much of a reading is
your sound card rather than your deck. Compare with the same post's real decks:
ZX-7 0.045%, RX-202 0.0425%, JVC TD-V661 0.055%, Aiwa HS-PC202 mkIII 0.25%.

This is a cheap, decisive calibration step and belongs in the README.

## Measuring along the whole tape

A.N.T.'s original purpose for the logging feature (post #1): log at 1 s intervals
across a full cassette, then plot speed and W&F against position. Post #5:
*"the information available from these logs for the full length of a cassette is
the best way of checking the mechanics performance."*

Metzler p.47 independently recommends measuring at **beginning, middle and end**
because W&F varies considerably along the tape.

Post #86 shows the technique finding a real fault: an autoreverse deck measured
~2× worse in reverse than forward, pointing at the reverse capstan/pinch roller.

## Working with `log.txt`

- Always named `log.txt`, in the program's folder, **overwritten every run** —
  rename it after each measurement (posts #1, #47).
- Three space-separated columns: **frequency, RMS, quasi-peak** (post #56,
  confirmed in source).
- One row per second (posts #58, #59).
- To import: open Excel **first**, then File → Open → Text Files, and it parses
  as fixed-width/space-delimited into three columns (post #7). Excel defaults to
  `*.xls`, so switch the filter to all file types (post #45).
- Post #86 plots a 10-point moving average to make trends readable.

> **Caution:** logging does not work in a binary built from this repository —
> the checkbox is not wired up. See `02-implementation.md` §4.

## Working with `WF_out.dat` — finding the guilty part

This is the most valuable advanced feature and the thread took years to pin down.
Source-confirmed parameters:

- **Raw headerless PCM**, signed **16-bit**, **mono**, little-endian.
- **Sample rate = 2 × the carrier**: **6000 Hz** for a 3000 Hz tone, **6300 Hz**
  for 3150 Hz (posts #10, #12, #64 — and confirmed in source).
- Contents: the **unweighted** demodulated deviation, i.e. the discriminator
  output, irrespective of the weighting selected in the UI.
- Always named `WF_out.dat`, overwritten each run. ~3.7 MB for 5 minutes (post #6).

**Audacity procedure** (post #63): File → Import → Raw Data, choosing signed
16-bit, mono, and 6000 or 6300 Hz to match the tone.

Then run a spectrum analysis and read off the peaks. Per Metzler p.50, a peak at
*f* Hz is a part rotating *f* times per second, and

```
circumference = tape speed ÷ peak frequency
```

At cassette speed (4.76 cm/s), a 5 Hz peak ⇒ a part of ~0.95 cm circumference.
Post #7 states the purpose exactly: *"useful if you need to determine the source
of an excessive W&F."*

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| **"NO Input Devices Found.."** then exits | No input device present. Plug the cable in *before* launching (posts #88, #91). Check the machine actually has a line input (post #92); add a USB interface if not (post #93). |
| **"Signal too low"** that never clears | Input peak below ~−56 dBFS. Check level, Windows input enablement, and mic privacy settings (post #98). Most often it is the **wrong device actually being opened** — see next row. |
| **Wrong input used regardless of selection** | Known bug: the combo index is passed as the device ID. Try the *other* entries until one works — post #96 got a Focusrite signal by selecting "microphone array". |
| **Device dropdown shows only one line** | Dialog-template bug. Use the **mouse wheel** (post #34) or the **arrow keys** (post #36) to scroll the hidden entries. |
| **Cannot change weighting without restarting** | Real bug, confirmed in source. Set it before Start (posts #96, #98). |
| **`log.txt` / `WF_out.dat` nowhere to be found** | Written to the program's working directory. Under UAC, writes into `Program Files` get silently redirected to `C:\Users\<you>\AppData\Local\VirtualStore\...` (post #59). **Fix: run it from a normal user-writable folder** (posts #52, #53). |
| Frequency box shows a number but nothing else updates | Tone more than ±5% off nominal, or wrong 3000/3150 setting. |
| Focusrite / vendor driver problems on Win10-11 | Post #99: uninstall the vendor driver and use the generic Windows one. |
| General Win10/11 flakiness | Post #32: XP SP3 compatibility mode + "Run as administrator" helped. Others report it working with no compatibility settings at all (posts #30, #35). |

## Running on Linux / macOS

Works under **Wine** (posts #19, #22, #23, #24). It fails silently if `MFC42.DLL`
is missing — drop that DLL next to the exe (post #22). Use
`WINEDEBUG=+dll` to see what is missing, since Wine is otherwise completely quiet
about it (post #24). Post #23 notes a MacBook's noise floor was markedly lower
than a cheap desktop PC's.

## Project status

Post #37: the original author, **Alex Freed**, stated he had no plans for further
development — he wrote it to test his own decks — *"though any aspiring
programmers are welcome to take a crack at it."* Post #21 asked for the source to
be opened. That is the context for this repository.

Attribution: the app is by **Alex Freed**; the About box carries a PayPal donation
address. A.N.T. (who distributed v8 and started the thread) explicitly redirects
credit: *"All thanks should go to Alex Freed"* (post #3).
