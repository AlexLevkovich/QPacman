/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "balloontip.h"
#include "wraplabel.h"

#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QApplication>
#include <QDesktopWidget>
#include <QGridLayout>
#include <QPainter>
#include <QBitmap>
#include <QMouseEvent>

static BalloonTip *theSolitaryBalloonTip = 0;

BalloonTip * BalloonTip::showBalloon(SystemTrayIcon::MessageIcon icon, const QString& title,
                              const QString& message, SystemTrayIcon *trayIcon,
                              const QPoint& pos, int timeout, bool showArrow) {
    hideBalloon();
    if (message.isEmpty() && title.isEmpty()) return NULL;

    theSolitaryBalloonTip = new BalloonTip(icon, title, message, trayIcon);
    if (timeout < 0) timeout = 10000; //10 s default
    theSolitaryBalloonTip->balloon(pos, timeout, showArrow);
    return theSolitaryBalloonTip;
}

void BalloonTip::hideBalloon() {
    if (!theSolitaryBalloonTip) return;
    theSolitaryBalloonTip->hide();
    delete theSolitaryBalloonTip;
    theSolitaryBalloonTip = 0;
}

bool BalloonTip::isBalloonVisible() {
    return theSolitaryBalloonTip;
}

BalloonTip::BalloonTip(SystemTrayIcon::MessageIcon icon, const QString& title,
                       const QString& message, SystemTrayIcon *ti) :
                       QWidget(0, Qt::ToolTip), trayIcon(ti), timerId(-1) {
    setAttribute(Qt::WA_DeleteOnClose);
    QObject::connect(ti, SIGNAL(destroyed()), this, SLOT(close()));
    QObject::connect(this, SIGNAL(destroyed()), ti, SIGNAL(messageWindowDestroyed()));

    QLabel *titleLabel = new QLabel(this);
    titleLabel->setText(title);
    QFont f = titleLabel->font();
    f.setBold(true);
    titleLabel->setFont(f);
    titleLabel->setTextFormat(Qt::PlainText); // to maintain compat with windows

    const int iconSize = 18;
    const int closeButtonSize = 15;

    QPushButton *closeButton = new QPushButton(this);
    closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    closeButton->setIconSize(QSize(closeButtonSize, closeButtonSize));
    closeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    closeButton->setFixedSize(closeButtonSize, closeButtonSize);
    QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    WrapLabel *msgLabel = new WrapLabel(this);
    msgLabel->setText(message);
    msgLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    connect(msgLabel,SIGNAL(anchorClicked(const QUrl &)),this,SIGNAL(anchorClicked(const QUrl &)));

    // smart size for the message label
    int limit = QApplication::desktop()->availableGeometry(msgLabel).size().width() / 3;
    if (msgLabel->sizeHint().width() > limit) {
        if (msgLabel->sizeHint().width() > limit) {
            msgLabel->setWordWrapMode(QTextOption::WrapAnywhere);
        }
        msgLabel->setFixedSize(limit, msgLabel->heightForWidth(limit));
    }

    QIcon si;
    switch (icon) {
    case SystemTrayIcon::Warning:
        si = style()->standardIcon(QStyle::SP_MessageBoxWarning);
        break;
    case SystemTrayIcon::Critical:
    si = style()->standardIcon(QStyle::SP_MessageBoxCritical);
        break;
    case SystemTrayIcon::Information:
    si = style()->standardIcon(QStyle::SP_MessageBoxInformation);
        break;
    case SystemTrayIcon::NoIcon:
    default:
        break;
    }

    QGridLayout *layout = new QGridLayout(this);
    if (!si.isNull()) {
        QLabel *iconLabel = new QLabel;
        iconLabel->setPixmap(si.pixmap(iconSize, iconSize));
        iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        iconLabel->setMargin(2);
        layout->addWidget(iconLabel, 0, 0);
        layout->addWidget(titleLabel, 0, 1);
    } else {
        layout->addWidget(titleLabel, 0, 0, 1, 2);
    }

    layout->addWidget(closeButton, 0, 2);
    layout->addWidget(msgLabel, 1, 0, 1, 3);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setMargin(3);
    setLayout(layout);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0xff, 0xff, 0xe1));
    pal.setColor(QPalette::WindowText, Qt::black);
    setPalette(pal);
}

BalloonTip::~BalloonTip() {
    theSolitaryBalloonTip = NULL;
}

void BalloonTip::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.drawPixmap(rect(), pixmap);
}

void BalloonTip::resizeEvent(QResizeEvent *ev) {
    QWidget::resizeEvent(ev);
}

