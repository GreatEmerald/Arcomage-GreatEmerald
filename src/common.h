#ifndef _COMMON_H_
#define _COMMON_H_ 1

#define CPUWAIT 10
#define ARCOVER "0.3.0"

#ifdef MSVC
	#define vsnprintf _vsnprintf
	#define inline __inline
#endif

	void FatalError(char *fmt,...);
	void OpenWebLink(char *web);

#ifdef linux
	#define OPERATINGSYSTEM 1
	#define CONFIGFILE "~/.config/arcomage.cfg"
	#define ARCODATADIR "/usr/share/arcomage/"
#endif

#ifdef WIN32
	#define OPERATINGSYSTEM 2
	#define CONFIGFILE ".\\arcomage.cfg"
	#define ARCODATADIR "data/"
#endif

#if !defined(linux) && !defined(WIN32)

#error Arcomage has been compiled and tested only under Windows and Linux
#error It would take some effort to compile on other platforms

#endif

#endif
