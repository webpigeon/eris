#ifndef ERIS_POLL_GLIB_VERSION_H
#define ERIS_POLL_GLIB_VERSION_H

#include <glib.h>

#if (GLIB_MAJOR_VERSION == 2) || (GLIB_MAJOR_VERSION == 1 && GLIB_MINOR_VERSION == 3)
#define ERIS_POLL_GLIB_2_0
#endif

#endif // ERIS_POLL_GLIB_VERSION_H