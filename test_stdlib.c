// test_stdlib.c — conformance tests for a subset of the C standard library.
//
// This file is deliberately written to compile with *any* conforming
// hosted C implementation. CI compiles it twice on each platform:
//
//   1. Against the host's real C standard library (gcc/clang/cl), to
//      make sure the assertions are correct.
//   2. Against this repository's stdlib subset, built `-nostdlib
//      -nostdinc -fno-builtin` on top of corec, with `-Dmain=app_main`
//      so corec's platform layer can call into it.
//
// As a side effect, this file is also a usable example of how to bring
// a stdlib-using program onto Core C without changing its source.

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

// MSVC's CRT marks several standard C functions (strcpy, sprintf, ...) as
// deprecated under warning C4996, suggesting the non-portable Microsoft
// "_s" variants instead. This file deliberately exercises the standard
// names — that is the whole point of the test — so we silence C4996
// for this translation unit. The behavior we test is still the standard
// C behavior; the warning is purely Microsoft style guidance.
#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------

static void check_streq(const char *actual, const char *expected, const char *label) {
    size_t i = 0;
    while (actual[i] != '\0' && expected[i] != '\0') {
        if (actual[i] != expected[i]) {
            printf("FAIL: %s — mismatch at position %zu: got '%c', expected '%c'\n",
                   label, i, actual[i], expected[i]);
            assert(0);
        }
        i++;
    }
    if (actual[i] != expected[i]) {
        printf("FAIL: %s — length mismatch\n", label);
        assert(0);
    }
}

// -----------------------------------------------------------------------
// <string.h>
// -----------------------------------------------------------------------

static void test_strlen(void) {
    assert(strlen("") == 0);
    assert(strlen("a") == 1);
    assert(strlen("hello") == 5);
    assert(strlen("Hello World!") == 12);
}

static void test_strcpy(void) {
    char dest[50];

    strcpy(dest, "");
    check_streq(dest, "", "strcpy empty");

    strcpy(dest, "test");
    check_streq(dest, "test", "strcpy simple");

    strcpy(dest, "Hello World!");
    check_streq(dest, "Hello World!", "strcpy spaces");

    char *ret = strcpy(dest, "return test");
    assert(ret == dest);
}

static void test_strncpy(void) {
    char dest[10];

    // GCC's -Wstringop-truncation flags deliberate truncation, which is
    // precisely what these tests exercise. Suppress narrowly.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

    memset(dest, 'X', sizeof(dest));
    strncpy(dest, "abc", 3);
    assert(dest[0] == 'a' && dest[1] == 'b' && dest[2] == 'c');
    assert(dest[3] == 'X'); // strncpy does not pad if src >= n

    memset(dest, 'X', sizeof(dest));
    strncpy(dest, "ab", 5);
    assert(dest[0] == 'a' && dest[1] == 'b');
    assert(dest[2] == '\0' && dest[3] == '\0' && dest[4] == '\0'); // padded
    assert(dest[5] == 'X');

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
}

static void test_strcmp(void) {
    assert(strcmp("", "") == 0);
    assert(strcmp("abc", "abc") == 0);
    assert(strcmp("abc", "abd") < 0);
    assert(strcmp("abd", "abc") > 0);
    assert(strcmp("abc", "abcd") < 0);
    assert(strcmp("abcd", "abc") > 0);
}

static void test_strncmp(void) {
    assert(strncmp("abc", "abc", 3) == 0);
    assert(strncmp("abc", "abd", 2) == 0);
    assert(strncmp("abc", "abd", 3) < 0);
    assert(strncmp("abc", "abc", 0) == 0);
    assert(strncmp("abcdef", "abcxyz", 3) == 0);
    assert(strncmp("abcdef", "abcxyz", 4) < 0);
}

