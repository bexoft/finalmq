
#ifndef PROCESSSERVER_DEFINIES_H
#define	PROCESSSERVER_DEFINIES_H

enum ErrorReason
{
	// negative: ERemoreError
	ERRORREASON_NON						= 0,
	ERRORREASON_USERDEFINEDSTART		= 1000,
};



//#define	PROTOCOLID_REBOOT				"REBOOT"				// value = -
//#define	PROTOCOLID_FAIL					"FAIL"					// value = errorReason
//#define	PROTOCOLID_STATECHANGE			"STATECHANGE"			// value = state
//#define	PROTOCOLID_WATCHTHREADBLOCKED	"WATCHTHREADBLOCKED"	// value = -	
//#define	PROTOCOLID_KILLPROCESS			"KILLPROCESS"			// value = pid
//#define	PROTOCOLID_STARTTIMEOUT			"STARTTIMEOUT"			// value = pid	
//#define	PROTOCOLID_RECOVER				"RECOVER"				// value = state
//#define	PROTOCOLID_SUPRESSRECOVER		"SUPRESSRECOVER"		// value = state

#endif	// PROCESSSERVER_DEFINIES_H
