/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef ALPMOPTIONSWIDGET_H
#define ALPMOPTIONSWIDGET_H

#include "categorylistview.h"

namespace Ui {
class AlpmOptionsWidget;
}

class AlpmOptionsWidget : public CategoryWidget {
    Q_OBJECT

public:
    explicit AlpmOptionsWidget(QWidget *parent = nullptr);
    ~AlpmOptionsWidget();

public slots:
    void okPressed();

private slots:
    void on_proxyTypeCombo_activated(int index);

private:
    Ui::AlpmOptionsWidget *ui;
};

#endif // ALPMOPTIONSWIDGET_H
