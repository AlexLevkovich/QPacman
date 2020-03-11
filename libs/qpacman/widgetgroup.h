/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef WIDGETGROUP_H
#define WIDGETGROUP_H

#include <QObject>

class QWidget;

class WidgetGroup : public QObject {
    Q_OBJECT
public:
    WidgetGroup(QObject *parent);
    bool add(QWidget * wnd);
    bool remove(QWidget * wnd);
    void clear();
    QList<QWidget *> widgets() const;
    bool setCurrent(QWidget * wnd);

signals:
    void selected(QWidget * wnd);

private slots:
    void destroyed(QObject * obj);

private:
    QList<QWidget *> m_widgets;
};

#endif // WIDGETGROUP_H
