#include <base/io.h>
#include <test_stdlib.h>

int app_main(void) {
    test_stdlib();

    println(str_lit("=== All tests passed ==="));
    return 0;
}
