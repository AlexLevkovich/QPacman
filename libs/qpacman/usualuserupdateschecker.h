/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef USUALUSERUPDATESCHECKER_H
#define USUALUSERUPDATESCHECKER_H

#include <QObject>

class Static;
class AlpmPackage;

class UsualUserUpdatesChecker : public QObject {
    Q_OBJECT
public:
    UsualUserUpdatesChecker(QObject * parent = NULL);
    QString lastError() const;
    QStringList updates() const;

    static const QString result(QStringList & updates);

signals:
    void ok(const QStringList & packages);
    void error(const QString & error);
    void database_updating();
    void getting_updates();

private slots:
    void process();

private:
    QString m_last_error;
    QStringList m_updates;
};

#endif // USUALUSERUPDATESCHECKER_H
