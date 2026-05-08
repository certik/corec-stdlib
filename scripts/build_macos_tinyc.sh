#!/usr/bin/env bash
# Build corec_stdlib_test_macos_tinyc by compiling each C source with
# tinyC (the small MLIR-based C compiler at ../mlir/examples/tinyc) and
# linking the resulting .ll files with clang.
#
# This is the corec-without-system-libc build: -nostdlib -nostdinc
# -fno-builtin, with our own freestanding stdlib (corec-stdlib's
# stdlib/) and corec base sitting on top of corec/platform/platform_macos.c.
#
# tinyC's preprocessor is intentionally tiny, so we delegate
# preprocessing to system clang per .c file (mirroring corec's
# scripts/build_macos_tinyc.sh) and then feed the preprocessed output to
# tinyC. Only the *compile* (parse → MLIR → LLVM IR) step uses tinyC.

set -euo pipefail

cd "$(dirname "$0")/.."

TINYC="${TINYC:-../mlir/tinyc}"
if [ ! -x "$TINYC" ]; then
    echo "tinyC binary not found at $TINYC." >&2
    echo "Build it first:  cd ../mlir && pixi run -e upstream build_tinyc_upstream" >&2
    exit 1
fi

OUT=corec_stdlib_test_macos_tinyc
WORK=build_tinyc
rm -rf "$WORK"
mkdir -p "$WORK"

SOURCES=(
    test_stdlib.c
    stdlib/stdio.c
    stdlib/stdlib.c
    stdlib/printf.c
    stdlib/string_impl.c
    corec/base/io.c
    corec/base/buddy.c
    corec/base/arena.c
    corec/base/scratch.c
    corec/base/format.c
    corec/base/math.c
    corec/base/string.c
    corec/base/mem.c
    corec/base/numconv.c
    corec/base/assert.c
    corec/base/exit.c
    corec/platform/platform_macos.c
)

LL_FILES=()
for src in "${SOURCES[@]}"; do
    pp="$WORK/$(echo "$src" | tr '/' '_').i"
    ll="$WORK/$(echo "$src" | tr '/' '_').ll"
    echo "[clang -E] $src"
    clang -E -P -nostdinc -fno-builtin -DNDEBUG -Dmain=app_main \
        -I corec -I stdlib -I . "$src" -o "$pp"
    echo "[tinyc   ] $pp -> $ll"
    "$TINYC" --emit=llvm -o "$ll" "$pp"
    LL_FILES+=("$ll")
done

echo "[link    ] $OUT"
clang -nostdlib -fno-builtin -o "$OUT" "${LL_FILES[@]}" \
    ../mlir/examples/tinyc/runtime.c -lSystem -Wl,-e,__start
echo "Built $OUT"
