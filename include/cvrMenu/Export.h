#ifndef WIN32
#    define CVRMENU_EXPORT
#else
#if defined( CVR_LIBRARY_STATIC )
#    define CVRMENU_EXPORT
#  elif defined( CVRMENU_LIBRARY )
#    define CVRMENU_EXPORT   __declspec(dllexport)
//#    define EXPIMP_TEMPLATE
#  else
#    define CVRMENU_EXPORT   __declspec(dllimport)
//#    define EXPIMP_TEMPLATE extern
#  endif
#endif

