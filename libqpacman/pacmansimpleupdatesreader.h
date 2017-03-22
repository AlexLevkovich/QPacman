/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANSIMPLEUPDATESREADER_H
#define PACMANSIMPLEUPDATESREADER_H

#include "pacmanprocessreader.h"
#include "libqpacman_global.h"

class LIBQPACMANSHARED_EXPORT PacmanSimpleUpdatesReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanSimpleUpdatesReader(QObject *parent = 0);
    QStringList packages() const {return m_packages;  }

protected:
    QString command() const;
    bool output(const QString & out);
    inline bool showErrorMessageAtExit() {
        return false;
    }

protected slots:
    void onFinished(int code,QProcess::ExitStatus status);

private:
    QStringList m_packages;
};

#endif // PACMANSIMPLEUPDATESREADER_H
