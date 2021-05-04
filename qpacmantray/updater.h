#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QTimer>
#include "qalpmtypes.h"

class ProgressView;
class QPlainTextEdit;
class QAction;
class QMainWindow;

class Updater : public QObject {
    Q_OBJECT
public:
    explicit Updater(QObject *parent = nullptr);

signals:
    void completed(ThreadRun::RC rc);

private slots:
    void onInstallerCompleted(ThreadRun::RC rc);
    void onupdate_method_finished(const QString &,ThreadRun::RC rc);

private:
    QTimer m_timer;
    QMetaObject::Connection conn1;
    QMetaObject::Connection conn2;

    ProgressView * progressView;
    QPlainTextEdit * logView;
    QAction * cancelAction;
    QAction * logAction;
    QMainWindow * updateWindow;
};

#endif // UPDATER_H
