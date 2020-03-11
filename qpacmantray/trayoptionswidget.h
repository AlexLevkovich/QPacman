/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef TRAYOPTIONSWIDGET_H
#define TRAYOPTIONSWIDGET_H


#include "categorylistview.h"

namespace Ui {
class TrayOptionsWidget;
}

class TrayOptionsWidget : public CategoryWidget {
    Q_OBJECT

public:
    explicit TrayOptionsWidget(QWidget *parent = nullptr);
    ~TrayOptionsWidget();
    int interval() const;
    int errInterval() const;

public slots:
    void okPressed();

private:
    Ui::TrayOptionsWidget *ui;
};

#endif // TRAYOPTIONSWIDGET_H
