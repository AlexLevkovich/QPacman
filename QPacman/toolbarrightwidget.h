/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef TOOLBARRIGHTWIDGET_H
#define TOOLBARRIGHTWIDGET_H

#include <QWidget>
#include <QMenu>

namespace Ui {
class ToolbarRightWidget;
}

class ToolbarRightWidget : public QWidget {
    Q_OBJECT

public:
    explicit ToolbarRightWidget(QWidget *parent = 0);
    ~ToolbarRightWidget();

private slots:
    void onHelp();
    void onAboutDlg();

private:
    Ui::ToolbarRightWidget *ui;
    QMenu menu;
};

#endif // TOOLBARRIGHTWIDGET_H
