#ifndef WIN32
#    define CVRKERNEL_EXPORT
#else
#if defined( CVR_LIBRARY_STATIC )
#    define CVRKERNEL_EXPORT
#  elif defined( CVRKERNEL_LIBRARY )
#    define CVRKERNEL_EXPORT   __declspec(dllexport)
//#    define EXPIMP_TEMPLATE
#  else
#    define CVRKERNEL_EXPORT   __declspec(dllimport)
//#    define EXPIMP_TEMPLATE extern
#  endif
#endif

