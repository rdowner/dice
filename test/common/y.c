
int b;
extern int a;
extern int c;

yy()
{
    b = 2;
    printf("%d %d %d\n", a, b, c);
}
