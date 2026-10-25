---
name: codacy-export
description: Export all Codacy static-analysis issues for the Spectrum repo (Cheaterdev/Spectrum) to CSV/JSON and summarize them. Use when asked to look at, triage, list or fix Codacy issues or the Codacy dashboard, instead of clicking through app.codacy.com.
---

# Codacy issue export

The repo is public on Codacy, so the v3 API answers without a token. Never ask for or enter an API token.

1. Run the bundled script, writing to the scratchpad (not the repo):
   `python -I .claude/skills/codacy-export/export.py <out_dir> [--category UnusedCode] [--tool <toolUuid>]`
   It pages through `POST /api/v3/analysis/organizations/gh/Cheaterdev/repositories/Spectrum/issues/search?limit=100`
   (cursor pagination) and writes `codacy_issues.csv` (category, level, tool, pattern, file, line, message) and
   `codacy_issues.json` (full records with source line and commit).
2. Summarize by pattern, tool and category (`collections.Counter` over the CSV). Compare the total with the dashboard
   (`/dashboard` shows the total issue count) to confirm nothing was cut off.
3. Triage: separate real problems from noise before fixing anything. Usual noise: `autogen/` folders (Prism output),
   `Modules/` wrappers, C++ module files that Cppcheck cannot parse, `*.sharpmake.cs` style rules (SonarC#), Python
   docstring rules on `build/` scripts.

The dashboard's tool UUIDs are in the JSON (`toolInfo.uuid`). Other useful endpoints live under the same
`/api/v3/analysis/organizations/gh/Cheaterdev/repositories/Spectrum/` prefix (e.g. `GET .../` for repo analysis status).
