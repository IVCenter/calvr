#ifndef WIN32
#    define CVRCOLLAB_EXPORT
#else
#if defined( CVR_LIBRARY_STATIC )
#    define CVRCOLLAB_EXPORT
#  elif defined( CVRCOLLAB_LIBRARY )
#    define CVRCOLLAB_EXPORT   __declspec(dllexport)
//#    define EXPIMP_TEMPLATE
#  else
#    define CVRCOLLAB_EXPORT   __declspec(dllimport)
//#    define EXPIMP_TEMPLATE extern
#  endif
#endif