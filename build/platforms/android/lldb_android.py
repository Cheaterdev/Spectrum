"""lldb helpers for debugging Spectrum on Android (loaded by debug.py).

ART (the Java runtime in the app) raises SIGSEGV on purpose for implicit null checks and stack-overflow
probes, and a few other signals are routine, so a debugger stopping on every one is unusable. The stop
hook resumes a signal stop unless a frame of our own library is on the faulting thread's stack, which is
what a crash in the engine looks like; breakpoints and steps are never touched.
"""
import lldb

OURS = "libspectrum.so"


class IgnoreForeignSignals:
    def __init__(self, target, extra_args, internal_dict):
        self.target = target

    def handle_stop(self, exe_ctx, stream):
        thread = exe_ctx.GetThread()
        if not thread.IsValid():
            return True
        reason = thread.GetStopReason()
        if reason == lldb.eStopReasonBreakpoint:
            # lldb's own breakpoints (ART's JIT debug-registration hook) have negative ids; those are
            # never a reason to stop, and returning True here would make lldb stop on them
            return thread.GetStopReasonDataAtIndex(0) >= 0
        if reason != lldb.eStopReasonSignal:
            return True
        for frame in thread:
            module = frame.GetModule()
            if module.IsValid() and module.GetFileSpec().GetFilename() == OURS:
                return True
        return False   # not ours: let the process carry on (the signal is passed to the app)
