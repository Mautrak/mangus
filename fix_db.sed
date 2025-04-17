# sed script for src/db.c
# BACK UP YOUR FILE FIRST!

# Fix -Wstrict-prototypes
3996s/()/ ( void )/
3660s/()/ ( void )/

# Fix -Wunused-parameter (inserting (void) cast)
# Line numbers are FUNCTION DEFINITION lines + 2
3455i\
    (void)ch;
3455i\
    (void)argument;
3425i\
    (void)argument;
3410i\
    (void)argument;