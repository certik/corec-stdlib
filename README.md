# corec-stdlib — a C standard library compatibility layer for Core C

`corec-stdlib` is a **compatibility layer that exposes a subset of the C
standard library on top of [Core C](https://github.com/certik/corec)**. Its
purpose is to let existing C programs — programs written against
`<stdio.h>`, `<string.h>`, `<stdlib.h>`, `<assert.h>` and friends, with a
normal `int main(void)` — build and run unmodified on top of Core C, without
linking against the host platform's libc.

Concretely, this repository provides a small subset of `<stdio.h>`,
`<stdlib.h>`, `<string.h>`, `<assert.h>`, `<stdint.h>`, `<stddef.h>`,
`<stdarg.h>`, `<stdbool.h>`, `<ctype.h>`, `<float.h>` and `<limits.h>`,
all implemented as thin wrappers around
[`corec`](https://github.com/certik/corec)'s `base/` and `platform/` layers.
Programs that use it still build with `-nostdlib -nostdinc -fno-builtin`;
the names just look familiar.

## When (and when not) to use this

[Core C](https://github.com/certik/corec) is the foundation. It already
provides everything a program needs — string handling, formatted I/O,
memory management, file and OS access — through `base/` and `platform/`.
A native Core C program uses those APIs directly, uses `int app_main(void)`
as its entry point, and never sees libc or this repository at all. **That
is the recommended way to write new code.**

`corec-stdlib` exists for one reason: **you have C source that is already
written against the C standard library and you want to build it on top of
Core C without rewriting it.** That covers two common cases:

* You are porting an existing application whose source uses `printf`,
  `malloc`, `strcpy`, `assert`, etc., and a `main()` function.
* You want to depend on a third-party C library that itself uses the C
  standard library internally.

Outside of those cases there is no particular reason to prefer this layer.
The stdlib API does not add anything on top of `corec/base/`; if anything
it is the weaker of the two:

* It models strings as `char *` plus a separately-tracked length, instead
  of `corec`'s explicit `string` (pointer + length together).
* It encourages ad-hoc `malloc` / `free`, instead of `corec`'s arena-based
  allocators that free everything in one call.
* It is a global, non-namespaced API surface; `corec/base/` is namespaced
  (`base_*`) and explicit about ownership and lifetimes.

There is **nothing wrong** with using `corec-stdlib` when you need it — the
implementation is small, well-tested, and built on the same primitives as
the rest of Core C. It is just a means to an end: bringing existing
stdlib-using code into the Core C world. For greenfield code on Core C,
use `corec/base/` directly.

## Entry point: `app_main` vs `main`

Native Core C programs use `int app_main(void)` as their entry point.
There is no `main`, no C runtime, and no implicit dependency on a host
libc; corec's platform layer (`platform_linux.c`, `platform_macos.c`,
`platform_windows.c`, `platform_wasm.c`) is what calls `app_main()`.

Programs that use `corec-stdlib` keep their original `int main(void)` —
that is the whole point of the compatibility layer. The build system
simply supplies `-Dmain=app_main` so the user's `main` is renamed to
`app_main` at compile time and corec's platform layer can call into it.
Nothing in the program itself has to change.

`test_stdlib.c` in this repository plays two roles at once and is worth
reading as an example:

1. **An example port of an existing stdlib-using program.** The file uses
   only standard headers and a normal `int main(void)`, exactly the way a
   typical C program is written. The pixi tasks build it on top of Core C
   with `-Dmain=app_main`; the source itself is unchanged.
2. **A conformance test for the stdlib subset.** Because the file is also
   a correct, portable C program, the same source can be compiled with
   the host system compiler against the host's real C standard library.
   CI does this first — if those assertions pass against the real libc,
   then passing them against our reimplementation actually means the
   reimplementation matches the standard.

## Layout

```
corec-stdlib/
├── corec/                  # Core C submodule (platform/, base/)
├── stdlib/                 # C standard library subset (headers + .c)
│   ├── assert.h
│   ├── ctype.h
│   ├── float.h
│   ├── limits.h
│   ├── stdarg.h
│   ├── stdbool.h
│   ├── stddef.h
│   ├── stdint.h
│   ├── stdio.h    stdio.c    # FILE I/O + snprintf/vsnprintf
│   ├── stdlib.h   stdlib.c   # malloc, exit, ato*, rand
│   ├── string.h   string_impl.c
│   └── printf.c              # printf/vprintf (declared in stdio.h)
├── test_stdlib.c           # Conformance tests + example port
├── pixi.toml               # Build tasks for linux/macos/windows/wasm
└── .github/workflows/      # CI: host stdlib + nostdlib on each platform
```

## API surface

The headers below mirror the standard ones. Anything not listed is
intentionally not provided yet — when something is needed, add it (see
[Contributing](#contributing--extending)).

| Header | Provided |
| --- | --- |
| `<assert.h>` | `assert(expr)` — aborts on failure with file/line/function. |
| `<ctype.h>` | `isdigit`, `isxdigit`, `isalpha`, `isalnum`, `isupper`, `islower`, `isspace`, `iscntrl`, `isprint`, `isgraph`, `ispunct`, `toupper`, `tolower` (ASCII / "C" locale, inline). |
| `<float.h>` | `FLT_MAX` (via `corec/base/types.h`). |
| `<limits.h>` | `CHAR_BIT`, `(S)CHAR_MIN/MAX`, `UCHAR_MAX`, `SHRT_MIN/MAX`, `USHRT_MAX`, `INT_MIN/MAX`, `UINT_MAX`, `LONG_MIN/MAX`, `ULONG_MAX`, `LLONG_MIN/MAX`, `ULLONG_MAX`. Platform-aware (LP64 / LLP64 / ILP32). |
| `<stdarg.h>` | `va_list`, `va_start`, `va_arg`, `va_end`, `va_copy` (compiler builtins). |
| `<stdbool.h>` | `bool`, `true`, `false`. |
| `<stddef.h>` | `size_t`, `ptrdiff_t`, `NULL`, `offsetof`. |
| `<stdint.h>` | `int8_t`..`int64_t`, `uint8_t`..`uint64_t`, `uintptr_t`, `size_t`, `INTn_MIN`/`INTn_MAX`/`UINTn_MAX` for `n ∈ {8,16,32,64}`, `SIZE_MAX`, `INTNN_C` / `UINTNN_C` macros. |
| `<stdio.h>` | `printf`, `vprintf`, `fprintf`, `vfprintf`, `sprintf`, `vsprintf`, `snprintf`, `vsnprintf`; `FILE*` I/O — `fopen`, `fclose`, `fseek`, `ftell`, `fread`, `fwrite`, `fputc`, `fputs`, `SEEK_SET`/`CUR`/`END`, `EOF`; standard streams `stdin`, `stdout`, `stderr`. |
| `<stdlib.h>` | `malloc`, `calloc`, `realloc`, `free`, `exit`, `abort`, `getenv`, `atoi`, `atoll`, `atof`, `rand`, `srand`, `RAND_MAX`, `EXIT_SUCCESS`, `EXIT_FAILURE`, `NULL`. |
| `<string.h>` | `strlen`, `strcpy`, `strncpy`, `strcmp`, `strncmp`, `strchr`, `strrchr`, `strstr`, `strcspn`, `memcpy`, `memmove`, `memcmp`, `memset`, `memchr`. |

`printf` / `fprintf` / `sprintf` / `snprintf` support the format set
provided by `corec/base/numconv.c`'s `base_vsnprintf`:
`%d`, `%i`, `%u`, `%ld`, `%lu`, `%lld`, `%llu`, `%zu`,
`%x`, `%X`, `%lx`, `%lX`, `%llx`, `%llX`,
`%p`, `%c`, `%s`, `%f`, `%.Nf`, `%%`.
Width specifiers (e.g. `%5d`, `%-10s`) are not supported.

`getenv` is a stub that always returns `NULL` — wasm32-wasi modules
have no environment block, and most callers treat `NULL` as "not set".

## Build & test

All builds are driven by [pixi](https://pixi.sh). Make sure the submodule
is checked out:

```bash
git submodule update --init --recursive
```

Then:

```bash
pixi run -e linux   test_linux       # Linux native (clang -nostdlib)
pixi run -e macos   test_macos       # macOS native (clang -nostdlib, on macOS only)
pixi run -e windows test_windows     # Windows native (MSVC /kernel, on Windows only)
pixi run -e wasm    test_wasm        # WebAssembly via wasmtime
```

To run the same `test_stdlib.c` against the host's *real* C standard
library (this is what CI runs first, to verify the assertions are correct
before checking them against the nostdlib build):

```bash
pixi run -e linux   test_host        # Linux: cc against glibc/musl/...
pixi run -e macos   test_host        # macOS: cc against libSystem
pixi run -e windows test_host        # Windows: cl against the MSVC CRT
```

The same `corec_stdlib_test.wasm` runs in `wasmtime` and any other
WASI-compatible runtime.

## Using corec-stdlib in your project

Add this repository (and Core C through it) as a submodule:

```bash
git submodule add https://github.com/certik/corec-stdlib third_party/corec-stdlib
git submodule update --init --recursive
```

Then build your `myapp.c` (containing a normal `int main(void)`) like so:

```bash
clang \
    -nostdlib -nostdinc -fno-builtin \
    -Dmain=app_main \
    -I third_party/corec-stdlib/corec \
    -I third_party/corec-stdlib/stdlib \
    -o myapp \
    myapp.c \
    third_party/corec-stdlib/stdlib/*.c \
    third_party/corec-stdlib/corec/base/*.c \
    third_party/corec-stdlib/corec/platform/platform_<host>.c
```

`-Dmain=app_main` rewrites your `main` so corec's platform layer (which
provides `_start`) can call into it. Look at `pixi.toml` in this repository
for ready-made command lines for each target (Linux raw syscalls, macOS
`libSystem`, Windows `kernel32`, WebAssembly WASI).

## Continuous Integration

GitHub Actions runs the test suite on Linux, macOS, and Windows on every
push and PR. Each platform runs `test_stdlib.c` twice:

1. **Against the host's standard library**, compiled with the system C
   compiler (`cc` / `cl.exe`) and linked against the platform's libc with
   `-Wall -Wextra -Werror` (or `/W3 /WX`). If the test passes here, the
   behavior it asserts is what the C standard library is supposed to do.
2. **Against this repository's stdlib subset**, compiled with `-nostdlib
   -nostdinc -fno-builtin` on top of `corec/`. If this also passes, our
   reimplementation matches that behavior.

The same `test_stdlib.c` source is used in both cases — the nostdlib build
just adds `-Dmain=app_main` so corec's platform layer can call into it as
its entry point.

The nostdlib pass also runs the WebAssembly build under `wasmtime`. See
`.github/workflows/CI.yml`.

## Contributing / extending

A few conventions worth knowing before submitting changes:

* **Stay inside the sandbox.** Code in `stdlib/` is built with
  `-nostdlib -nostdinc -fno-builtin`. Do not include the host's `<stdio.h>`,
  `<string.h>`, etc. Use what `corec/base/` and `corec/platform/platform.h`
  provide; if something is missing in `corec`, add it there first.
* **Adding a stdlib function.** Add the prototype to the appropriate
  `stdlib/*.h` header, implement it in the matching `stdlib/*.c` (typically
  a thin wrapper around a `base_*` function from `corec/base/`), and add a
  test case to `test_stdlib.c`. Add the function to the table in the
  [API surface](#api-surface) section of this README.
* **Tests.** All tests live in `test_stdlib.c`. Add a `test_<topic>()`
  function and call it from `run_tests()`. Tests must compile and pass
  *both* against the host's real stdlib and against this repository's
  reimplementation — that is what the two-stage CI verifies. If a test
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

## License

MIT — see [`LICENSE`](LICENSE).
