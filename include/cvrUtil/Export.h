#ifndef WIN32
#    define CVRUTIL_EXPORT
#else
#if defined( CVR_LIBRARY_STATIC )
#    define CVRUTIL_EXPORT
#  elif defined( CVRUTIL_LIBRARY )
#    define CVRUTIL_EXPORT   __declspec(dllexport)
//#    define EXPIMP_TEMPLATE
#  else
#    define CVRUTIL_EXPORT   __declspec(dllimport)
//#    define EXPIMP_TEMPLATE extern
#  endif
#endif

