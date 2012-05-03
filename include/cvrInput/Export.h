#ifndef WIN32
#    define CVRINPUT_EXPORT
#else
#if defined( CVR_LIBRARY_STATIC )
#    define CVRINPUT_EXPORT
#  elif defined( CVRINPUT_LIBRARY )
#    define CVRINPUT_EXPORT   __declspec(dllexport)
//#    define EXPIMP_TEMPLATE
#  else
#    define CVRINPUT_EXPORT   __declspec(dllimport)
//#    define EXPIMP_TEMPLATE extern
#  endif
#endif

