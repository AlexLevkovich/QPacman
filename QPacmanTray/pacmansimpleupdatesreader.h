/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANSIMPLEUPDATESREADER_H
#define PACMANSIMPLEUPDATESREADER_H

#include "pacmanprocessreader.h"
#include <QStringList>

class PacmanSimpleUpdatesReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanSimpleUpdatesReader(QObject *parent = 0);
    QStringList packages() const {return m_packages; }

protected:
    void send_parameters();
    QByteArray command() const;

private slots:
    void on_packages_ready(const QStringList & packages);

private:
    QStringList m_packages;
};

#endif // PACMANSIMPLEUPDATESREADER_H
