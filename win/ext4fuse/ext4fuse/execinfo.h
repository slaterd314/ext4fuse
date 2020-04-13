#ifndef EXECINFO_H
#define EXECINFO_H


int backtrace(void** buffer, int size);
char** backtrace_symbols(void* const* buffer, int size);

#endif // EXECINFO_H
