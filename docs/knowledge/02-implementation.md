# Knowledge: what the code actually does

All claims below are read off this repository's source. File:line references are
to the tracked `master` revision. `Copy of wfguiDlg.cpp` and
`Copy (2) of wfguiDlg.cpp` are **older backups** (verified by diff — they lack
logging, the 4% range and `SetScaleDecimals`); `wfguiDlg.cpp` is the live file.

## Signal chain end to end

```
sound card 44.1 kHz/16-bit/mono
  → validity gates (level, frequency)
  → process_2nd_order()            fixed pre-filter
  → zero-crossing period measurement with sub-sample interpolation   [DISCRIMINATOR]
  → err = (nominal_half_period − measured_half_period) / nominal_half_period
  ├─→ raw 16-bit value → WF_out.dat        (written BEFORE weighting)
  └─→ process_DIN / _wow / _flutter / _unweighted                    [WEIGHTING]
        ├─→ quasi-peak envelope  → needle, "quasi-peak" figure
        ├─→ 1-second RMS         → RMS (%) box
        ├─→ 7-sample average     → oscilloscope trace
        └─→ mean half-period     → Frequency box
```

## Audio input

`wfguiDlg.cpp:618-624` — the format is **fixed and not user-selectable**:
44100 Hz, 16 bits, **1 channel (mono)**. The device must report support for
`WAVE_FORMAT_4M16` or you get "Format not supported by device"
(`wfguiDlg.cpp:580-582`).

Mono means **only one channel is analysed** — consistent with post #65's
empirical finding that the data comes from the left channel alone.

Buffers are `nAvgBytesPerSec/10` = 4410 samples = **0.1 s** each
(`wfguiDlg.cpp:940`), with `MAX_BUFFERS = 2` (`wfguiDlg.h:13`). 0.1 s is the
fundamental processing tick for everything downstream.

## The two validity gates (this is what "Signal too low" means)

Per 0.1 s buffer, `wfguiDlg.cpp:704-729`:

1. **Level gate.** The maximum positive sample must be ≥ **50** (of 32767),
   i.e. about **−56 dBFS**. Below that: status shows `"Signal too low"`, the LED
   goes out, and the buffer is discarded. This is a very low bar — hitting it
   normally means the wrong input device, a muted input, or nothing connected.

2. **Frequency gate.** Zero crossings per buffer must be within
   `center_freq/5 × [0.95, 1.05]`, i.e. the incoming tone must be within
   **±5% of nominal** (`wfguiDlg.cpp:687-688, 722`). Outside that, the buffer is
   discarded and the Frequency box shows the out-of-range estimate
   (`zero_cross × 5` Hz) so you can still see roughly what is arriving.

   **Consequence:** a deck running more than 5% off speed cannot be measured at
   all — the meter simply never updates. Same if the wrong test tone is selected.

## Discriminator: zero-crossing period measurement, not FFT

`wfguiDlg.cpp:734-761`. Each 44.1 kHz sample is passed through
`process_2nd_order()` (a fixed 2nd-order bandpass, `filters.c:1-17`), then sign
changes are detected. The crossing instant is refined to **sub-sample resolution**
by linear interpolation:

```c
delta = -last_val * nanosec_per_sample / (this_val - last_val);
```

`interval` accumulates nanoseconds between successive crossings — the measured
**half-period**. Nominal is `proper_interval = 0.5e9 / center_freq` ns
(158730.2 ns at 3150 Hz; 166666.7 ns at 3000 Hz).

Fractional deviation (`wfguiDlg.cpp:777`):

```c
err = (proper_interval - interval) / proper_interval;
```

**Sign convention:** positive `err` = measured half-period shorter than nominal
= frequency high = **tape running fast**.

This is a centre-to-peak instantaneous deviation (see the ambiguity discussion in
`01-theory-and-standards.md`).

## Weighting filters — and how they map onto the standards

`filters.c`. The descriptive comments precede the function they describe:

| Combo entry | Function | Filter | Standard role |
|---|---|---|---|
| `DIN` | `process_DIN` | `BpBe2/1.2-15` weighting + `LpBe2/200` + `HpBe2/0.2` | the ~4 Hz-peaked standard weighting curve, 200 Hz bandwidth |
| `Unweighted` | `process_unweighted` | `BpBe4/0.3-200` | unweighted 0.3–200 Hz band |
| `Wow` | `process_wow` | `BpBe4/0.3-6` | wow band only (diagnostic) |
| `Flutter` | `process_flutter` | `BpBe4/6-200` | flutter band only (diagnostic) |

