# Core C — stdlib subset

A minimal C standard library subset built on top of [Core C](https://github.com/certik/corec).

This repository implements a small subset of the C standard library
(`<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<assert.h>`, `<stdint.h>`,
`<stddef.h>`, `<stdarg.h>`, `<stdbool.h>`, `<ctype.h>`, `<float.h>`,
`<limits.h>`, plus a custom `<printf.h>`) for programs that build with
`-nostdlib -nostdinc -fno-builtin`.

The actual platform interface and `base/` utilities come from the
[`corec`](https://github.com/certik/corec) submodule. This repository only
adds the standard-library names that wrap them.

## Layout

* `corec/` — the [Core C](https://github.com/certik/corec) submodule
  (provides `platform/` and `base/`).
* `stdlib/` — C standard library subset headers and implementation files.
* `tests.c`, `test_stdlib.c`, `test_stdlib.h` — test entry point and
  stdlib test suite.

## Build & test

All builds are driven by [pixi](https://pixi.sh). Make sure the submodule
is checked out:

```bash
git submodule update --init --recursive
```

Then:

```bash
pixi run -e linux   test_linux       # Linux native
pixi run -e macos   test_macos       # macOS native (on macOS only)
pixi run -e windows test_windows     # Windows native (on Windows only, MSVC)
pixi run -e wasm    test_wasm        # WebAssembly via wasmtime
```

The same `corec_stdlib_test.wasm` runs in `wasmtime` and any other
WASI-compatible runtime.
