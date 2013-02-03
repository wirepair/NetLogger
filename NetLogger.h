#ifndef NETLOGGER_H
#define NETLOGGER_H

#include "pin.H"
#include <iostream>
#include <fstream>


namespace WINDOWS {
#include "Winsock2.h"
#include "Windows.h"
}

// Helpers
VOID PrintHexBuffer( const char *buf, const int size, const bool onlyAscii );

// Hooks
VOID HookRecvFrom( IMG img );
VOID HookRecv( IMG img );

static FILE * LogFile;

#endif // NETLOGGER_H