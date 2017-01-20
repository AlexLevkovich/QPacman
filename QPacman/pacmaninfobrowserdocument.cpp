/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmaninfobrowserdocument.h"
#include "pacmanitemmodel.h"
#include <QFontMetrics>
#include <QTextEdit>
#include <QPainter>
#include <QProcess>

PacmanInfoBrowserDocument::PacmanInfoBrowserDocument(QObject *parent) : QTextDocument(parent) {
    m_model = NULL;
}

QVariant PacmanInfoBrowserDocument::loadResource(int type,const QUrl & name) {
    if ((type != QTextDocument::ImageResource) || (name.scheme() != "qpc")) return QTextDocument::loadResource(type,name);

    if (img_cache.contains(name)) return img_cache[name];

    QString img_name = name.path().mid(1);
    QPixmap pixmap;
    if (m_model != NULL) {
        QModelIndex index = m_model->installedIndexByPackageName(img_name);
        if (!index.isValid()) {
            index = m_model->installedProviderIndex(img_name);
            if (!index.isValid()) pixmap = QPixmap(":/pics/notinstalled.png");
            else pixmap = QPixmap(":/pics/installed.png");
        }
        else pixmap = QPixmap(":/pics/installed.png");
    }
    else {
        QProcess pacman;
        pacman.start(QString("%1 -Qi %2").arg(PACMAN_BIN).arg(img_name));
        pacman.waitForFinished(-1);
        if (pacman.exitCode() == 0) pixmap = QPixmap(":/pics/installed.png");
        else pixmap = QPixmap(":/pics/notinstalled.png");
    }

    QString text = PacmanEntry::urlParmsToPacmanDep(img_name + '?' + name.query());
    QTextEdit * textEdit = (QTextEdit *)parent();
    QFont font = textEdit->font();
    font.setUnderline(true);
    QFontMetrics metrix(font);
    int height = qMax(15,metrix.height());
    int img_height = qMin(height,pixmap.height());
    pixmap = pixmap.scaled((18*img_height)/15,img_height,Qt::KeepAspectRatio,Qt::SmoothTransformation);
    QPixmap ret(pixmap.width()+2+metrix.width(text),height);
    ret.fill(Qt::transparent);
    QPainter painter(&ret);
    painter.drawPixmap(0,ret.height()-pixmap.height(),pixmap);
    painter.setFont(font);
    painter.setPen(textEdit->palette().color(QPalette::Link));
    painter.drawText(QRect(pixmap.width()+2,ret.height()-metrix.height(),ret.width()-pixmap.width()-2,metrix.height()),Qt::AlignLeft|Qt::AlignBottom,text);

    img_cache[name] = ret;
    return ret;
}

void PacmanInfoBrowserDocument::clearImageCache() {
    img_cache.clear();
}

