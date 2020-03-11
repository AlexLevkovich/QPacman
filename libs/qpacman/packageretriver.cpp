/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "packageretriver.h"
#include "static.h"
#include <QTimer>

PackageRetriver::PackageRetriver(QObject *parent) : QObject(parent) {
    connect(Alpm::instance(),SIGNAL(listing_packages_completed()),this,SIGNAL(listing_packages_completed()));
    connect(Alpm::instance(),SIGNAL(listing_packages_completed()),this,SLOT(deleteLater()));
    QTimer::singleShot(0,this,SLOT(processing()));
}

void PackageRetriver::processing() {
    Alpm::instance()->queryPackages();
}
