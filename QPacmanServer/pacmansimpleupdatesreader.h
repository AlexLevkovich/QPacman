/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANSIMPLEUPDATESREADER_H
#define PACMANSIMPLEUPDATESREADER_H

#include "pacmanprocessreader.h"

class PacmanSimpleUpdatesReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanSimpleUpdatesReader(QObject *parent = 0);
    QStringList packages() const {return m_packages;  }

protected:
    QString command() const;
    bool output(const QString & out);

private:
    QStringList m_packages;
};

#endif // PACMANSIMPLEUPDATESREADER_H
