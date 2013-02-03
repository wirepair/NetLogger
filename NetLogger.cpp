#include "NetLogger.h"
/*
NetLogger:
================================

Author: @_wirepair / isaac.dawson{}gmail.com.

Logs sent and received buffers to a file, hooks send, recv, sendto and recvfrom.
Writes hex bytes + ascii out to disk, optionally allowing for writing only ascii.
Only really tested in Windows.

How to build:
-------------------------

Copy this project into your pin source directory:
%pin%\source\tools\NetLogger
Open Visual Studio (2008) and build.

How to run:
-------------------------

Hooks netcat's network functions and writes to bonk.log:

c:\pin\pin.exe -t C:\pin\NetLogger.dll -o bonk.log -- nc.exe -l -v -p 999

Log only ascii strings, careful if they are not null terminated we will probably crash.

c:\pin\pin.exe -t C:\pin\NetLogger.dll -a - donks.log -- nc.exe -l -v -p 999
*/

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
                "o", "donks.log", "where to write captured packet data");
KNOB<BOOL>   KnobAsciiMode(KNOB_MODE_WRITEONCE, "pintool", "a", "0", "logs ascii only");

// Replacements
// UDP
static WINDOWS::INT replacementUDP(
	AFUNPTR functionUDP,
	WINDOWS::SOCKET s,
	WINDOWS::CHAR *buf,
	WINDOWS::INT len,
	WINDOWS::INT flags,
	WINDOWS::SOCKADDR *from,
	WINDOWS::INT *fromlen,
	CONTEXT *ctx,
	WINDOWS::CHAR *functionName
	)
{
	WINDOWS::INT retval = 0;
	
	PIN_CallApplicationFunction(
		ctx, 
		PIN_ThreadId(),
		CALLINGSTD_STDCALL, functionUDP,
		PIN_PARG(WINDOWS::INT), &retval,
		PIN_PARG(WINDOWS::SOCKET), s,
		PIN_PARG(WINDOWS::CHAR *), buf,
		PIN_PARG(WINDOWS::INT), len,
		PIN_PARG(WINDOWS::INT), flags,
		PIN_PARG(WINDOWS::SOCKADDR *), from,
		PIN_PARG(WINDOWS::INT *), fromlen,
		PIN_PARG_END()
		);
	if ( retval != -1 ) 
	{
		fprintf( LogFile, "%s [%d]:\r\n", functionName, retval );
		fprintf( LogFile, "{\r\n" );
		PrintHexBuffer( buf, retval, KnobAsciiMode );
		fprintf( LogFile, "}\r\n" );
	}
	fflush( LogFile );
	return retval;
}


// TCP
static WINDOWS::INT replacementTCP(
	AFUNPTR functionTCP, 
	WINDOWS::SOCKET s,
	WINDOWS::CHAR *buf,
	WINDOWS::INT len,
	WINDOWS::INT flags,
	CONTEXT *ctx,
	WINDOWS::CHAR * functionName
	)
{
	WINDOWS::INT retval = 0;
	
	PIN_CallApplicationFunction(
		ctx, 
		PIN_ThreadId(),
		CALLINGSTD_STDCALL, functionTCP, // send or recv
		PIN_PARG(WINDOWS::INT), &retval, // return value first
		PIN_PARG(WINDOWS::SOCKET), s, 
		PIN_PARG(WINDOWS::CHAR *), buf,
		PIN_PARG(WINDOWS::INT), len,
		PIN_PARG(WINDOWS::INT), flags,
		PIN_PARG_END()
		);
	
	
	if ( retval != -1 ) 
	{
		fprintf( LogFile, "%s [%d]:\r\n", functionName, retval );
		fprintf( LogFile, "{\r\n" );
		PrintHexBuffer( buf, retval, KnobAsciiMode );
		fprintf( LogFile, "}\r\n" );
	}
	fflush( LogFile );
	return retval;
}

