/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef EXCLUSIVEACTIONGROUP_H
#define EXCLUSIVEACTIONGROUP_H

#include <QActionGroup>

class ExclusiveActionGroup : public QActionGroup {
    Q_OBJECT
public:
    ExclusiveActionGroup(QObject *parent);

private slots:
    void ontriggered(QAction * action);
};

#endif // EXCLUSIVEACTIONGROUP_H
