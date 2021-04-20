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
    DBRefresher(ProgressView * view = NULL,QAction * cancelAction = NULL,QObject *parent = NULL);

private slots:
    ThreadRun::RC process(const QString & pw);
};

#endif // DBRefresher_H
