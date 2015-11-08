/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmaninfobrowserdocument.h"
#include "pacmanitemmodel.h"
#include <QPixmap>

PacmanInfoBrowserDocument::PacmanInfoBrowserDocument(QObject *parent) : QTextDocument(parent) {
    m_model = NULL;
}

QVariant PacmanInfoBrowserDocument::loadResource(int type,const QUrl & name) {
    if ((type != QTextDocument::ImageResource) || (name.scheme() != "qpc")) return QTextDocument::loadResource(type,name);
    if (m_model == NULL) return QPixmap(":/pics/notinstalled.png").scaled(1,1);

    QString img_name = name.host();
    bool is_optional = false;
    QStringList parts = img_name.split(".");
    if ((parts.count() >= 2) && (parts[0] == "optional")) {
        is_optional = true;
        img_name.clear();
        for (int i=1;i<parts.count();i++) img_name+=parts[i]+".";
        img_name = img_name.left(img_name.length()-1);
    }

    QModelIndex index = m_model->installedIndexByPackageName(img_name);
    if (!index.isValid()) {
        index = m_model->installedProviderIndex(img_name);
        if (!index.isValid()) return QPixmap(":/pics/notinstalled.png").scaled(is_optional?QSize(18,15):QSize(1,1));
    }

    return QPixmap(":/pics/installed.png").scaled(18,15,Qt::KeepAspectRatio,Qt::SmoothTransformation);
}
