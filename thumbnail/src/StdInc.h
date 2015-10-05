//#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <atomic>

#include "Reg.h"
#include <strsafe.h>

#include <thumbcache.h>

#include <renderware.h>

#include "thumbprov.h"
#include "contextprov.h"

#include "rwwin32streamwrap.h"

extern HMODULE module_instance;
extern std::atomic <unsigned long> module_refCount;

extern rw::Interface *rwEngine;