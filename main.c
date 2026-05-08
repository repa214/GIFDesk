#include "manager.h"

/**
        Minimal system requirements:

        OS: Windows XP
        CPU: i686 with SSE support
        GPU: OpenGL 1.1 support
        RAM: 512 Mb

        Recommended system requirements:

        OS: Windows Vista
        CPU: Intel Pentium T4400
        GPU: OpenGL 2.0 support
        RAM: 2 Gb

**/

int main(int argc, char* argv[]) {
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-help")) {
            printf("\nUsage:\n  gifdesk <command> [options]\n\n");
            printf("General Options:\n");
            printf("  -h, -help        Show help.\n");
            printf("  -s, -silent      Launch with no GUI.\n");
            return 0;
        }
    }

    Manager* manager = ManagerCreate(APP_NAME_VER, argc, argv);

    if (manager) ManagerRun(manager);

    return 0;
}
