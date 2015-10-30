/* shadowVIC – Copyright (c) 2015 Sven Michael Klose <pixel@hugbox.org> */

struct command {                                                                
    char * name;
    int    (*handler) (char * p);
};

char * skip_whitespace (char * p);
address parse_address (char * p);


#define COMMAND_NOT_RECOGNIZED  -1

int parse_command (struct command * commands, char * p);
