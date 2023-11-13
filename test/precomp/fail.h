
  struct fail1 {
          struct fail1 *nextfail1;
          char a1 [0x02];
          char b1 [0x01]; /* hex consts in structure defs are no good for
                          * precompiling sometimes:-(( */
          };

  struct fail2 {
          struct fail2 *nextfail2;
          char a1 [0x02];
          char b1 [0x02]; /* hex consts in structure defs are no good for
                          * precompiling sometimes:-(( */
          };

