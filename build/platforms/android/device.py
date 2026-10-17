"""Finding the phone: shared by run.py and debug.py.

With exactly one phone connected nothing is shown and it is just used. With none, several, or when asked
(`pick=True`), a small window lists the phones and lets you connect (ip:port) or pair (ip:port + code)
one, so wireless debugging needs no typing in a terminal. A paired phone that advertises itself over
mDNS is connected automatically before the window is considered.

The choice is pinned in ANDROID_SERIAL for this process and its children (adb, lldb honour it).
"""
import json
import os
import re
import subprocess
import sys
import threading
import time
from pathlib import Path

SDK = next(Path(p) for p in (os.environ.get("ANDROID_HOME"), os.environ.get("ANDROID_SDK_ROOT"),
                             os.path.join(os.environ.get("LOCALAPPDATA", ""), "Android", "Sdk"))
           if p and Path(p).is_dir())
ADB = str(SDK / "platform-tools" / "adb.exe")
LAST = Path(__file__).resolve().parents[3] / "projects" / "android" / "last_device.json"


def _adb(*args, serial=None, timeout=20):
    cmd = [ADB] + (["-s", serial] if serial else []) + list(args)
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, stdin=subprocess.DEVNULL, timeout=timeout,
                           creationflags=subprocess.CREATE_NO_WINDOW)
        return (r.stdout + r.stderr).strip()
    except subprocess.TimeoutExpired:
        return "timed out"


class Phone:
    def __init__(self, serial, model, hwid):
        self.serial, self.model, self.hwid = serial, model, hwid   # hwid: ro.serialno, the same for every entry of one phone

    def __str__(self):
        return f"{self.model or 'device'}   ({self.serial})"


def connected_phones():
    """One Phone per physical device in the `device` state (a phone can have several adb entries:
    an mDNS name, ip:port, a stale one)."""
    phones, seen = [], set()
    for line in _adb("devices", "-l").splitlines()[1:]:
        m = re.match(r"(\S.*?)\s+device\b(.*)", line)
        if not m:
            continue
        serial, rest = m.group(1), m.group(2)
        model = (re.search(r"model:(\S+)", rest) or [None, ""])[1]
        hwid = _adb("shell", "getprop", "ro.serialno", serial=serial, timeout=8) or serial
        if hwid in seen:
            continue
        seen.add(hwid)
        phones.append(Phone(serial, model, hwid))
    return phones


def mdns_endpoints(text):
    """ip:port of every connectable wireless-debugging service in `adb mdns services` output."""
    return re.findall(r"_adb-tls-connect\._tcp\.?\s+(\d{1,3}(?:\.\d{1,3}){3}:\d+)", text)


def _remember(phone):
    try:
        LAST.write_text(json.dumps({"hwid": phone.hwid}))
    except OSError:
        pass


def _last_hwid():
    try:
        return json.loads(LAST.read_text()).get("hwid")
    except (OSError, ValueError):
        return None


def _use(phone, say):
    os.environ["ANDROID_SERIAL"] = phone.serial
    _remember(phone)
    say(f"using {phone}")


def ensure_device(say=print, pick=False):
    """Pins ANDROID_SERIAL to the phone to use, asking in a window when it is not obvious. Exits with
    a message if there is none."""
    phones = connected_phones()
    if not phones:
        say("no device connected; looking for a paired phone over mDNS...")
        for endpoint in mdns_endpoints(_adb("mdns", "services")):
            say(f"  adb connect {endpoint}: " + _adb("connect", endpoint))
        phones = connected_phones()

    if len(phones) == 1 and not pick:
        _use(phones[0], say)
        return
    if os.environ.get("ANDROID_SERIAL") and not pick:
        chosen = [p for p in phones if p.serial == os.environ["ANDROID_SERIAL"]]
        if chosen:
            _use(chosen[0], say)
            return

    chosen = choose_in_window(phones)
    if not chosen:
        sys.exit("no device selected")
    _use(chosen, say)


def install_apk(apk, say=print):
    """adb install -r; if the installed app was signed with another debug key, uninstall it (its data goes
    too) and install again. True on success."""
    out = _adb("install", "-r", str(apk), timeout=300)
    if "INSTALL_FAILED_UPDATE_INCOMPATIBLE" in out:
        # The debug keystore (projects/android/debug.keystore) is generated per checkout/clean tree, so an
        # app installed from another one is signed differently and cannot be updated in place.
        say("installed app was signed with a different key; uninstalling it (its data goes too) and retrying")
        _adb("uninstall", "com.spectrum")
        out = _adb("install", str(apk), timeout=300)
    say(out)
    return "Success" in out