These match Metzler exactly: a weighting curve peaking near 4 Hz, a 200 Hz
post-discriminator bandwidth, and the 6 Hz wow/flutter split. `Wow` and `Flutter`
are **not** standard readings — they are band-split diagnostics for locating a
fault.

**The weighting filters run at the zero-crossing rate**, i.e. 2 × carrier =
**6300 Hz at 3150 Hz**, which is exactly the design rate stated in `filters.c`
("Sampling rate is 6300Hz"). At the 3000 Hz setting the rate is 6000 Hz, so the
whole weighting curve is scaled by 3000/3150 (−4.8%). **3150 Hz is the setting
the filters were designed for.** The effect is small (Metzler: the two tones give
the same numbers in practice) but 3150 Hz is technically the more correct choice.

## Detectors

WFGUI runs **both detectors simultaneously** on whichever weighting is selected —
unlike a hardware meter there is no DIN/JIS "indication" switch (post #82 makes
the same observation).

### Quasi-peak (the needle, and the "quasi-peak" figure)

`wfguiDlg.cpp:820-826`:

```c
v = fabs(err) * 10000 / 85;              // empirical calibration
if (v > peak) peak += (v - peak)/500;    // attack
else          peak += (v - peak)/6000;   // decay
max_peak = peak;
```

An asymmetric attack/decay envelope follower. At 6300 crossings/s the time
constants are **attack ≈ 500/6300 ≈ 79 ms**, **decay ≈ 6000/6300 ≈ 0.95 s**.

That ~1 second decay is the classic DIN/IEC quasi-peak ballistic — matching
post #78's "DIN/IEC/CCIR use quasi-peak with 1 second decay". **So the needle is
the DIN-style reading.**

Note the `× 10000 / 85` factor: `err` is a fraction, so this is
`percent × 100/85` ≈ **percent × 1.176**. It is an empirical calibration constant
compensating the weighting filter's gain.

### RMS (the RMS (%) box)

`wfguiDlg.cpp:839, 873, 883-893`. `err²` is accumulated per 0.1 s buffer into
`RMS_sums[]`; every 10 buffers the ten sums are added and

```c
max_RMS[index_100] = sqrt(sum_of_squares1 / good_samples) * 100;
```

So the RMS box is a **linear (not exponential) RMS over exactly the last
1 second, refreshed once per second**. This answers post #57's unanswered
question about step and averaging method.

> **Important asymmetry:** the RMS path has **no** `/85` calibration, while the
> quasi-peak path does. The two readings therefore use different scalings, which
> is part of why post #68's attempts to relate peak and RMS by a clean √2 never
> worked out.

### "Max in 10 sec"

`wfguiDlg.cpp:876-904`. Two 100-slot ring buffers advanced once per 0.1 s buffer
⇒ a **10-second** window. Every second both are scanned for their maximum and the
status line is formatted as:

```
Max in 10 sec %.4f RMS %.4f quasi-peak
```

i.e. **"Max in 10 sec ⟨max 1-s RMS⟩ RMS ⟨max quasi-peak⟩ quasi-peak"**. Matching
the screenshot in `docs/wf-meter-ui.png`: max RMS 0.1489%, max quasi-peak 0.2724%.

This resolves post #68's "why does one say Peak and the other quasi-peak?" — they
are the **same detector**: the needle shows it live, the status line shows its
maximum over 10 s.

> Post #70's description ("RMS is the last peak RMS over the last 20 seconds")
> is **not** what the code does: the window is 10 s, and the RMS box itself is a
> 1-second RMS, not a held maximum.

## Frequency / speed readout

`wfguiDlg.cpp:849-850, 911`. `average` = mean half-period over the 1 s block;
`freq = 1e9 / average / 2`, displayed to 0.1 Hz once per second.

Because a reference tape is recorded at exactly nominal, this *is* the speed
error:

```
speed error % = (displayed frequency / nominal − 1) × 100
```

Screenshot example: 3105.4 Hz against 3150 Hz nominal = **−1.42%** (running slow).

## Oscilloscope trace

`wfguiDlg.cpp:794-801`. Accumulates `err × 100` (percent) and plots the mean of
every **7 zero crossings** as one point. Y range follows the Range radio; X axis
is labelled in samples. It shows the *weighted* instantaneous deviation, so its
character changes with the combo selection.

## Range radios

`wfguiDlg.cpp:415-460` — **0.1 / 0.4 / 1 / 4 %** full scale, setting both the
scope Y range and the needle range. Default is **0.4%**
(`wfguiDlg.cpp:245`). Purely a display scaling; it does not affect measurement.

## Output files

Both are opened at Start with **relative paths** — i.e. in the process's current
working directory — and are **truncated on every run**
(`wfguiDlg.cpp:529-533`):

