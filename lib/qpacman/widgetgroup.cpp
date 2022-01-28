/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "widgetgroup.h"
#include <QWidget>

WidgetGroup::WidgetGroup(QObject *parent) : QObject(parent) {}

bool WidgetGroup::add(QWidget * wnd) {
    if (wnd == NULL || m_widgets.contains(wnd)) return false;
    m_widgets.append(wnd);
    wnd->setVisible(false);
    connect(wnd,&QObject::destroyed,this,&QObject::destroyed);
    return true;
}

void WidgetGroup::destroyed(QObject * obj) {
    m_widgets.removeAll((QWidget *)obj);
}

bool WidgetGroup::remove(QWidget * wnd) {
    bool ret = (m_widgets.removeAll(wnd) > 0);
    if (ret) disconnect(wnd,&QObject::destroyed,this,&QObject::destroyed);
    return ret;
}

void WidgetGroup::clear() {
    while (!m_widgets.isEmpty()) disconnect(m_widgets.takeFirst(),&QObject::destroyed,this,&QObject::destroyed);
}

QList<QWidget *> WidgetGroup::widgets() const {
    return m_widgets;
}

bool WidgetGroup::setCurrent(QWidget * wnd) {
    if (!m_widgets.contains(wnd)) return false;
    for (int i=0;i<m_widgets.count();i++) {
        m_widgets[i]->setVisible(m_widgets[i] == wnd);
    }
    emit selected(wnd);
    return true;
}
