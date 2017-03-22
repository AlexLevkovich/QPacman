/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QDialog>
#include "libqpacman_global.h"

namespace Ui {
class LogWindow;
}

class LIBQPACMANSHARED_EXPORT LogWindow : public QDialog {
    Q_OBJECT

public:
    explicit LogWindow(QWidget *parent = 0);
    ~LogWindow();

protected:
    void hideEvent(QHideEvent * event);

signals:
    void hidden();

public slots:
    void addText(const QString & text);

private:
    Ui::LogWindow *ui;
};

#endif // LOGWINDOW_H
