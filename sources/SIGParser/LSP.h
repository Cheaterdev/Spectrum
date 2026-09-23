#pragma once

// `sigparser --lsp`: a Language Server Protocol server over stdin/stdout that
// publishes the same diagnostics a generator run would report, live, for the
// unsaved contents of open editor buffers.
int run_lsp();
