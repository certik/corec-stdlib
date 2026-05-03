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
* `test_stdlib.c` — test entry point and stdlib test suite.

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

## Continuous Integration

GitHub Actions runs the full test suite — native binary on Linux, macOS, and
Windows, plus WebAssembly via `wasmtime` — on every push and PR. See
`.github/workflows/CI.yml`.

## Contributing / extending

A few conventions worth knowing before submitting changes:

* **Stay inside the sandbox.** Code in `stdlib/` is built with
  `-nostdlib -nostdinc -fno-builtin`. Do not include the host's `<stdio.h>`,
  `<string.h>`, etc. Use what `corec/base/` and `corec/platform/platform.h`
  provide; if something is missing in `corec`, add it there first.
* **Adding a stdlib function.** Add the prototype to the appropriate
  `stdlib/*.h` header, implement it in the matching `stdlib/*.c` (typically a
  thin wrapper around a `base_*` function from `corec/base/`), and add a test
  case to `test_stdlib.c`.
* **Tests.** All tests live in `test_stdlib.c`. Add a `test_<topic>()`
  function and call it from `test_stdlib()` in `test_stdlib.c`. If a test
  creates files on disk, add their names to `.gitignore`.

### Making changes that span `corec` and `corec-stdlib`

Because `corec` is included as a submodule, a change that touches both
repositories needs to be staged carefully so CI in this repository can
verify the combined result before anything is merged. The typical workflow is:

1. **Open a branch in `corec`.** Inside `corec/`, create a branch off `main`,
   commit your change, and push it:
   ```bash
   cd corec
   git checkout -b my-corec-change
   # edit files...
   git commit -am "..."
   git push -u origin my-corec-change
   ```
   Open a PR against `corec` `main`, but **do not merge it yet**.
2. **Open a branch in `corec-stdlib`.** From the top of this repository,
   create a branch off `main` for the corresponding change. The submodule
   is already pointing at the new commit you just pushed in step 1, so
   stage that pointer together with your `corec-stdlib` changes:
   ```bash
   git checkout -b my-stdlib-change
   git add corec stdlib/...        # submodule bump + local edits
   git commit -m "..."
   git push -u origin my-stdlib-change
   ```
   Open a PR against `corec-stdlib` `main`. CI will check out the submodule
   at the branch commit and exercise the combined change on every platform.
3. **Merge `corec` first.** Once CI is green here, merge the `corec` PR.
4. **Bump the submodule to `corec` `main`.** On the `corec-stdlib` branch,
   move the submodule pointer from the temporary branch commit to the now-
   merged `corec` `main`:
   ```bash
   cd corec
   git fetch origin
   git checkout origin/main
   cd ..
   git add corec
   git commit --amend --no-edit    # or a fresh commit
   git push --force-with-lease
   ```
5. **Merge the `corec-stdlib` PR.** Once CI is green again, merge it.

This keeps `main` of both repositories pointing at commits that build and
test cleanly together.
