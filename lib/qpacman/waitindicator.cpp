#include "waitindicator.h"
#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QApplication>
#include <QPixmapCache>
#include <QPainterPath>
#include <QDebug>

const double WaitIndicator::m_innerRadius = 0.8;
const double WaitIndicator::m_outerRadius = 1.0;
const double WaitIndicator::m_max_size = 150.0;

WaitIndicator::WaitIndicator(QWidget *parent) : QThread(parent) {
    angle = 360;
    m_size = 0;

    m_backgroundColor = QApplication::palette().color(QPalette::Highlight);
    m_foregroundColor = QApplication::palette().color(QPalette::Base);

    parent->setMinimumSize(m_size,m_size);
    parent->setAutoFillBackground(true);
}

WaitIndicator::~WaitIndicator() {
    quit();
    while(isRunning()) QApplication::processEvents();
}

void WaitIndicator::run() {
    QWidget * parent = (QWidget *)this->parent();
    parent->installEventFilter(this);

    QTimer timer;
    timer.setInterval(50);
    connect(&timer,&QTimer::timeout,this,&WaitIndicator::changeAngle,Qt::QueuedConnection);
    connect(this,&WaitIndicator::doStart,&timer,[&]() { timer.start(); },Qt::QueuedConnection);
    connect(this,&WaitIndicator::doStop,&timer,&QTimer::stop,Qt::QueuedConnection);

    if (parent->isVisible()) QMetaObject::invokeMethod(this,"doStart",Qt::QueuedConnection);

    exec();
}

bool WaitIndicator::eventFilter(QObject *obj, QEvent *event) {
    QWidget * parent = (QWidget *)this->parent();

    if (obj == parent) {
        if (event->type() == QEvent::Show) emit doStart();
        else if (event->type() == QEvent::Hide) emit doStop();
        else if (event->type() == QEvent::Paint) {
            QPaintEvent * e = (QPaintEvent *)event;
            QRect pixmapRect((parent->width()-m_size)*0.5,(parent->height()-m_size)*0.5,m_size,m_size);
            if (!e->rect().isNull() && e->rect().isValid() && e->rect().contains(pixmapRect)) {
               QPainter painter(parent);
               painter.drawPixmap(pixmapRect.x(),pixmapRect.y(),m_pixmap);
               painter.end();
            }
        }
    }

    return QThread::eventFilter(obj,event);
}

void WaitIndicator::changeAngle() {
    angle -= 10;
    if (angle == 0) angle = 360;

    QWidget * parent = (QWidget *)this->parent();
    m_size = qMin(m_max_size,(double)(qMin(parent->width(),parent->height()) - 10));
    m_actualInnerRadius = m_size*0.5*m_innerRadius;
    m_actualOuterRadius = m_size*0.5*m_outerRadius;

    QPainterPath path;
    path.setFillRule(Qt::OddEvenFill);
    path.addEllipse(QPointF(m_actualOuterRadius,m_actualOuterRadius),m_actualOuterRadius,m_actualOuterRadius);
    path.addEllipse(QPointF(m_actualOuterRadius,m_actualOuterRadius),m_actualInnerRadius,m_actualInnerRadius);

    double nActualDiameter = 2 * m_actualOuterRadius;
    if (nActualDiameter <= 0) return;

    QPixmap pixmap(nActualDiameter,nActualDiameter);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);

    // Draw the ring background
    p.setPen(Qt::NoPen);
    p.setBrush(m_backgroundColor);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawPath(path);

    // Draw the ring foreground
    QConicalGradient gradient(QPointF(m_actualOuterRadius,m_actualOuterRadius),angle);
    gradient.setColorAt(0.0,Qt::transparent);
    gradient.setColorAt(0.05,m_foregroundColor);
    gradient.setColorAt(0.8,Qt::transparent);
    p.setBrush(gradient);
    p.drawPath(path);
    p.end();

    m_pixmap = pixmap;
    parent->update();
}
