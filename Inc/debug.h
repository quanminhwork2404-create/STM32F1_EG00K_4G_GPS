#ifndef DEBUG_H
#define DEBUG_H

void DEBUG_Init(void);
void DEBUG_SendChar(char c);
void DEBUG_SendString(const char *s);

/* Gui chuoi co dinh dang, tuong tu printf */
void DEBUG_Printf(const char *format, ...);

#endif
