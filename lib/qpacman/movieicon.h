/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef MOVIEICON_H
#define MOVIEICON_H

#include <QObject>
#include <QList>
#include <QIcon>
#include <QPixmap>
#include <QTimer>
#include <QVariant>

class QMovie;

class MovieIcon : public QObject {
    Q_OBJECT
public:
    MovieIcon(const QList<QIcon> & icons = QList<QIcon>(),const QSize & output_frame_size = QSize());
    MovieIcon(const QString & iconpath,int input_frame_height,const QSize & output_frame_size = QSize());
    MovieIcon(const QString & iconpath,const QList<QPoint> & start_poses,const QSize & size,const QSize & output_frame_size = QSize());
    MovieIcon(const QIcon & icon,const QSize & output_frame_size = QSize());
    MovieIcon(QMovie * movie,const QSize & output_frame_size = QSize());

    bool add(const QList<QIcon> & icons);
    bool add(const QString & iconpath,int input_frame_height);
    bool add(const QString & iconpath,const QList<QPoint> & start_poses,const QSize & size);
    bool add(const QIcon & icon);
    bool add(QMovie * movie);
    bool putStatic(const QIcon & icon,const QRect & output_place);
    bool putStaticToCenter(const QIcon & icon,const QSize & output_size);
    void removeStatic();
    void setSize(const QSize & output_frame_size);
    QSize size() const;

    void clear();
    int frameCount() const;

    void setDelay(int value);
    int delay() const;

    QPixmap frame(int frame_number) const;

    bool start();
    void stop();
    int currentFrameNumber() const;

    static int defaultSpeed;

signals:
    void finished();
    void frameChanged(int frame_number);
    void started();

private slots:
    void timeout();

private:
    bool add(const QString & iconpath,const QList<QRect> & rects);
    QSize iconSize(const QString & iconpath);
    void putStaticImage(QPixmap & frame);
    void updateArrays();
    bool update(int index);

    QList<QPixmap> m_pixmaps;
    QList<QVariant> m_icons;
    QList<QList<QRect> > m_is_simple;
    QSize m_size;
    int m_speed;
    QTimer m_timer;
    int frame_id;
    QIcon m_static_icon;
    QRect m_static_rect;
};

#endif // MOVIEICON_H
