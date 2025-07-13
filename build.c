#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#define NOB_IMPLEMENTATION
#include "deps/nob/nob.h"
#include <locale.h>

#define BUILD_FOLDER "bin/"
#define OBJ_FOLDER "obj/"
#define SRC_FOLDER "src/"

#define FLAG_ALL  "all"

#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)
#define HAS_FLAG(__flag) check_for_pack_flag(__flag, argc, argv)

int check_for_pack_flag(char *flag, int argc, char **argv) {
    for (int i = 1; i < argc; i++) { 
        size_t a = strlen(argv[i]);
        size_t b = strlen(flag);
        if (a != b) continue;
        if (memcmp(argv[i], flag, a) == 0) return 1;
    }

    return 0;
}

int build_chiff(void) {
    Nob_Cmd cmd = { 0 };

    nob_cmd_append(&cmd, "clang-cl", "/std:c++14", "/W3", "/utf-8");

    nob_cmd_append(&cmd, 
            "/D_CRT_SECURE_NO_WARNINGS",
            "/D_WINSOCK_DEPRECATED_NO_WARNINGS",
            "/DDEBUG",

            "-FC", "/Zi", "-EHsc", "-mavx2", "-maes", "-mpclmul",
            "-o", BUILD_FOLDER"chiff",
            SRC_FOLDER"main.cpp",

            "-Ic:.\\deps",
            "-Ic:.\\external",
            "-Ic:.\\meta");

    if (!nob_cmd_run_sync_and_reset(&cmd)) return 0;
    return 1;
}


int main(int argc, char **argv) {
    setlocale(LC_ALL, ".utf-8");
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    if (HAS_FLAG(FLAG_ALL)) {
        if (!build_chiff()) return 1;
        return 0;
    }

    if (!build_chiff()) return 1;
    return 0;
}

