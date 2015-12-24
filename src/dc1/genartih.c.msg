Fixed comma operator yet again... it was setting CONDACK even if the rhs
could not handle the COND.  Plus it was doing the wrong thing if the rhs
could not handle the COND.
