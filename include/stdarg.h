#ifndef STDARG_H_
#define STDARG_H_

typedef __builtin_va_list va_list;

#define va_start(v, t)	__builtin_va_start(v, t)
#define va_arg(v, t)	__builtin_va_arg(v, t)
#define va_end(v)	__builtin_va_end(v)

#endif	// STDARG_H_
