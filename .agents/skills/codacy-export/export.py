"""Export all Codacy issues of Cheaterdev/Spectrum to CSV + JSON (public repo, no token needed).

Usage: python -I export.py [out_dir] [--category UnusedCode] [--tool Cppcheck]
"""
import csv
import json
import sys
import urllib.request

BASE = "https://app.codacy.com/api/v3/analysis/organizations/gh/Cheaterdev/repositories/Spectrum/issues/search"


def main():
    args = sys.argv[1:]
    opts = {}
    while "--category" in args or "--tool" in args:
        for flag, key in (("--category", "categories"), ("--tool", "toolUuids")):
            if flag in args:
                i = args.index(flag)
                opts[key] = [args[i + 1]]
                del args[i:i + 2]
    out_dir = args[0] if args else "."

    body = json.dumps(opts).encode()
    issues, cursor = [], None
    while True:
        url = BASE + "?limit=100" + (f"&cursor={cursor}" if cursor else "")
        req = urllib.request.Request(url, body, {"content-type": "application/json"})
        page = json.load(urllib.request.urlopen(req))
        issues += page["data"]
        cursor = page.get("pagination", {}).get("cursor")
        if not cursor:
            break

    with open(f"{out_dir}/codacy_issues.csv", "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["category", "level", "tool", "pattern", "file", "line", "message"])
        for i in issues:
            p = i["patternInfo"]
            w.writerow([p["category"], p["severityLevel"], i["toolInfo"]["name"], p["id"],
                        i["filePath"], i["lineNumber"], i["message"]])
    with open(f"{out_dir}/codacy_issues.json", "w", encoding="utf-8") as f:
        json.dump(issues, f)
    print(f"{len(issues)} issues -> {out_dir}")


main()
