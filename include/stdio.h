#include <stdarg.h>

typedef void FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

#define EOF -1

enum
{
	SEEK_SET,
	SEEK_END,
};

void clearerr(FILE *fd);
int fclose(FILE *fd);
int feof(FILE *fd);
int ferror(FILE *fd);
int fflush(FILE *fd);
int fgetc(FILE *fd);
char *fgets(char *dst, size_t limit, FILE *fd);
FILE *fopen(const char *filename, const char *mode);
void fprintf(FILE *fd, const char *fmt, ...);
int fputc(int c, FILE *fd);
size_t fread(void *p, size_t size, size_t n, FILE *fd);
int fseek(FILE *fd, long off, int whence);
long ftell(FILE *fd);
size_t fwrite(const void *p, size_t size, size_t n, FILE *fd);
void perror(const char *s);
void printf(const char *fmt, ...);
void rewind(FILE *stream);
int sscanf(const char *s, const char *fmt, ...);
int sprintf(char *dst, const char *fmt, ...);
int vsprintf(char *dst, const char *fmt, va_list ap);
