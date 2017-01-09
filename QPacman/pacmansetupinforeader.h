/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanSetupInfoReader_H
#define PacmanSetupInfoReader_H

#include "pacmanprocessreader.h"

class PacmanSetupInfoReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanSetupInfoReader(QObject *parent = 0);

    static QString pacman_cache_dir;
    static QString pacman_conf;
    static QString pacman_lock_file;
    static QString pacman_db_path;

protected slots:
    bool error(const QString & err);
    bool output(const QString & out);

protected slots:
    void onFinished(int code,QProcess::ExitStatus status);

protected:
    QString command() const;
};

#endif // PacmanSetupInfoReader_H
