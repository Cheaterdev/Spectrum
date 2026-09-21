---
name: sig-regen
description: Regenerate C++ and HLSL binding code after editing any .sig file in sources/SIGParser/sigs/. Use this skill whenever you add, edit, or remove a struct, ComputePSO, GraphicsPSO, PassNode, or Pipeline entry in a .sig file, or whenever generated code under sources/HAL/autogen, sources/RenderSystem/FrameGraph/autogen, or workdir/shaders/autogen looks stale or out of sync with the .sig sources. Also use it when a build fails with unknown Slots::, PSOS::, or Passes:: identifiers, since that almost always means the .sig edit was never regenerated.
---

# Regenerating SIG code

`.sig` files are the single source of truth for GPU/CPU shared structs, PSO
definitions, and FrameGraph pass declarations. Editing one changes nothing on
its own — the generated C++ and HLSL must be rebuilt from it.

## The one thing that goes wrong

`generate_sigs.bat` does **not** regenerate code from `.sig` files. It runs
ANTLR over `SIG.g4` to rebuild the *parser grammar*, which is only needed when
the grammar itself changes. Running it after a `.sig` edit appears to succeed
and produces no useful change — which is exactly why it's the trap.

The actual generator is `bin/debug/sigparser.exe`.

## Running the generator

The generator resolves every path relative to its working directory —
`sigs/` for input, `../../sources/...` and `../../workdir/...` for output (see
`sources/SIGParser/Main.cpp:9-13`). Run it anywhere else and it either finds no
input or writes to the wrong place, so the working directory is not incidental:

```bash
cd sources/SIGParser && ../../bin/debug/sigparser.exe
```

If `bin/debug/sigparser.exe` is missing or older than the `.sig` edits and the
grammar or generator sources changed, build the `sigparser` project first —
otherwise you are regenerating with a stale generator and the output won't
reflect template changes.

## Deciding whether the project needs regenerating too

Sharpmake enumerates source files at generation time, so the `.vcxproj` files
list generated sources explicitly. Modifying the *contents* of an existing
generated file is invisible to the build system, but a **new** generated file
will not compile until the projects know about it.

After running the generator, check whether the file set changed:

```bash
git status --short sources/HAL/autogen sources/RenderSystem/FrameGraph/autogen workdir/shaders/autogen
```

Lines starting with `??` (untracked) or `D` (deleted) mean the file set changed
— run `generate_project.bat`. Only `M` lines means contents changed in place and
the existing projects already cover it.

Adding a new `struct`, `ComputePSO`/`GraphicsPSO`, or `PassNode` usually creates
new files, so a new declaration generally does need the project regenerated.

## Full sequence

1. Edit the `.sig` file under `sources/SIGParser/sigs/`.
2. `cd sources/SIGParser && ../../bin/debug/sigparser.exe`
3. `git status --short` the three autogen directories.
4. If any file was added or removed, run `generate_project.bat` from the repo root.
5. Build, and confirm the new `Slots::`/`PSOS::`/`Passes::` names resolve.

## Reporting back

Say which `.sig` files changed, what the generator wrote, and — explicitly —
whether `generate_project.bat` was needed. That last point is what the next
person (or the next session) needs in order to trust the result, since a
missing project regeneration produces a confusing "identifier not found" error
far away from its cause.

Never hand-edit files under the autogen directories. They carry a
DO-NOT-EDIT banner and the next generator run silently discards the changes.
If generated output is wrong, fix the `.sig` file or the Jinja template in
`sources/SIGParser/templates/`.
