
char TmpBuf[256];

extern double IEEEDPFlt();
extern double IEEEDPAdd();
extern double IEEEDPMul();
extern double IEEEDPPow();

main(ac, av)
char *av[];
{
    double acc; 	/*  accumulated value	*/
    double ten = IEEEDPFlt(10);
    short x;

    /*
     *	.<TmpBuf> x 10^X    Convert a digit at a time.
     */

    strcpy(TmpBuf, av[1]);
    x = 0;

    clrmem(&acc, sizeof(acc));
    {
	char c;
	short i;

	for (i = 0; c = TmpBuf[i]; ++i) {
	    acc = IEEEDPAdd(IEEEDPMul(acc, ten), IEEEDPFlt(c - '0'));
	}
	/*x -= i;	  /*  adjust exponent */
    }
    acc = IEEEDPMul(acc, IEEEDPPow(IEEEDPFlt(atoi(av[2])), ten));

    printf("$%08lx,$%08lx\n", ((long *)&acc)[0], ((long *)&acc)[1]);
    fpa((float)acc, TmpBuf);
    puts(TmpBuf);
}


