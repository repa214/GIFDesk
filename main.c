#include "gifdesk.h"

int main()
{
    setlocale(LC_NUMERIC, "C");

    GIFDesk app;
    int c = Run(&app);

    return c;
}
