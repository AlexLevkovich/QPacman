/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANWAITVIEW_H
#define PACMANWAITVIEW_H

#include <QTimer>
#include <QWidget>

class QPaintEvent;

class PacmanWaitView :public QWidget {
    Q_OBJECT
public:
    explicit PacmanWaitView(QWidget *parent = 0);

protected:
    void showEvent(QShowEvent * event);
    void hideEvent(QHideEvent * event);
    void paintEvent(QPaintEvent *event);

private slots:
    void changeAngle();

private:
    QPixmap generatePixmap() const;

    QTimer m_timer;
    int angle;
    double m_actualInnerRadius;
    double m_actualOuterRadius;
    QColor m_backgroundColor;
    QColor m_foregroundColor;

    static const double m_innerRadius;
    static const double m_outerRadius;
    static const double m_size;
};

#endif // PACMANWAITVIEW_H
