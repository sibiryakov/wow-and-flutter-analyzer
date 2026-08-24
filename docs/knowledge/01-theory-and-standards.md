# Knowledge: W&F theory and measurement standards

Sources: Bob Metzler, *Audio Measurement Handbook*, "Wow and Flutter Measurements",
pp. 46–50 (PDF in repo root); tapeheads thread posts as cited.

## What wow and flutter is

Short-term speed variation of an analog transport shows up as a momentary pitch
change — i.e. **wow and flutter is frequency modulation (FM)** of whatever is
recorded (Metzler p.46). Causes are mechanical: eccentric or worn capstans,
idlers, pulleys, belts, plus short-term motor speed variation.

- **Wow** = speed variation below ~6 Hz — large-circumference rotating parts.
- **Flutter** = speed variation above ~6 Hz — small-circumference parts.
- A part rotating *n* times per second produces a component at *n* Hz. W&F
  component frequencies scale with tape speed.

This 6 Hz split is exactly the split WFGUI implements (`Wow` = 0.3–6 Hz,
`Flutter` = 6–200 Hz — see `02-implementation.md`).

## How it is measured

Metzler's block diagram (Fig. 33): **discriminator → weighting filter →
detector & meter**. Play a recorded pure tone; an FM discriminator converts
frequency deviation into a proportional amplitude, which is then filtered and
metered. WFGUI is a software implementation of exactly this chain.

**Definition of the percentage.** Peak W&F in percent = the peak frequency
deviation expressed as a percentage of the carrier frequency (Metzler p.46).
Metzler's worked example: *0.1% with a 3 kHz test tone = 3 Hz peak deviation.*

> This is a **centre-to-peak** definition. It matters — see the ambiguity note
> below.

Test tones are 3.00 kHz or 3.15 kHz. Metzler is explicit that the choice is
**not normally critical**: "in practice, the same percentage numbers will be
obtained at either 3 kHz or 3.15 kHz."

## The standards

Metzler's table (p.47) — the standards differ only in **detector and time
constant**, not in the weighting curve:

| Standard | Test tone | Detector | Time constant |
|---|---|---|---|
| IEC/DIN | 3.15 kHz | peak | normal |
| NAB | 3.0 kHz | rms | normal |
| JIS | 3.0 kHz | rms | slow |

- **IEC/DIN** uses a peak detector → always gives the **highest** number.
- **NAB** and **JIS** are rms-calibrated → **lower** numbers.
- **JIS** has very long post-detector time constants, averaging out short-term
  flutter → lowest numbers of all. Commonly labelled **WRMS**.

The **weighting filter is the same in all three standards** (Metzler p.48). Its
response peaks at about **4 Hz** and rolls off either side, following the
perceived annoyance of W&F to a listener. All three use a post-discriminator
bandwidth of **200 Hz**.

Practical consequence, confirmed repeatedly in the thread: for the same machine,
**DIN (weighted peak) reads roughly 2–3× the WRMS/JIS (weighted rms) figure.**
A.N.T.'s own Sony TC-D5 ProII plots (post #1) show DIN ≈ 0.05% against WRMS
≈ 0.023% — a ratio of ~2.2.

Nakdoc (post #76) on why this matters commercially: JIS weighting flattered
single-capstan decks, so Japanese makers quoted it; DIN weights audible flutter
harder and yields "worse" numbers. **Always check which standard a
manufacturer's spec is quoted in before comparing.**

## The peak-to-peak vs centre-to-peak ambiguity (unresolved in the literature)

This caused the longest confusion in the thread (posts #68–#71, #78) and the
sources genuinely disagree:

- **Metzler (p.46):** 0.1% at 3 kHz = **3 Hz peak** deviation → 1% = ±30 Hz.
- **R. Ockleshaw, *Wireless World* 1971** (quoted in post #68): "a 30-Hz
  deviation from 3 kHz — 15 Hz either side — is a peak-to-peak deviation of 1%"
  → 1% = ±15 Hz.

These differ by a factor of two. Hardware meters also differ: the Leader LFM-39A
documents CCIR/DIN readings as **peak-to-peak**, while Audio Precision's ATS-1
and Virtins measure **centre-to-peak**.

**What WFGUI does:** it computes the deviation of each measured half-period from
nominal and takes the magnitude — a **centre-to-peak** quantity, matching
Metzler and AP, not the Leader. See `02-implementation.md`.

Consequence for the README: state the convention explicitly, and warn that a
manufacturer's "±0.1% DIN" may have been quoted either way.

## "2-sigma" — what manufacturer specs often actually are

Metzler pp.48–49: because the live reading is very jumpy, a "single number" for
a spec sheet is usually the **two-sigma** value, defined *not* statistically but
operationally as **the value exceeded only 5% of the time**. All commercial W&F
meters implement it this way as a running indication.

**WFGUI does not implement 2-sigma.** It offers an instantaneous quasi-peak, a
1-second RMS, and a "max over the last 10 seconds". The 10-second max is a
*maximum*, which will read **higher** than a 2-sigma value over the same period.
This is a real reason WFGUI numbers may not line up with a spec sheet.

## Measurement technique (Metzler pp.47–48) — directly usable as README guidance

1. Use a **standard reference tape** recorded on a machine with very low W&F.
2. Measure at **several points — beginning, middle and end** of the tape. W&F
   varies considerably along a cassette.
3. **Tape speed error is measured at the same time**: because the reference tape
   was recorded at precisely correct speed, any deviation of the average
   reproduced frequency from nominal *is* the machine's speed error.
4. **Never measure in simultaneous record-reproduce (3-head) mode.** The
   record→replay head delay creates a **comb filter** that makes the measurement
   completely blind to flutter at certain frequencies, giving falsely low
   readings. Record, rewind, *then* play back.
5. On very good machines the **reference tape may be the worse of the two**.
   Check by recording your own 3 kHz/3.15 kHz tone on blank tape, rewinding and
   playing it; if that reads lower, use the self-recorded tape.

## Diagnosing *which part* is at fault (Metzler p.50)

FFT the discriminator output. A spectral peak at *f* Hz corresponds to a part
rotating *f* times per second. Metzler's example: a 7.5 Hz peak at 7.5 ips is a
pulley of exactly 1 inch circumference.

circumference = tape speed ÷ peak frequency

For cassette (4.76 cm/s ≈ 1.875 ips), a peak at 5 Hz ⇒ ~0.95 cm circumference.

**This is precisely what WFGUI's `WF_out.dat` file exists for** — it is the raw
discriminator output, ready to be FFT'd. See `03-practical-usage.md`.

## Out of scope: scrape flutter

Metzler p.49: scrape/high-band flutter (tape bending and stretching over heads
and guides) extends to ~5 kHz and requires a **12.5 kHz** test tone and a
recorder good to 17.5 kHz. WFGUI cannot measure it — it is locked to a ~3 kHz
carrier within ±5%.
