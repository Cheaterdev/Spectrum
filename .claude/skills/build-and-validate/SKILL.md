---
name: build-and-validate
description: Build Spectrum and run it to capture real D3D12 debug-layer output, turning "this should work" into a verified result. Use this skill whenever a change needs to be proven — after editing HAL, FrameGraph, barrier, resource-state, or render pass code, when asked to build, compile, run, or test the engine, when checking whether a fix actually removed validation errors, or when reporting how many D3D12 errors a change fixed or introduced. Prefer this over reasoning about whether code compiles or whether barriers are correct.
---

# Build and validate

For barrier, resource-state, and FrameGraph work the D3D12 debug layer *is*
the test. Reasoning about whether a transition is legal is unreliable; running
the engine and counting what the validation layer says is not. This skill is
the loop that produces a trustworthy number.

## Choosing a configuration

Use **`Debug-D3D12`** for validation work. The plain `Debug` configuration and
the `-Vulkan` variants will not produce D3D12 debug-layer output, so a clean
result from them says nothing about barrier correctness.

Available configurations are `Debug`, `Profile`, `Retail`, each also as
`-D3D12` and `-Vulkan`, all `x64`.

Note that the output path is derived from the *mode* only
(`main.sharpmake.cs:202` uses `target.Mode`), so `Debug-D3D12` and
`Debug-Vulkan` both produce `bin/debug/spectrum.exe`. The binary on disk gives
no indication of which API it was built against, and a leftover Vulkan build
runs perfectly while emitting no D3D12 messages at all. The `D3D12 SETUP` line
described below is the only reliable way to tell them apart.

## Building

Locate MSBuild via vswhere rather than hardcoding a version, since the VS
install path moves between machines and updates:

```bash
MSBUILD=$("/c/Program Files (x86)/Microsoft Visual Studio/Installer/vswhere.exe" -latest -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | head -1)
"$MSBUILD" projects/spectrum.sln /p:Configuration=Debug-D3D12 /p:Platform=x64 /m /nologo /v:minimal
```

Building a single project (`projects/hal/hal.vcxproj`) is much faster and is
the right call when iterating inside one layer. Build the full solution before
reporting a change as done, because C++ module consumers frequently break in
ways a single-project build never surfaces.

Build times are long — allow several minutes and set generous tool timeouts
rather than assuming a hang.

## Running to capture validation output

The engine registers an `ID3D12InfoQueue1` message callback that mirrors
debug-layer messages into the log (`sources/HAL/API/D3D12/HAL.D3D12.Device.cpp`),
so validation output is available from a plain console run without a debugger
attached. The working directory must be `workdir/`, which is where assets,
shaders, and the log live.

Run it in the background and stop it after roughly 15 seconds. That is enough
to get through device creation, asset load, and a number of steady-state
frames, which is where essentially all validation errors appear. Waiting
minutes adds repeated copies of the same messages and nothing new.

```bash
cd workdir && ../bin/Debug/spectrum.exe
```

Stop the process once it has run long enough, then read `workdir/log.txt`.

## Reading the results

Messages are written in a fixed shape, so counting is exact:

```bash
grep -cE "^D3D12 (ERROR|CORRUPTION)" workdir/log.txt
grep -E "^D3D12 (ERROR|CORRUPTION|WARNING)" workdir/log.txt | sed -E 's/^(D3D12 [A-Z]+ #[0-9]+).*/\1/' | sort | uniq -c | sort -rn
```

The second command groups by severity and message ID, which is what makes a
large error count actionable — 85,000 errors is routinely three distinct IDs
repeating per frame, and the ID is what identifies the bug.

## Guarding against a fake clean result

A "zero errors" result is only meaningful if validation was actually running.
Before believing it, confirm all of the following:

- **The callback registered.** `grep "D3D12 SETUP" workdir/log.txt` must show
  `message callback registered`. If it reports `ID3D12InfoQueue1 unavailable`,
  nothing was captured and the run proves nothing.
- **The log is from this run.** The engine appends across runs. Truncate or
  delete `workdir/log.txt` before launching, or the counts mix old and new.
- **The build actually succeeded.** A failed build leaves the previous
  executable in place, so the run silently tests stale code. Check the MSBuild
  exit code, don't just skim the output.
- **The run got past device creation.** If the app crashed or exited in the
  first second, few frames were submitted and most passes never ran.

These are the ways this loop lies. Checking them costs seconds; skipping them
produces a confident and wrong "fixed it."

## Reporting

Give the before and after counts and the distinct message IDs, not a
qualitative summary. "Was 85518, now 0" and "the remaining 12 are all #1334 on
PSSM_Depths" are the useful forms. If errors remain, say so plainly with the
counts rather than describing the change as complete.

To interpret a specific barrier or layout error once you have the ID, use the
`barrier-triage` skill.
