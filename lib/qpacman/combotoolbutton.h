/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef COMBOTOOLBUTTON_H
#define COMBOTOOLBUTTON_H

#include <QToolButton>
#include <QAction>

class QChildEvent;

class ComboToolButton : public QToolButton {
    Q_OBJECT
public:
    explicit ComboToolButton(QWidget *parent = 0);
    QAction * currentAction() const;
    virtual QString iconText() const;
    void setMenu(QMenu *menu);

protected:
    void keyPressEvent(QKeyEvent * event);
    bool eventFilter(QObject * obj,QEvent * event);

protected slots:
    virtual void onMenuSelected(QAction * action);

private:
    QObject * menuObject;
    QAction * curr_action;
    QString currActionText;
};
Q_DECLARE_METATYPE(QAction *)

#endif // COMBOTOOLBUTTON_H