static WINDOWS::INT replacementWSASendTo(
	AFUNPTR functionWSASendTo, 
	WINDOWS::SOCKET s,
	WINDOWS::LPWSABUF lpBuffers,
	WINDOWS::DWORD dwBufferCount,
	WINDOWS::LPDWORD lpNumberOfBytesSend,
	WINDOWS::DWORD dwFlags,
	WINDOWS::SOCKADDR * lpTo,
	WINDOWS::INT iToLen,
	WINDOWS::LPWSAOVERLAPPED lpOverlapped,
	WINDOWS::LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine,
	CONTEXT *ctx,
	WINDOWS::CHAR * functionName
	)
{
	WINDOWS::INT retval = 0;
	
	PIN_CallApplicationFunction(
		ctx, 
		PIN_ThreadId(),
		CALLINGSTD_STDCALL, functionWSASendTo, // send or recv
		PIN_PARG(WINDOWS::INT), &retval, // return value first
		PIN_PARG(WINDOWS::SOCKET), s,
		PIN_PARG(WINDOWS::LPWSABUF), lpBuffers,
		PIN_PARG(WINDOWS::DWORD), dwBufferCount,
		PIN_PARG(WINDOWS::LPDWORD), lpNumberOfBytesSend,
		PIN_PARG(WINDOWS::DWORD), dwFlags,
		PIN_PARG(WINDOWS::SOCKADDR *), lpTo,
		PIN_PARG(WINDOWS::INT),	iToLen,
		PIN_PARG(WINDOWS::LPWSAOVERLAPPED),	lpOverlapped,
		PIN_PARG(WINDOWS::LPWSAOVERLAPPED_COMPLETION_ROUTINE), lpCompletionRoutine,
		PIN_PARG_END()
		);
	
	
	if ( retval != -1 ) 
	{
		fprintf( LogFile, "%s [%d]:\r\n", functionName, retval );
		fprintf( LogFile, "{\r\n" );
		PrintHexBuffer( (char *)lpBuffers, retval, KnobAsciiMode );
		fprintf( LogFile, "}\r\n" );
	}
	fflush( LogFile );
	return retval;
}

// Helper Functions
VOID PrintHexBuffer( const char *buf, const int size, const bool onlyAscii )
{
	int row, column, i = 0;
	
	// You better be damn sure this ends with a \0.
	if ( onlyAscii )
	{
		fprintf( LogFile, "%s\r\n", buf );
		return;
	}

	for ( row = 0; ( i + 1 ) < size; row++ )
	{
		// hex
		for( column = 0; column < 16; column++ )
		{
			i = row * 16 + column;
			if ( column == 8 )
			{
				fprintf(LogFile, " " );
			}

			if ( i < size )
			{
				fprintf( LogFile, "%02X", (unsigned char)buf[i] );
			}
			else
			{
				fprintf( LogFile, "  " );
			}
			fprintf( LogFile, " " );
		}
		// ascii 
		for( column = 0; column < 16; column++ )
		{
			i = row * 16 + column;
			if ( column == 8 )
			{
				fprintf( LogFile, " " );
			}
			if( i < size )
			{
				if( buf[i] > 0x20 && buf[i] < 0x7F )
				{
					fprintf( LogFile, "%c", buf[i] );
				}
				else 
				{
					fprintf( LogFile, "." );
				}
			}
			else
			{
				break;
			}
		}
		fprintf( LogFile, "\r\n" );
	}
}

VOID Finish( int ignored, VOID *arg )
{
	fflush( LogFile );
	fclose( LogFile );
}

UINT32 Usage()
{
	std::cout << "Records socket communications, good for sniffing localhost.";
	std::cout << std::endl;
	std::cout << KNOB_BASE::StringKnobSummary();
	std::cout << std::endl;
	return 2;
}
// Hooks
VOID HookUDP( const IMG img, const AFUNPTR replacement, const char *functionName )
{
	std::cerr << "Found " << functionName << "..." << endl;
	RTN rtn = RTN_FindByName(img, functionName );

	PROTO proto = 
		PROTO_Allocate( PIN_PARG(WINDOWS::INT),		// retval
				CALLINGSTD_STDCALL,
				functionName,
				PIN_PARG(WINDOWS::SOCKET),			// s,
				PIN_PARG(WINDOWS::CHAR *),			// buf,
				PIN_PARG(WINDOWS::INT),				// len
				PIN_PARG(WINDOWS::INT),				// flags
				PIN_PARG(WINDOWS::SOCKADDR *),		// from|to,
				PIN_PARG(WINDOWS::INT *),			// fromlen|tolen,
				PIN_PARG_END()
				);

	RTN_ReplaceSignature(rtn, replacement,
			IARG_PROTOTYPE, proto,
			IARG_ORIG_FUNCPTR,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 4,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 5,
			IARG_CONTEXT,
			IARG_PTR, functionName,
			IARG_END
			);
	std::cerr << "Replaced " << RTN_Name( rtn ) << " and stuff. " << endl;
	PROTO_Free(proto);
}
// WSASendTo HOOK Untested, I think. I forget :/.
VOID HookWSASendTo( const IMG img, AFUNPTR replacement, const char *functionName ) 
{
	std::cerr << "Found " << functionName << "..." << endl;
	RTN rtn = RTN_FindByName(img, functionName );

	PROTO proto = 
		PROTO_Allocate( PIN_PARG(WINDOWS::INT),   // retval
				CALLINGSTD_STDCALL,
				functionName,
				PIN_PARG(WINDOWS::SOCKET),			// s,
				PIN_PARG(WINDOWS::LPWSABUF),		// lpBuffers,
				PIN_PARG(WINDOWS::DWORD),			// dwBufferCount
				PIN_PARG(WINDOWS::LPDWORD),			// lpNumberOfBytesSend
				PIN_PARG(WINDOWS::DWORD),			// dwFlags
				PIN_PARG(WINDOWS::SOCKADDR *),		// lpTo,
				PIN_PARG(WINDOWS::INT),				// iToLen,
				PIN_PARG(WINDOWS::LPWSAOVERLAPPED),	// lpOverlapped
				PIN_PARG(WINDOWS::LPWSAOVERLAPPED_COMPLETION_ROUTINE), // lpCompletionRoutine
				PIN_PARG_END()
				);

	RTN_ReplaceSignature(rtn, replacement,
			IARG_PROTOTYPE, proto,
			IARG_ORIG_FUNCPTR,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 4,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 5,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 6,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 7,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 8,
			IARG_CONTEXT,
			IARG_PTR, functionName,
			IARG_END
			);
	std::cerr << "Replaced " << RTN_Name( rtn ) << endl;
	PROTO_Free(proto);
}

