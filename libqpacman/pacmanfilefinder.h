/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanFileFinder_H
#define PacmanFileFinder_H

#include "pacmanprocessreader.h"
#include "libqpacman_global.h"

class LIBQPACMANSHARED_EXPORT PacmanFileFinder : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanFileFinder(const QString & filePath,QObject *parent = 0);
    inline QString package() const { return m_package; }

protected:
    QString command() const;

protected slots:
    bool output(const QString & out);
    void onFinished(int code,QProcess::ExitStatus status);

private:
    QString m_filePath;
    QString m_searchStr;
    QString m_package;
};

#endif // PacmanFileFinder_H
