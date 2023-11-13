
void
strcpy(d, s)
char *d;
char *s;
{
    while (*s) {
        *d = *s;
        ++s;
        ++d;
    }
    *d = 0;
}


