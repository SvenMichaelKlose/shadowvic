/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

struct prg_info {
    char *  pathname;
    address load_addr;
    size_t  size;
    int     is_relocated;
};

void load_prg (struct prg_info *);