| File | Written | Contents |
|---|---|---|
| `WF_out.dat` | per zero crossing | `(short)(10 * (proper_interval - interval))`, signed 16-bit LE |
| `log.txt` | once per second | `"%.1f %.4f %.4f\n"` — frequency Hz, 1-second RMS %, max quasi-peak over last 10 s % |

**`WF_out.dat` is written before the weighting filter is applied**
(`wfguiDlg.cpp:772-774` precedes `:780`) — it is the **raw, unweighted
discriminator output**. That is what makes it valid for the FFT part-identification
technique regardless of which weighting is selected in the UI.

Its sample rate is **one sample per zero crossing = 2 × carrier**: 6000 Hz for a
3000 Hz tone, 6300 Hz for 3150 Hz. This confirms from source what the thread
worked out empirically over four years (posts #9–#12, #58–#65); n3mmr's
conclusion in #64 ("sample rate is twice the actual centre frequency") is
correct, and Matcs' original 8571 Hz guess in #9 was retracted in #12.

## Source-confirmed bugs behind long-standing thread complaints

These are worth knowing because they explain reported behaviour that otherwise
looks like user error.

### 1. Wrong input device gets opened — combo index used as device ID

`FillDevices()` adds device names in device-ID order (`wfguiDlg.cpp:550-555`),
but `IDC_DEVICES` carries the **`CBS_SORT`** style (`wfgui.rc:111`), so the combo
re-sorts them alphabetically. `OpenDevice()` then passes
**`pDevices->GetCurSel()`** — the *combo index* — straight to `waveInOpen()` as
the *device ID* (`wfguiDlg.cpp:627-628`).

Whenever alphabetical order differs from device-ID order, **WFGUI opens a
different device than the one shown as selected.** This is exactly post #54
("continues to only receive the signal from my built-in soundcard despite what is
selected") and post #96 ("switch WFGUI to microphone array and it all of a sudden
works, despite the signal being fed to the Scarlett").

*Practical workaround for the README:* if you get "Signal too low" on the obviously
correct entry, try the other entries — one of them will be the device you want.

### 2. Device dropdown shows only one line

`wfgui.rc:111` declares `COMBOBOX IDC_DEVICES,6,7,90,30`. In a dialog template
a combo box's height field is the height **including its dropped-down list**;
30 DLU leaves room for roughly one item. (`IDC_FILTER_TYPE` is declared with 74
and behaves normally — `wfgui.rc:109`.)

Explains posts #17, #31, #36, #60 and the mouse-wheel / arrow-key workarounds
users discovered.

### 3. Weighting cannot be changed without restarting

`OnButton2()` (Start) disables the combo (`wfguiDlg.cpp:498`) and **`OnButton1()`
(Stop) never re-enables it**. Confirms posts #96 and #98 — a restart really is
required. A genuine bug, one line to fix.

### 4. "Save log" checkbox does nothing in this source revision

`m_log` is declared (`wfguiDlg.h:112`) and initialised to `false`
(`wfguiDlg.cpp:115`), and the logging code is present and correct
(`:532-533, :914-915`). But `IDC_CHECK1` has **no `DDX_Check` binding and no
message-map handler** — verified by grep. `m_log` therefore stays `false` forever
and **`log.txt` is never created** by a build of this tree.

> This is a discrepancy between *this repository* and the **released v8 binary**,
> where logging demonstrably works (thread posts #39, #44, #53, #66 show real
> log files). The About box here reads "wfgui Version 3.0, Copyright Alex Freed
> 2010-2014" (`wfgui.rc:82-83`) — see `04-open-questions.md`. Documentation must
> not promise logging works in a self-built binary without this being fixed.

### 5. "NO Input Devices Found.." closes the app

`wfguiDlg.cpp:218-222`: if `waveInGetNumDevs()` returns 0 the message box is shown
and the dialog immediately `OnOK()`s — the app exits. Since many laptop inputs only
appear once a jack is physically inserted, plugging the cable in *before* launching
fixes it. Confirms posts #87/#88 (self-solved), #90–#93.

## Build

`.github/workflows/build-windows.yml`: MSBuild `wfgui.vcxproj`,
**Release|Win32**, toolset v143, with the VC++ ATL/MFC component. Artefact
`wfgui-<sha>.exe`; tagged pushes (`v*`) publish a GitHub Release. MFC and the CRT
are statically linked (commit `faf446a`) so the exe is self-contained.

Prebuilt originals are in the repo: `wfgui_4_60.zip`, `wfgui_7_60.zip`,
`wfgui_8_60.zip` (each one exe, dated 2014).
