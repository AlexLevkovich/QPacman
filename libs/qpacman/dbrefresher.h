/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef DBREFRESHER_H
#define DBREFRESHER_H

#include "packageprocessor.h"

class DBRefresher : public PackageProcessor {
    Q_OBJECT
public:
    DBRefresher(ProgressView * view,QAction * cancelAction,QObject *parent = nullptr);

private slots:
    ThreadRun::RC process();
};

#endif // DBRefresher_H
