/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef COMBOTOOLBUTTON_H
#define COMBOTOOLBUTTON_H

#include <QToolButton>

class QAction;
class QShowEvent;

class ComboToolButton : public QToolButton {
    Q_OBJECT
public:
    explicit ComboToolButton(QWidget *parent = 0);
    QAction * currentAction();
    QString iconText() const;

protected:
    void showEvent(QShowEvent * event);
    void keyPressEvent(QKeyEvent * event);

protected slots:
    virtual void onMenuSelected(QAction * action);

private:
    bool isFirstTime;
    QAction * curr_action;
};

#endif // COMBOTOOLBUTTON_H
