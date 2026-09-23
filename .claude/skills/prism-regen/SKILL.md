---
name: prism-regen
description: Regenerate C++ and HLSL code after editing any .prism file in sources/Prism/defs/. Use this skill whenever you add, edit, or remove a struct, ComputePSO, GraphicsPSO, PassNode, Pipeline entry or HLSL function in a .prism file, or whenever generated code under sources/HAL/autogen, sources/RenderSystem/FrameGraph/autogen, or workdir/shaders/autogen looks stale or out of sync with the .prism sources. Also use it when a build fails with unknown Slots::, PSOS::, or Passes:: identifiers, since that almost always means the .prism edit was never regenerated.
---

# Regenerating Prism code

`.prism` files are the single source of truth for GPU/CPU shared structs, PSO
definitions, and FrameGraph pass declarations. Editing one changes nothing on
its own — the generated C++ and HLSL must be rebuilt from it.

## The one thing that goes wrong

`generate_prism_parser.bat` does **not** regenerate code from `.prism` files. It
runs ANTLR over `Prism.g4` to rebuild the *parser*, which is only needed when
the grammar itself changes. Running it after a `.prism` edit appears to succeed
and produces no useful change — which is exactly why it's the trap.

The actual generator is `prismc.exe`, built by the `Prism` project.

## Running the generator

In Visual Studio with the Prism extension installed: **Tools → Regenerate Prism
code** (also on the Prism toolbar). It saves open `.prism` files, runs the
generator, and reports added/removed/modified files in the Output window's
"Prism" pane — including whether `generate_project.bat` is needed.

From a shell, the working directory matters: the generator reads `defs/` and
writes to `../../sources/...` and `../../workdir/...` relative to it.

```bash
cd sources/Prism && ../../bin/profile/prismc.exe
```

A `.prism` mistake makes the generator print `file(line,col): error: ...`, exit
with code 1 and write **nothing**; fix the reported lines and rerun.

Prebuilt exes in different `bin/` configurations drift independently. If the
generator or templates changed since `bin/profile/prismc.exe` was built, rebuild
the `Prism` project (Profile) first — otherwise the output won't reflect those
changes.

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

1. Edit the `.prism` file under `sources/Prism/defs/`.
2. Regenerate (Tools → Regenerate Prism code, or `cd sources/Prism && ../../bin/profile/prismc.exe`).
3. `git status --short` the three autogen directories.
4. If any file was added or removed, run `generate_project.bat` from the repo root.
5. Build, and confirm the new `Slots::`/`PSOS::`/`Passes::` names resolve.

## Reporting back

Say which `.prism` files changed, what the generator wrote, and — explicitly —
whether `generate_project.bat` was needed. That last point is what the next
person (or the next session) needs in order to trust the result, since a
missing project regeneration produces a confusing "identifier not found" error
far away from its cause.

Never hand-edit files under the autogen directories. They carry a
DO-NOT-EDIT banner and the next generator run silently discards the changes.
If generated output is wrong, fix the `.prism` file or the Jinja template in
`sources/Prism/templates/`.
