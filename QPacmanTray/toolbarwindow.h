/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef TOOLBARWINDOW_H
#define TOOLBARWINDOW_H

#include <QMainWindow>

namespace Ui {
class ToolbarWindow;
}

class QShowEvent;

class ToolbarWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ToolbarWindow(QWidget *parent = 0);
    ~ToolbarWindow();

protected:
    void showEvent(QShowEvent * event);

signals:
    void actionCheck_triggered();
    void actionUpdate_triggered();
    void actionErrors_triggered();

private slots:
    void actionCheckUIUpdate(bool flag);
    void actionUpdateUIUpdate(bool flag);
    void actionErrorsUIUpdate(bool flag);

private:
    Ui::ToolbarWindow *ui;
};

#endif // TOOLBARWINDOW_H
