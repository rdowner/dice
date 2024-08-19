/*  Matt,

    Here's an update of the ASL file requester mods I sent to you a few
    months ago. I've made a few enhancements:

    1.) The size and placement of the ASL requester is saved as part of
	the DME.CONFIG file. This required expanding the data saved from
	Ep. The additions to DEFS.H are included below. It's probably
	best to delete the old DME.CONFIG before running the new DME,
	but it won't hurt ASL if you pass it bad coordinates and
	loadconfig() doesn't choke on the old, smaller config file.

    2.) If you have DME open on a public screen that is not the default,
	then the ASL requesters do not appear in the proper place. This
	has been fixed with the addition of a few tags. However, each
	requester still creates it's own IDCMP. (It just occured to me
	that some of the "normal" requester might need some work also.)

    3.) I added a kludge: fixdirname(). This puts an initial directory
	path into the requester so that it displays something in the
	"Drawer" gadget when it's opened. The splitpath() function
	always seems to return a null DIR string when using Ep->Name. I
	find the DIR string by using NameFromLock() on Ep->dirlock. This
	call is 2.0 only, so I didn't add it to the ARP requesters.
	(Actually, I removed the ARP commands from my copy of DME.)

    The other file in this archive is a new filereq.c. What follows are
    the changes to defs.h.

    --fran
 */
    int     TPen;	    /* The old LAST element in the config area */

    /* ADD THE FOLLOWING 4 LINES TO DEFS.H, AFTER TPen */

    short   ASLleft;	    /* NEW: save position of ASL requester */
    short   ASLtop;	    /* NEW: save position of ASL requester */
    short   ASLwidth;	    /* NEW: save position of ASL requester */
    short   ASLheight;	    /* NEW: save position of ASL requester */

    char    EndConfig;
} ED;

