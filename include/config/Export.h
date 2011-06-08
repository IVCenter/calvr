#ifndef WIN32
#    define CVRCONFIG_EXPORT
#else
#if defined( CVR_LIBRARY_STATIC )
#    define CVRCONFIG_EXPORT
#  elif defined( CVRCONFIG_LIBRARY )
#    define CVRCONFIG_EXPORT   __declspec(dllexport)
//#    define EXPIMP_TEMPLATE
#  else
#    define CVRCONFIG_EXPORT   __declspec(dllimport)
//#    define EXPIMP_TEMPLATE extern
#  endif
#endif

