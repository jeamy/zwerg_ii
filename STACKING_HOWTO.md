# Stacking How-To

This document describes the current stacking workflow in `zwergII`.

The implementation now supports:

- Tele live stacking
- Wide live stacking
- dark-frame capture
- EQ solving

Important limitations still apply and are listed below.

## Critical Rule

**Do GOTO before starting stacking.**

If stacking is started without a valid target/GOTO state, the device can reject the request.

Known error:

- `-11513`: GOTO required before stacking

## What the App Currently Does

When you start stacking from the Astro panel, the app:

1. switches the selected stacking camera to manual exposure and gain
2. activates Astro mode via `Go Live`
3. starts Tele or Wide stacking depending on the selected source
4. tracks progress through Astro notifications

This is why stacking temporarily interrupts the normal preview workflow.

## Supported Stacking Modes

### Tele Stacking

- uses the Tele camera
- supports optional dark-frame usage
- is the main deep-sky stacking mode

### Wide Stacking

- uses the Wide camera
- is selectable in the Astro UI via the stacking source selector
- currently starts without dark-frame usage

Important:

- dark-frame capture remains Tele-only
- flat and bias UI fields exist, but are currently not active capture workflows

## Recommended Workflow

### 1. Connect to the Device

- connect to the DWARF II Wi-Fi or reachable network
- open `zwergII`
- connect to the telescope
- wait until the live stream is available

### 2. Calibrate

Recommended before serious astro use:

- open the Astro tab
- start calibration
- wait until calibration completes

Calibration is especially important after moving the telescope.

### 3. Select a Target

- search for an object in the Astro tab
- select it in the star map or search results
- verify that it is above the horizon

### 4. Run GOTO

- start GOTO
- wait until the movement/centering process is finished
- do not start stacking before GOTO has settled

### 5. Choose Stacking Source

In the Astro settings area:

- select `Tele` for normal astro stacking
- select `Wide` for wide-angle stacking

### 6. Set Exposure / Gain / Frame Count

Current configurable values:

- number of frames
- exposure index
- gain index
- optional dark-frame usage for Tele stacking

Practical guidance:

- start with moderate exposure and gain
- use brighter targets first
- increase frame count after the basic workflow is stable

### 7. Start Stacking

Expected UI sequence:

- `Starting...`
- `Preparing camera...` or `Preparing wide camera...`
- `Activating Astro mode...`
- `Starting stacking...`
- then `Capturing...` / `Stacking...`

During stacking:

- the preview can go dark or stop updating normally
- this is expected because the device switches into astro/raw capture behavior

### 8. Monitor Progress

The Astro panel shows:

- progress bar
- elapsed time
- current frame count
- stacked frame count
- rejected frame count

### 9. Stop or Wait for Completion

- use `Stop Stacking` to abort manually
- or wait until the requested frame count completes

After stacking stops:

- the UI returns to idle state
- the preview path may need a moment to recover

### 10. Capture Dark Frames

Dark-frame capture is available from the Astro panel.

Current behavior:

- Tele only
- uses the current astro exposure/gain values
- activates Astro mode first
- stores dark-frame profiles on the device

Recommended workflow:

1. set dark-frame count above zero
2. set Tele stacking exposure/gain
3. start dark capture
4. wait until the profile list is reported as available
5. start Tele stacking with dark frames enabled

### 11. EQ Solving

EQ solving is separate from stacking, but related to astro setup quality.

Current UI support:

- start EQ solve
- stop EQ solve
- show azimuth and altitude error output

This is useful for alignment refinement, not as a replacement for the normal stacking workflow.

## Troubleshooting

### Error: GOTO required

Code:

- `-11513`

Meaning:

- stacking was started without a valid GOTO/target state

Fix:

- stop stacking
- perform GOTO first
- then start stacking again

### Error: Dark frame not found

Typical meaning:

- Tele stacking was started with dark-frame usage enabled, but no matching dark profile exists

Fix:

- capture dark frames first
- then retry stacking

### GOTO fails after stacking was already started

Cause:

- stacking switches the device into astro/raw capture flow
- GOTO and plate-solving related actions are safer before stacking starts

Fix:

- stop stacking
- wait for the app/device preview path to recover
- reconnect only if the device does not return to a usable state

### Preview turns black during stacking

This is expected.

The current client relies on MJPEG for preview, while stacking runs through astro/raw commands. A dark or paused preview during stacking does not automatically mean failure.

### Many frames are rejected

Possible reasons:

- weak target
- poor tracking
- bad seeing
- wind or vibration
- too aggressive exposure/gain settings

Try:

- recalibration
- a brighter target
- lower gain
- shorter exposures

### Wide stacking does not behave like Tele stacking

This can be normal.

Wide stacking is supported in the UI, but the overall astro workflow is still more mature on the Tele side.

## Current Practical Limitations

- GOTO should be completed before stacking
- Tele and Wide stacking share UI concepts, but not all behavior is identical
- dark-frame capture is Tele-only
- flat/bias capture is not currently active
- stacking flow still contains firmware-oriented delays and heuristics
- RTSP is not used; preview is MJPEG-based

## Suggested First Test

For a safe first test:

- calibrate
- GOTO a bright target
- choose `Tele`
- use a moderate frame count
- keep exposure and gain conservative
- run one short stacking session first

After that:

- try Tele stacking with dark frames
- then try Wide stacking separately

## Summary

Current best practice:

`Connect -> Calibrate -> GOTO -> Select Tele/Wide -> Set exposure/gain -> Start stacking`

For Tele stacking with darks:

`Capture dark frames first -> enable dark-frame usage -> start Tele stacking`