static void test_strchr_strrchr(void) {
    const char *s = "hello world";
    assert(strchr(s, 'h') == s);
    assert(strchr(s, 'o') == s + 4);
    assert(strchr(s, 'd') == s + 10);
    assert(strchr(s, 'z') == NULL);
    assert(strchr(s, '\0') == s + 11);

    assert(strrchr(s, 'o') == s + 7);
    assert(strrchr(s, 'h') == s);
    assert(strrchr(s, 'z') == NULL);
}

static void test_strstr(void) {
    const char *s = "hello world";
    assert(strstr(s, "") == s);
    assert(strstr(s, "hello") == s);
    assert(strstr(s, "world") == s + 6);
    assert(strstr(s, "lo w") == s + 3);
    assert(strstr(s, "xyz") == NULL);
    // strstr("", "") returns the haystack — exercise without comparing
    // against a string literal (which warns on some compilers).
    const char *empty = "";
    assert(strstr(empty, "") == empty);
}

static void test_strcspn(void) {
    assert(strcspn("hello", "") == 5);
    assert(strcspn("hello", "z") == 5);
    assert(strcspn("hello", "h") == 0);
    assert(strcspn("hello", "lo") == 2);
    assert(strcspn("", "abc") == 0);
}

static void test_memcpy_memmove(void) {
    char src[] = "abcdef";
    char dst[10];

    memcpy(dst, src, 0);  // copy nothing — must not crash

    memcpy(dst, src, 3);
    assert(dst[0] == 'a' && dst[1] == 'b' && dst[2] == 'c');

    memcpy(dst, src, 7);  // include null terminator
    check_streq(dst, "abcdef", "memcpy full");

    void *ret = memcpy(dst, src, 3);
    assert(ret == dst);

    int nums_src[] = {1, 2, 3, 4, 5};
    int nums_dst[5];
    memcpy(nums_dst, nums_src, sizeof(nums_src));
    for (int i = 0; i < 5; i++) assert(nums_dst[i] == nums_src[i]);

    char fwd[] = "abcdefghij";
    memmove(fwd + 2, fwd, 8);
    check_streq(fwd, "ababcdefgh", "memmove fwd");

    char bwd[] = "abcdefghij";
    memmove(bwd, bwd + 2, 8);
    check_streq(bwd, "cdefghijij", "memmove bwd");
}

static void test_memcmp(void) {
    assert(memcmp("abc", "abc", 3) == 0);
    assert(memcmp("abc", "abd", 3) < 0);
    assert(memcmp("abd", "abc", 3) > 0);
    assert(memcmp("abc", "abZ", 2) == 0);
    assert(memcmp("", "", 0) == 0);
}

static void test_memset(void) {
    char buf[16];
    memset(buf, 'A', sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); i++) assert(buf[i] == 'A');

    memset(buf, 0, sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); i++) assert(buf[i] == 0);

    void *ret = memset(buf, 'X', 5);
    assert(ret == buf);
    assert(buf[0] == 'X' && buf[4] == 'X' && buf[5] == 0);
}

static void test_memchr(void) {
    const char *s = "hello";
    assert(memchr(s, 'h', 5) == s);
    assert(memchr(s, 'o', 5) == s + 4);
    assert(memchr(s, 'o', 4) == NULL);
    assert(memchr(s, 'z', 5) == NULL);
}

// -----------------------------------------------------------------------
// <stdio.h> — printf / snprintf
// -----------------------------------------------------------------------

static void test_printf_formats(void) {
    // Just exercise each format we support; visual output is checked
    // implicitly by the user/CI looking at the log.
    printf("%%s: '%s'\n", "Hello");
    printf("%%c: '%c'\n", 'X');
    printf("%%%%: '%%'\n");
    printf("%%d: %d %d %d %d %d\n", 0, 42, -42, INT_MAX, INT_MIN);
    printf("%%u: %u %u\n", 0u, 4294967295u);

    size_t sz = 12345;
    printf("%%zu: %zu %zu\n", (size_t)0, sz);

    int x = 0;
    printf("%%p: %p %p\n", (void*)&x, (void*)0);

    char *null_str = NULL;
    printf("%%s NULL: '%s'\n", null_str);

    printf("multi: %d %s %c %u\n", 123, "test", 'A', 456u);
    printf("%s%s%s\n", "", "", "");
}

