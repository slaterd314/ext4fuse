

int backtrace(void** buffer, int size)
{
	((void)buffer);
	((void)size);
	return 0;
}

char** backtrace_symbols(void* const* buffer, int size)
{
	((void)buffer);
	((void)size);
	return (char **)0;
}