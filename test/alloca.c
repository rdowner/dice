
#include <alloca.h>

main()
{
    char a[4] = { 1, 2, 3, 4 };
    char *ptr;

    ptr = alloca(5);
    ptr[0] = 10;
    ptr[1] = 11;
    ptr[2] = 12;
    ptr[3] = 13;
    ptr[4] = 14;
    puts("hello");

    printf("FUBAR %d %d %d %d : %d %d %d %d %d\n",
        a[0], a[1], a[2], a[3],
        ptr[0], ptr[1], ptr[2], ptr[3], ptr[4]
    );
}

