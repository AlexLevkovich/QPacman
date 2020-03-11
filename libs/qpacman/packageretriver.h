/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef PACKAGERETRIVER_H
#define PACKAGERETRIVER_H

#include <QObject>
#include "libalpm.h"

class PackageRetriver : public QObject {
    Q_OBJECT
public:
    PackageRetriver(QObject *parent = nullptr);

signals:
    void listing_packages_completed();

private slots:
    void processing();

};

#endif // PACKAGERETRIVER_H
