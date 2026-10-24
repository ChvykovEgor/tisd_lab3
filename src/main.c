#include "errors.h"
#include "menu.h"
#include <time.h>
#include <stdio.h>

int main()
{
    srand((unsigned) time(NULL));
    error_code rc = menu_process();
    if (rc == ERR_MEMORY_ALLOC)
        printf("Ошибка выделения памяти");
    return rc;
}