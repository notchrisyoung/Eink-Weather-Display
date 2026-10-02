#pragma once
// Pulls in secrets.h if you've created it, otherwise the placeholder example
// (which builds fine but won't connect).
#if __has_include("secrets.h")
#include "secrets.h"
#else
#warning "include/secrets.h not found - copy secrets.example.h to secrets.h and fill it in"
#include "secrets.example.h"
#endif