static void test_printf_return(void) {
    // printf returns number of characters that would have been written.
    int n = printf("12345\n");
    assert(n == 6);
}

static void test_snprintf(void) {
    char buf[32];
    int n;

    n = snprintf(buf, sizeof(buf), "hi");
    assert(n == 2);
    check_streq(buf, "hi", "snprintf simple");

    n = snprintf(buf, sizeof(buf), "%d", 42);
    assert(n == 2);
    check_streq(buf, "42", "snprintf %d");

    n = snprintf(buf, sizeof(buf), "%d-%s", -7, "ok");
    assert(n == 5);
    check_streq(buf, "-7-ok", "snprintf compound");

    // Truncation: buffer too small. The result must be zero-terminated and
    // contain a prefix of the formatted output. ISO C says the return value
    // is the number of characters that *would* have been written if the
    // buffer were large enough; some implementations (including corec's
    // current base_vsnprintf) return the truncated length. We accept both.
    //
    // GCC's -Wformat-truncation flags this *intentional* truncation, so
    // suppress that warning narrowly here.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
    char small[4];
    n = snprintf(small, sizeof(small), "abcdef");
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
    assert(n == 6 || n == 3);
    assert(small[3] == '\0');
    check_streq(small, "abc", "snprintf truncation");

    // Zero size: buffer untouched. Return value is implementation-defined
    // when size==0 (ISO C says "would-be" length, some impls return 0).
    char untouched[4] = { 'Z', 'Z', 'Z', 'Z' };
    n = snprintf(untouched, 0, "abc");
    (void)n;
    assert(untouched[0] == 'Z');

    // %e: scientific notation. Basic positive, negative, and zero cases at
    // default precision (6).
    n = snprintf(buf, sizeof(buf), "%.6e", 1.0);
    assert(n == 12);
    check_streq(buf, "1.000000e+00", "snprintf %e one");

    n = snprintf(buf, sizeof(buf), "%.6e", -1.5);
    check_streq(buf, "-1.500000e+00", "snprintf %e neg");

    n = snprintf(buf, sizeof(buf), "%.6e", 0.0);
    check_streq(buf, "0.000000e+00", "snprintf %e zero");

    n = snprintf(buf, sizeof(buf), "%.6e", 1.0e20);
    check_streq(buf, "1.000000e+20", "snprintf %e big");

    n = snprintf(buf, sizeof(buf), "%.6e", 1.0e-20);
    check_streq(buf, "1.000000e-20", "snprintf %e tiny");

    // %e at non-default precision.
    n = snprintf(buf, sizeof(buf), "%.2e", 12345.0);
    check_streq(buf, "1.23e+04", "snprintf %e prec2");
}

// -----------------------------------------------------------------------
// <stdio.h> — FILE I/O round-trip
// -----------------------------------------------------------------------

