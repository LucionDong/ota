#ifndef UUID_HELP_H
#define UUID_HELP_H

void uuid_to_string(const unsigned char *uuid, char *buf, char const *restrict fmt);
void uuid_unparser(const unsigned char *uuid, char *buf);

#endif