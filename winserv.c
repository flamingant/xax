#ifdef __cplusplus /*Z*/
extern "C" {
#endif

// Beeper Service.cpp

// Copyright (c) 2001-2002 Jarmo Muukka. All rights reserved.

#include <windows.h>

TCHAR* serviceName = TEXT("Beeper Service");
SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle = 0;
HANDLE stopServiceEvent = 0;

#include	"uf.h"

void WINAPI ServiceControlHandler( DWORD controlCode )
{
	switch ( controlCode )
	{
		case SERVICE_CONTROL_INTERROGATE:
			break;

		case SERVICE_CONTROL_SHUTDOWN:
		case SERVICE_CONTROL_STOP:
			serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
			SetServiceStatus( serviceStatusHandle, &serviceStatus );

			SetEvent( stopServiceEvent );
			ufs_quit() ;
			return;

		case SERVICE_CONTROL_PAUSE:
			break;

		case SERVICE_CONTROL_CONTINUE:
			break;

		default:
			if ( controlCode >= 128 && controlCode <= 255 )
				// user defined control code
				break;
			else
				// unrecognised control code
				break;
	}

	SetServiceStatus( serviceStatusHandle, &serviceStatus );
}

void WINAPI ServiceLoop(void)
{
    ufs_loop() ;
    }

void WINAPI ServiceMain( DWORD argc, TCHAR* argv[] )
{
	// initialise service status
	serviceStatus.dwServiceType = SERVICE_WIN32;
	serviceStatus.dwCurrentState = SERVICE_STOPPED;
	serviceStatus.dwControlsAccepted = 0;
	serviceStatus.dwWin32ExitCode = NO_ERROR;
	serviceStatus.dwServiceSpecificExitCode = NO_ERROR;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;

	serviceStatusHandle = RegisterServiceCtrlHandler( serviceName, ServiceControlHandler );

	if ( serviceStatusHandle )
	{
		// service is starting
		serviceStatus.dwCurrentState = SERVICE_START_PENDING;
		SetServiceStatus( serviceStatusHandle, &serviceStatus );

		// do initialisation here
		stopServiceEvent = CreateEvent( 0, FALSE, FALSE, 0 );

		// running
		serviceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		serviceStatus.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus( serviceStatusHandle, &serviceStatus );

		ServiceLoop() ;

		// service was stopped
		serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus( serviceStatusHandle, &serviceStatus );

		// do cleanup here
		CloseHandle( stopServiceEvent );
		stopServiceEvent = 0;

		// service is now stopped
		serviceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus( serviceStatusHandle, &serviceStatus );
	}
}

void RunService()
{
	SERVICE_TABLE_ENTRY serviceTable[] =
	{
		{ serviceName, ServiceMain },
		{ 0, 0 }
	};

	StartServiceCtrlDispatcher( serviceTable );
}

#define _MAX_PATH	1024

/* the service is installed to run this exectable with "-mhttp --service-start"
   when program is invoked in this way it finds its way to RunService()
   This is done explicitly in the program in response to this command line
   RunService() creates a SERVICE_TABLE_ENTRY referencing ServiceMain which is
   our entry point to the service code startup.
   This registers a ServiceCtrlHandler which is how we wait for control, e.g
   stop, pause etc. Then we init,loop,cleanup.
   The original ServiceLoop() did nothing. It must be modified to call ufs_loop
   and we can add windows specific poll using Events etc.

   When running as Local System account and allowed to interact, emacs fails
   When running as nick, notepad etc don't open on desktop
   
   So, until this problem is solved, we can't run our little server as a service
   Which isn't the end of the world, we can just start it when we reboot

   The http server that runs our emacshelper must not serve other requests.

   */

void InstallService()
{
	SC_HANDLE serviceControlManager = OpenSCManager( 0, 0, SC_MANAGER_CREATE_SERVICE );

	if ( serviceControlManager )
	{
		TCHAR path[ _MAX_PATH + 1 ];
		if ( GetModuleFileName( 0, path, sizeof(path)/sizeof(path[0]) ) > 0 )
		{
			SC_HANDLE service ;
			strcat(path," -de:/.p/_/T/torrent/deluge/tc/.bwg -mhttp --service-start=y --log-file=squee.log") ;
			service = CreateService( serviceControlManager,
							serviceName, serviceName,
							SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
							SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path,
							0, 0, 0, 0, 0 );
			if ( service )
				CloseServiceHandle( service );
		}

		CloseServiceHandle( serviceControlManager );
	}
}

void UninstallService()
{
	SC_HANDLE serviceControlManager = OpenSCManager( 0, 0, SC_MANAGER_CONNECT );

	if ( serviceControlManager )
	{
		SC_HANDLE service = OpenService( serviceControlManager,
			serviceName, SERVICE_QUERY_STATUS | DELETE );
		if ( service )
		{
			SERVICE_STATUS serviceStatus;
			if ( QueryServiceStatus( service, &serviceStatus ) )
			{
				if ( serviceStatus.dwCurrentState == SERVICE_STOPPED )
					DeleteService( service );
			}

			CloseServiceHandle( service );
		}

		CloseServiceHandle( serviceControlManager );
	}
}


#ifdef __cplusplus /*Z*/
}
#endif
