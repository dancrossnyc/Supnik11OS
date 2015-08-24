inline int abs(int i) { return (i < 0) ? -i : i; }
void *calloc(size_t num, size_t size);
void *malloc(size_t size);
void free(void *p);
int rand(void);
unsigned long strtoul(const char *s, char **endp, int radix);
