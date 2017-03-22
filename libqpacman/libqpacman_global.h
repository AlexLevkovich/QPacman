#ifndef LIBQPACMAN_GLOBAL_H
#define LIBQPACMAN_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LIBQPACMAN_LIBRARY)
#  define LIBQPACMANSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIBQPACMANSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIBQPACMAN_GLOBAL_H
