/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef REPOTOOLBUTTON_H
#define REPOTOOLBUTTON_H

#include "combotoolbutton.h"
#include <QStringList>

class RepoToolButton : public ComboToolButton {
    Q_OBJECT
public:
    explicit RepoToolButton(QWidget *parent = 0);
    void fill(const QStringList & repos);
    void selectAllMenuItem();
    void selectMenuItem(const QString & repo);
    QString iconText() const;

private slots:
    void onMenuItemSelected(QAction * action);

signals:
    void selected(const QString & repo);

private:
    static QString RepoAll_Str;
};

#endif // REPOTOOLBUTTON_H