VOID HookRecvFrom( const IMG img )
{
	HookUDP( img, (AFUNPTR)replacementUDP, "recvfrom" );
}

VOID HookSendTo( const IMG img )
{
	HookUDP( img, (AFUNPTR)replacementUDP, "sendto" );
}

VOID HookTCP( const IMG img, const AFUNPTR replacement, const char *functionName )
{
	fprintf(LogFile, "Found %s...\r\n", functionName );
	std::cerr << "Found " << functionName << "..." << endl;

	// hook 
	RTN rtn = RTN_FindByName( img, functionName );

	PROTO proto = 
		PROTO_Allocate( PIN_PARG(WINDOWS::INT),
				CALLINGSTD_STDCALL,
				functionName,
				PIN_PARG(WINDOWS::SOCKET),			// s,
				PIN_PARG(WINDOWS::CHAR *),			// buf,
				PIN_PARG(WINDOWS::INT),				// len
				PIN_PARG(WINDOWS::INT),				// flags
				PIN_PARG_END()
				);
	
	RTN_ReplaceSignature( rtn, replacement,
			IARG_PROTOTYPE, proto,
			IARG_ORIG_FUNCPTR,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
			IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
			IARG_CONTEXT,
			IARG_PTR, functionName,
			IARG_END
			);

	PROTO_Free( proto );
}

VOID HookSend( const IMG img )
{
	HookTCP( img, (AFUNPTR)replacementTCP, "send" );	
}
VOID HookRecv( const IMG img )
{
	HookTCP( img, (AFUNPTR)replacementTCP, "recv" );
}


VOID ImageLoad( IMG img, VOID *v )
{
	RTN rtn;
	fprintf( LogFile, "Loaded %s\r\n", IMG_Name( img ).c_str() );
	// UDP
	if ( (rtn = RTN_FindByName( img, "recvfrom" )) != RTN_Invalid() )
	{
		HookRecvFrom( img );
	}
	if ( (rtn = RTN_FindByName( img, "sendto" )) != RTN_Invalid() )
	{
		HookSendTo( img );
	}

	// TCP	
	if ( (rtn = RTN_FindByName( img, "recv" )) != RTN_Invalid() )
	{
		HookRecv( img );
	}
	if ( (rtn = RTN_FindByName( img, "send" )) != RTN_Invalid() )
	{
		HookSend( img );
	}
	/*
	Uncomment if necessary, untested though!
	if ( (rtn = RTN_FindByName( img, "WSASendTo" )) != RTN_Invalid() )
	{
		HookWSASendTo( img, (AFUNPTR)replacementWSASendTo, "WSASendTo" );
	}
	*/
}

int main( int argc, char **argv )
{
	PIN_InitSymbols();

	if ( PIN_Init(argc, argv) )
		return Usage();

	LogFile = fopen( KnobOutputFile.Value().c_str(), "wb" );
	IMG_AddInstrumentFunction( ImageLoad, NULL );
	PIN_AddFiniFunction( Finish, NULL );
	PIN_StartProgram();

	return 0;
}