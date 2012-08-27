#include <cstdio>
#include <kernel/die.hpp>

#include <kernel/c++/cxxabi.hpp>

extern "C"
void __cxxabiv1::__cxa_pure_virtual()
{
    die("/!\\ pure virtual called\n");
}
