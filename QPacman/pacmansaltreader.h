/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANSALTREADER_H
#define PACMANSALTREADER_H

#include "pacmanprocessreader.h"

class PacmanSaltReader : public PacmanProcessReader {
    Q_OBJECT
public:
    explicit PacmanSaltReader(QObject *parent = 0);
    QByteArray salt() const { return m_salt; }

protected:
    void send_parameters();
    QByteArray command() const;

private slots:
    void on_salt_ready(const QByteArray & salt);

protected slots:
    void error(const QString &) {}

private:
    QByteArray m_salt;
};

#endif // PACMANSALTREADER_H
