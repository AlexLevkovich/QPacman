#ifndef WAITINDICATOR_H
#define WAITINDICATOR_H

#include <QThread>
#include <QColor>
#include <QPixmap>

class QEvent;

class WaitIndicator : public QThread {
    Q_OBJECT
public:
    WaitIndicator(QWidget *parent);
    ~WaitIndicator();

protected:
    void run();
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void changeAngle();

signals:
#ifndef Q_MOC_RUN
    private:
#endif
   void doStop();
   void doStart();

private:
    double m_size;
    int angle;
    double m_actualInnerRadius;
    double m_actualOuterRadius;
    QColor m_backgroundColor;
    QColor m_foregroundColor;
    QPixmap m_pixmap;

    static const double m_innerRadius;
    static const double m_outerRadius;
    static const double m_max_size;
};

#endif // WAITINDICATOR_H