def choose_in_window(phones):
    import tkinter as tk
    from tkinter import ttk

    root = tk.Tk()
    root.title("Spectrum: choose the phone")
    root.geometry("520x430")
    root.attributes("-topmost", True)   # F5 starts this behind the IDE otherwise
    result = {"phone": None}
    state = {"phones": list(phones)}

    frame = ttk.Frame(root, padding=10)
    frame.pack(fill="both", expand=True)

    ttk.Label(frame, text="Connected phones", font=("Segoe UI", 10, "bold")).pack(anchor="w")
    listbox = tk.Listbox(frame, height=5, activestyle="dotbox", exportselection=False)
    listbox.pack(fill="x", pady=(2, 6))
    status = tk.StringVar(value="Select a phone, or connect one below.")

    def fill():
        listbox.delete(0, "end")
        for p in state["phones"]:
            listbox.insert("end", str(p))
        last = _last_hwid()
        for i, p in enumerate(state["phones"]):
            if p.hwid == last or len(state["phones"]) == 1:
                listbox.selection_set(i)
                break

    def refresh(connect_mdns=True):
        def work():
            if connect_mdns:
                for ep in mdns_endpoints(_adb("mdns", "services")):
                    _adb("connect", ep)
            phones_now = connected_phones()
            root.after(0, lambda: (state.update(phones=phones_now), fill(),
                                   status.set(f"{len(phones_now)} phone(s) connected" if phones_now else "No phone connected yet.")))
        status.set("Looking for phones...")
        threading.Thread(target=work, daemon=True).start()

    def use_selected(_event=None):
        sel = listbox.curselection()
        if not sel:
            status.set("Select a phone first.")
            return
        result["phone"] = state["phones"][sel[0]]
        root.destroy()

    row = ttk.Frame(frame)
    row.pack(fill="x")
    ttk.Button(row, text="Use selected", command=use_selected).pack(side="left")
    ttk.Button(row, text="Refresh", command=refresh).pack(side="left", padx=6)
    listbox.bind("<Double-Button-1>", use_selected)
    root.bind("<Return>", use_selected)

    def labelled_entry(parent, text, width, default=""):
        ttk.Label(parent, text=text).pack(side="left")
        var = tk.StringVar(value=default)
        ttk.Entry(parent, textvariable=var, width=width).pack(side="left", padx=(4, 8))
        return var

    def run_adb_button(args, done_text):
        def work():
            out = _adb(*args)
            root.after(0, lambda: (status.set(out.splitlines()[-1] if out else done_text), refresh(False)))
        status.set("Working...")
        threading.Thread(target=work, daemon=True).start()

    ttk.Separator(frame).pack(fill="x", pady=10)
    ttk.Label(frame, text="Connect (Wireless debugging screen: IP address & port)", font=("Segoe UI", 10, "bold")).pack(anchor="w")
    crow = ttk.Frame(frame)
    crow.pack(fill="x", pady=4)
    endpoint = labelled_entry(crow, "ip:port", 22)
    ttk.Button(crow, text="Connect", command=lambda: run_adb_button(["connect", endpoint.get().strip()], "connected")).pack(side="left")

    ttk.Separator(frame).pack(fill="x", pady=10)
    ttk.Label(frame, text="Pair (once per PC: 'Pair device with pairing code' on the phone)", font=("Segoe UI", 10, "bold")).pack(anchor="w")
    prow = ttk.Frame(frame)
    prow.pack(fill="x", pady=4)
    pair_endpoint = labelled_entry(prow, "ip:port", 22)
    pair_code = labelled_entry(prow, "code", 8)
    ttk.Button(prow, text="Pair", command=lambda: run_adb_button(
        ["pair", pair_endpoint.get().strip(), pair_code.get().strip()], "paired")).pack(side="left")

    ttk.Label(frame, textvariable=status, wraplength=480, foreground="#555").pack(anchor="w", pady=(12, 0))

    fill()
    if not state["phones"]:
        refresh()
    root.mainloop()
    return result["phone"]