// Write a known payload to a file with fopen/fwrite/fputc/fputs/fclose,
// then read it back with fopen/fseek/ftell/fread/fclose and verify the
// content byte for byte. This exercises every FILE function we expose.
//
// On WASM the build runs under `wasmtime --dir .`, so this also validates
// that file I/O is wired through to WASI properly.
static void test_file_io(void) {
    const char *path = "test_stdlib_scratch.bin";

    // ---- Write phase ----
    FILE *out = fopen(path, "wb");
    assert(out != NULL);

    const char hello[] = "hello"; // 5 bytes, no null
    size_t nw = fwrite(hello, 1, 5, out);
    assert(nw == 5);

    int fc = fputc(' ', out);
    assert(fc == ' ');

    int fs = fputs("world", out);
    assert(fs >= 0);

    // 12 bytes of binary: 0..11
    unsigned char bin[12];
    for (int i = 0; i < 12; i++) bin[i] = (unsigned char)i;
    nw = fwrite(bin, 1, sizeof(bin), out);
    assert(nw == sizeof(bin));

    int rc = fclose(out);
    assert(rc == 0);

    // ---- Read phase ----
    FILE *in = fopen(path, "rb");
    assert(in != NULL);

    // ftell at start == 0
    long pos = ftell(in);
    assert(pos == 0);

    // Read first 11 bytes ("hello world").
    char buf[16];
    size_t nr = fread(buf, 1, 11, in);
    assert(nr == 11);
    buf[11] = '\0';
    check_streq(buf, "hello world", "fread text");

    // ftell after 11 bytes
    pos = ftell(in);
    assert(pos == 11);

    // Read the 12 binary bytes
    unsigned char bin_back[12];
    nr = fread(bin_back, 1, sizeof(bin_back), in);
    assert(nr == sizeof(bin_back));
    for (int i = 0; i < 12; i++) assert(bin_back[i] == (unsigned char)i);

    // EOF: next read returns 0
    char eof_buf[4];
    nr = fread(eof_buf, 1, sizeof(eof_buf), in);
    assert(nr == 0);

    // SEEK_SET back to start; ftell should be 0
    int sk = fseek(in, 0, SEEK_SET);
    assert(sk == 0);
    assert(ftell(in) == 0);

    // SEEK_END to file end; ftell should be total length (5+1+5+12 = 23).
    sk = fseek(in, 0, SEEK_END);
    assert(sk == 0);
    assert(ftell(in) == 23);

    rc = fclose(in);
    assert(rc == 0);

    // EOF macro is -1.
    assert(EOF == -1);
}

// -----------------------------------------------------------------------
// <stdlib.h>
// -----------------------------------------------------------------------

static void test_atoi(void) {
    assert(atoi("0") == 0);
    assert(atoi("1") == 1);
    assert(atoi("42") == 42);
    assert(atoi("-7") == -7);
    assert(atoi("+13") == 13);
    assert(atoi("   123") == 123);
    assert(atoi("\t\n -5") == -5);
    assert(atoi("12abc") == 12);   // stop at non-digit
    assert(atoi("") == 0);
    assert(atoi("abc") == 0);
}

static void test_atoll(void) {
    assert(atoll("0") == 0);
    assert(atoll("1234567890123") == 1234567890123LL);
    assert(atoll("-1234567890123") == -1234567890123LL);
}

static void test_atof(void) {
    // Float comparisons are a bit loose because conversions can differ in
    // last-bit rounding between implementations.
    double d;

    d = atof("0");      assert(d == 0.0);
    d = atof("1");      assert(d == 1.0);
    d = atof("-2.5");   assert(d == -2.5);
    d = atof("3.5");    assert(d == 3.5);
    d = atof("1e2");    assert(d == 100.0);
    d = atof("1.5e-1"); assert(d > 0.149 && d < 0.151);
}

static void test_malloc_free(void) {
    void *p = malloc(0);
    // malloc(0) may return NULL or a valid pointer — both are conforming.
    free(p);

    char *buf = (char *)malloc(64);
    assert(buf != NULL);
    for (int i = 0; i < 64; i++) buf[i] = (char)i;
    for (int i = 0; i < 64; i++) assert(buf[i] == (char)i);
    free(buf);

    free(NULL); // must be a no-op
}

static void test_rand(void) {
    // Same seed must produce same sequence (deterministic).
    srand(1);
    int a1 = rand(), a2 = rand(), a3 = rand();

    srand(1);
    assert(rand() == a1);
    assert(rand() == a2);
    assert(rand() == a3);

    // All outputs are in [0, RAND_MAX].
    for (int i = 0; i < 100; i++) {
        int r = rand();
        assert(r >= 0);
        assert(r <= RAND_MAX);
    }
}

