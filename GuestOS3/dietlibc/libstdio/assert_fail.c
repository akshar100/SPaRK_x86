#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dietwarning.h"
#include <write12.h>

void __assert_fail (const char *assertion, const char *file, unsigned int line, const char *function);

void __assert_fail (const char *assertion, const char *file, unsigned int line, const char *function)
{
}

link_warning("__assert_fail","warning: your code still has assertions enabled!")
