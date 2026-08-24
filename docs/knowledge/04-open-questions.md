# Knowledge: open questions, discrepancies and things I will NOT assert

Written so the README does not state anything I cannot back up. **Please review
this file in particular** — several items need your decision.

## 1. This source tree is not the released v8

The About box reads **"wfgui Version 3.0"**, "Copyright (C) Alex Freed
2010-2014" (`wfgui.rc:82-83`), and the version resource says `3, 0, 0, 1`
(`wfgui.rc:149`). The repo also ships prebuilt `wfgui_4_60.zip`, `wfgui_7_60.zip`
and `wfgui_8_60.zip`, all dated 2014.

But the source *does* contain v8-era features (logging code, 3000/3150 selection,
4% range, "Max in 10 sec"). So the version string is most likely just stale.

**I cannot establish which released version this tree corresponds to.** It has at
least one feature gap versus the released v8 (logging is not wired up, §3 below).

**Decision needed:** should the README
(a) describe the code as it is in this repo,
(b) describe the released v8 binary that the forum documents, or
(c) describe the repo and flag the differences?
I would recommend **(c)**, and have written the knowledge files that way.

## 2. The peak percentage convention — genuinely contested

Metzler says 0.1% at 3 kHz = 3 Hz **peak** deviation (centre-to-peak).
*Wireless World* (1971) says 1% = 30 Hz **peak-to-peak** (±15 Hz). Leader's
LFM-39A documents CCIR/DIN as peak-to-peak; Audio Precision and Virtins use
centre-to-peak. These differ by 2×.

The code takes `fabs()` of a deviation from nominal, which is centre-to-peak.
**But** the needle also carries an empirical `× 100/85 ≈ 1.176` calibration
factor whose derivation is not documented anywhere in the source. So I can say
what the code computes, but **I cannot certify that WFGUI's absolute numbers
match any particular standard's definition.**

The strongest available evidence is empirical, from post #70: readings "match
well" with a National VP-7750A and a Leader LFM-3610, and a serviced Nakamichi
Dragon reads at its 0.018% spec. I will cite that as corroboration, attributed,
rather than claiming certified conformance.

## 3. Logging is dead code in this tree

`m_log` has no DDX binding and no message-map handler, so it is permanently
`false` and `log.txt` is never created — verified by grep over `wfguiDlg.cpp`,
`wfguiDlg.h`, `wfgui.rc` and `resource.h`.

Yet the forum shows many users producing real log files from the released v8.

**Decision needed.** Options:
- **(a) Fix it** — a one-line `DDX_Check(pDX, IDC_CHECK1, m_log);` plus making
  `m_log` a `BOOL`. Then document logging normally. *(My recommendation — it is
  small, and logging is the feature the thread values most.)*
- **(b) Document it as a known limitation** of builds from this repo.
- **(c) Leave it undocumented** — I do not recommend this; users will tick the
  box and get nothing, exactly as in posts #38–#53.

Note this is a **code change**, which is outside the documentation task you set —
so I have not touched it. Same for the three other confirmed bugs in
`02-implementation.md` (device index/`CBS_SORT`, dropdown height, weighting
combo not re-enabled). Say the word if you want any of them fixed in this branch
or a separate one.

## 4. `process_2nd_order` runs at a rate it was not designed for

`filters.c` documents it as designed for a **6300 Hz** sampling rate with a
1 Hz–3150 Hz passband, but `wfguiDlg.cpp:738` applies it to the **44100 Hz**
input stream. Its corner frequencies therefore scale by 7×, making it
behave roughly as a high-pass around 7 Hz rather than the documented bandpass.

The *weighting* filters (`process_DIN` etc.) are fine — they run per zero
crossing at 6300 Hz, exactly as designed.

Functionally this is probably harmless (its job before zero-crossing detection is
DC/rumble blocking), and I have no way to test it here. **I will not mention this
in the README** — it is a code observation, not user-facing. Flagging it only in
case it interests you.

## 5. Things the thread asserts that the source contradicts

I will follow the **source**, not the thread, on these:

| Thread claim | Source says |
|---|---|
| Post #70: "RMS is the last peak RMS reading over the last 20 seconds" | Window is **10 s**; the RMS box is a **1-second RMS**, not a held maximum |
| Post #9: `WF_out.dat` sample rate is 8571 Hz | 2 × carrier (6000/6300 Hz). Retracted by its author in post #12 |
| Post #62: the .dat is "cumulative offset (delta from the previous value)" | It is per-crossing deviation from nominal, not a running delta |

## 6. Questions I could not answer from any source

- **Minimum measurement duration** for a valid figure (asked in post #67, never
  answered). Metzler's Fig. 35 uses a 30-second window and WFGUI's own longest
  integration is 10 s, so "at least 10–30 s, and ideally a full tape pass" is a
  defensible suggestion — but it is **my inference, not a sourced fact**. Tell me
  if you would rather I leave it out or phrase it as a rule of thumb.
- **What the `/85` and `/93` empirical calibration constants were derived from.**
- **Whether the 3150 Hz filter-design advantage is measurable in practice.**
  Metzler says the two tones give the same numbers; I will present 3150 Hz as
  "technically the design centre" without claiming a measurable difference.
- **Licence.** There is **no licence file in the repo.** Alex Freed released the
  binary as free/donationware (post #1), and post #37 reports he invited others
  to continue it, but that is not a licence grant. I will not state a licence in
  the README. **You may want to add one / clarify provenance.**

## 7. Scope check on the README

Your brief: concise; quick start first; requirements; then gradually deeper into
features and advanced usage; include measurement setup, explain what W&F means,
and how to obtain results per the different standards; audience is people
measuring tape recorders against manufacturer specs.

Planned structure:

1. What this is (one paragraph) + the UI screenshot
2. **Quick start** — cable, test tape, five steps to a number
3. What you need (hardware/software table)
4. What wow and flutter is (short — FM of a test tone; wow vs flutter)
5. Reading the display
6. **Matching manufacturer specs** — the standards table and which number to read
7. Measurement technique that makes numbers trustworthy (Metzler's rules,
   noise-floor check)
8. Advanced: logging along the tape; `WF_out.dat` + FFT to find the guilty part
9. Troubleshooting
10. Building from source; credits

Two things I would like to include but that add length — tell me to cut them if
you want it tighter:
- the **noise-floor loopback check** (§"Establishing your noise floor")
- the **peak-to-peak vs centre-to-peak** caveat

Images available: `docs/wf-meter-ui.png` (annotated UI reference — good for §5)
and `docs/IMG_3288.jpeg` (the RCA→3.5 mm cable — good for §2/§3).