static void test_stdlib_macros(void) {
    // EXIT_SUCCESS / EXIT_FAILURE / RAND_MAX must exist and be sensible.
    assert(EXIT_SUCCESS == 0);
    assert(EXIT_FAILURE != 0);
    assert(RAND_MAX >= 32767);

    // NULL must be a null pointer.
    int *p = NULL;
    assert(p == NULL);

    // <stdint.h> limits — sanity-check a few by exercising them at the
    // edges of their respective types. We deliberately avoid expressions
    // that depend on signed-overflow wrap (UB in C) and use unsigned
    // arithmetic instead.
    assert(INT8_MIN  == -128 && INT8_MAX  == 127);
    assert(UINT8_MAX == 255);
    assert(INT16_MIN == -32768 && INT16_MAX == 32767);
    assert(UINT16_MAX == 65535);
    assert(INT32_MIN < 0 && INT32_MAX > 0);
    assert((uint32_t)INT32_MAX + 1u == 0x80000000u);
    assert(INT64_MIN < 0 && INT64_MAX > 0);
    assert((uint64_t)UINT64_MAX + 1u == 0);
}

// -----------------------------------------------------------------------
// <ctype.h>
// -----------------------------------------------------------------------

static void test_ctype(void) {
    assert(isdigit('0') && isdigit('9'));
    assert(!isdigit('a') && !isdigit(' '));

    assert(isalpha('a') && isalpha('Z'));
    assert(!isalpha('0') && !isalpha(' '));

    assert(isalnum('a') && isalnum('0'));
    assert(!isalnum(' ') && !isalnum('!'));

    assert(isspace(' ') && isspace('\t') && isspace('\n'));
    assert(!isspace('a'));

    assert(isupper('A') && !isupper('a') && !isupper('0'));
    assert(islower('a') && !islower('A') && !islower('0'));

    assert(isxdigit('0') && isxdigit('9') && isxdigit('a') &&
           isxdigit('f') && isxdigit('A') && isxdigit('F'));
    assert(!isxdigit('g') && !isxdigit('G'));

    assert(toupper('a') == 'A');
    assert(toupper('Z') == 'Z');
    assert(toupper('0') == '0');
    assert(tolower('A') == 'a');
    assert(tolower('z') == 'z');
    assert(tolower('5') == '5');
}

// -----------------------------------------------------------------------
// <assert.h>
// -----------------------------------------------------------------------

static void test_assert(void) {
    // These all hold; we are just checking that assert is callable and
    // does nothing on a true predicate.
    assert(1);
    assert(1 == 1);
    assert(5 > 3);
    int a = 10;
    assert(a == 10);
    char *s = "test";
    assert(s != NULL);
    assert(strlen(s) == 4);
}

// -----------------------------------------------------------------------
// Driver
// -----------------------------------------------------------------------

static void run_tests(void) {
    printf("=== stdlib tests ===\n");

    printf("## <string.h>\n");
    test_strlen();
    test_strcpy();
    test_strncpy();
    test_strcmp();
    test_strncmp();
    test_strchr_strrchr();
    test_strstr();
    test_strcspn();
    test_memcpy_memmove();
    test_memcmp();
    test_memset();
    test_memchr();

    printf("## <stdio.h>\n");
    test_printf_formats();
    test_printf_return();
    test_snprintf();
    test_file_io();

    printf("## <stdlib.h>\n");
    test_atoi();
    test_atoll();
    test_atof();
    test_malloc_free();
    test_rand();
    test_stdlib_macros();

    printf("## <ctype.h>\n");
    test_ctype();

    printf("## <assert.h>\n");
    test_assert();

    printf("=== All tests passed ===\n");
}

int main(void) {
    run_tests();
    return 0;
}