void BalloonTip::showEvent(QShowEvent *e) {
    QWidget::showEvent(e);

    QPoint pos = property("pos").toPoint();
    int msecs = property("msecs").toInt();
    bool showArrow = property("showArrow").toBool();

    QRect scr = QApplication::desktop()->screenGeometry(pos);
    QSize sh = sizeHint();
    const int border = 1;
    const int ah = 18, ao = 18, aw = 18, rc = 7;
    bool arrowAtTop = (pos.y() + sh.height() + ah < scr.height());
    bool arrowAtLeft = (pos.x() + sh.width() - ao < scr.width());

    int ml, mr, mt, mb;
    QSize sz = sizeHint();
    if (!arrowAtTop) {
        ml = mt = 0;
        mr = sz.width() - 1;
        mb = sz.height() - ah - 1;
    } else {
        ml = 0;
        mt = ah;
        mr = sz.width() - 1;
        mb = sz.height() - 1;
    }

    QPainterPath path;
#if defined(QT_NO_XSHAPE) && defined(Q_WS_X11)
    // XShape is required for setting the mask, so we just
    // draw an ugly square when its not available
    path.moveTo(0, 0);
    path.lineTo(sz.width() - 1, 0);
    path.lineTo(sz.width() - 1, sz.height() - 1);
    path.lineTo(0, sz.height() - 1);
    path.lineTo(0, 0);
    move(qMax(pos.x() - sz.width(), scr.left()), pos.y());
#else
    path.moveTo(ml + rc, mt);
    if (arrowAtTop && arrowAtLeft) {
        if (showArrow) {
            path.lineTo(ml + ao, mt);
            path.lineTo(ml + ao, mt - ah);
            path.lineTo(ml + ao + aw, mt);
        }
        move(qMax(pos.x() - ao, scr.left() + 2), pos.y());
    } else if (arrowAtTop && !arrowAtLeft) {
        if (showArrow) {
            path.lineTo(mr - ao - aw, mt);
            path.lineTo(mr - ao, mt - ah);
            path.lineTo(mr - ao, mt);
        }
        move(qMin(pos.x() - sh.width() + ao, scr.right() - sh.width() - 2), pos.y());
    }
    path.lineTo(mr - rc, mt);
    path.arcTo(QRect(mr - rc*2, mt, rc*2, rc*2), 90, -90);
    path.lineTo(mr, mb - rc);
    path.arcTo(QRect(mr - rc*2, mb - rc*2, rc*2, rc*2), 0, -90);
    if (!arrowAtTop && !arrowAtLeft) {
        if (showArrow) {
            path.lineTo(mr - ao, mb);
            path.lineTo(mr - ao, mb + ah);
            path.lineTo(mr - ao - aw, mb);
        }
        move(qMin(pos.x() - sh.width() + ao, scr.right() - sh.width() - 2),
             pos.y() - sh.height());
    } else if (!arrowAtTop && arrowAtLeft) {
        if (showArrow) {
            path.lineTo(ao + aw, mb);
            path.lineTo(ao, mb + ah);
            path.lineTo(ao, mb);
        }
        move(qMax(pos.x() - ao, scr.x() + 2), pos.y() - sh.height());
    }
    path.lineTo(ml + rc, mb);
    path.arcTo(QRect(ml, mb - rc*2, rc*2, rc*2), -90, -90);
    path.lineTo(ml, mt + rc);
    path.arcTo(QRect(ml, mt, rc*2, rc*2), 180, -90);

    // Set the mask
    QBitmap bitmap = QBitmap(sizeHint());
    bitmap.fill(Qt::color0);
    QPainter painter1(&bitmap);
    painter1.setPen(QPen(Qt::color1, border));
    painter1.setBrush(QBrush(Qt::color1));
    painter1.drawPath(path);
    setMask(bitmap);
#endif

    // Draw the border
    pixmap = QPixmap(sz);
    QPainter painter2(&pixmap);
    painter2.setPen(QPen(palette().color(QPalette::Window).darker(160), border));
    painter2.setBrush(palette().color(QPalette::Window));
    painter2.drawPath(path);

    if (msecs > 0) timerId = startTimer(msecs);
}

void BalloonTip::balloon(const QPoint& pos, int msecs, bool showArrow) {
    setProperty("pos",pos);
    setProperty("msecs",msecs);
    setProperty("showArrow",showArrow);
    QRect scr = QApplication::desktop()->screenGeometry(pos);
    QSize sh = sizeHint();
    const int border = 1;
    const int ah = 18;
    bool arrowAtTop = (pos.y() + sh.height() + ah < scr.height());
    setContentsMargins(border + 3,  border + (arrowAtTop ? ah : 0) + 2, border + 3, border + (arrowAtTop ? 0 : ah) + 2);
    updateGeometry();
    show();
    emit trayIcon->messageWindowShowUp();
}

void BalloonTip::mousePressEvent(QMouseEvent *e) {
    close();
    if(e->button() == Qt::LeftButton) emit trayIcon->messageClicked();
}

void BalloonTip::timerEvent(QTimerEvent *e) {
    if (e->timerId() == timerId) {
        killTimer(timerId);
        if (!underMouse()) close();
        return;
    }
    QWidget::timerEvent(e);
}
