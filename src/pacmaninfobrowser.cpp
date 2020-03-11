/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmaninfobrowser.h"
#include <QDesktopServices>
#include <QApplication>
#include "byteshumanizer.h"
#include "static.h"
#include "themeicons.h"
#include <QTextTable>
#include <QUrlQuery>
#include "widgettextobject.h"
#include "waitindicator.h"
#include <QDebug>

class PackageTextObject : public SimpleLabelTextObject {
public:
    PackageTextObject(QTextEdit * parent,const AlpmPackage::Dependence & dep) : SimpleLabelTextObject(parent,dep.toString(),icon(dep.isInstalled()),dep.url(QString::fromLatin1("package")),dep.description()) {}
    PackageTextObject(QTextEdit * parent,const QString & name) : SimpleLabelTextObject(parent,name,icon(AlpmPackage::Dependence(name).isInstalled()),AlpmPackage::Dependence(name).url(QString::fromLatin1("package"))) {}
private:
    static QIcon icon(bool installed) {
        return ThemeIcons::get(installed?ThemeIcons::PKG_INSTALLED_MARK:ThemeIcons::PKG_NONINSTALLED_MARK);
    }
};

class GroupTextObject : public SimpleLabelTextObject {
public:
    GroupTextObject(QTextEdit * parent,const QString & name) : SimpleLabelTextObject(parent,name,ThemeIcons::get(ThemeIcons::PKG_GROUP),AlpmPackage::Dependence(name).url(QString::fromLatin1("group"))) {}
};

PacmanInfoBrowser::PacmanInfoBrowser(QWidget *parent) : CustomPopupTextBrowser(parent) {
    QDesktopServices::setUrlHandler("mailto",this,"openMailUrl");
    QDesktopServices::setUrlHandler("http",this,"openUrl");
    QDesktopServices::setUrlHandler("https",this,"openUrl");
}

void PacmanInfoBrowser::openMailUrl(const QUrl & url) {
    Static::runDetachedUnderOrigUser("xdg-email",QString::fromLatin1(url.toEncoded().constData()));
}

void PacmanInfoBrowser::openUrl(const QUrl & url) {
    Static::runDetachedUnderOrigUser("xdg-open",QString::fromLatin1(url.toEncoded().constData()));
}

void PacmanInfoBrowser::setSource(const QUrl & name) {
    if (name.scheme() != "qpc") QDesktopServices::openUrl(name);
    else {
        QString type = name.host();
        QString package = name.path().mid(1);
        QUrlQuery urlquery(name.query());
        if (type == "package") emit packageUrlSelected(package,urlquery.queryItemValue("ver"),urlquery.queryItemValue("oper").isEmpty()?(int)AlpmPackage::UNKNOWN:urlquery.queryItemValue("oper").toInt());
        if (type == "group") emit groupUrlSelected(package);
        if (type == "reason") emit reasonUrlSelected(package);
    }
}

void PacmanInfoBrowser::fillByInfo(AlpmPackage * pkg) {
    clear();
    repaint();
    if (pkg == NULL) return;

    int i;
    int width = calculateFirstFolumnFidth(20);
    QColor color = palette().button().color();
    QColor textColor = palette().buttonText().color();

    QTextTable * table = insertTable(textCursor(),pkg->isInstalled()?20:18,2,0.5);
    QTextTableFormat tbfmt(table->format());
    tbfmt.setColumnWidthConstraints(QVector<QTextLength>() << QTextLength(QTextLength::FixedLength,width) << QTextLength(QTextLength::PercentageLength,100));
    table->setFormat(tbfmt);
    setCellText(table,0,0,fieldNames[0],color,textColor,true);
    setCellText(table,1,0,fieldNames[1],color,textColor,true);
    setCellText(table,1,1,pkg->version());
    setCellText(table,2,0,fieldNames[19],color,textColor,true);
    setCellText(table,2,1,pkg->isInstalled()?QString::fromLatin1("local"):pkg->repo());
    setCellText(table,3,0,fieldNames[2],color,textColor,true);
    setCellText(table,3,1,pkg->description());
    setCellText(table,4,0,fieldNames[3],color,textColor,true);
    setCellText(table,4,1,pkg->arch());
    QString tmp = pkg->url().toString();
    setCellText(table,5,0,fieldNames[4],color,textColor,true);
    setCellLink(table,5,1,isMailAddress(tmp)?modifyMailAddress(tmp):tmp,tmp);
    setCellText(table,6,0,fieldNames[5],color,textColor,true);
    setCellText(table,6,1,pkg->licenses().join(" "));
    setCellText(table,7,0,fieldNames[6],color,textColor,true);
    setCellText(table,8,0,fieldNames[7],color,textColor,true);
    setCellText(table,9,0,fieldNames[8],color,textColor,true);
    setCellText(table,10,0,fieldNames[9],color,textColor,true);
    setCellText(table,11,0,fieldNames[10],color,textColor,true);
    setCellText(table,12,0,fieldNames[11],color,textColor,true);
    setCellText(table,13,0,fieldNames[12],color,textColor,true);
    setCellText(table,14,0,fieldNames[13],color,textColor,true);
    setCellText(table,15,0,fieldNames[14],color,textColor,true);
    setCellText(table,15,1,BytesHumanizer(pkg->installedSize()).toString());
    setCellText(table,16,0,fieldNames[15],color,textColor,true);
    setCellText(table,16,1,pkg->packager());
    setCellText(table,17,0,fieldNames[16],color,textColor,true);
    setCellText(table,17,1,pkg->buildDate().toString("ddd MMM dd yyyy hh:mm:ss"));
    QTextTableCell cell;
    if (pkg->isInstalled()) {
        setCellText(table,18,0,fieldNames[17],color,textColor,true);
        setCellText(table,18,1,pkg->installDate().toString("ddd MMM dd yyyy hh:mm:ss"));
        setCellText(table,19,0,fieldNames[18],color,textColor,true);
        setCellText(table,19,1,(pkg->reason() == AlpmPackage::Explicit)?tr("Explicitly installed"):tr("Installed as a dependency for another package"));
        cell = table->cellAt(19,1);
        insertText(cell.lastCursorPosition()," ");
        if (getuid() == 0) insertLink(cell.lastCursorPosition(),QString("qpc://reason/%1").arg(pkg->name()),(pkg->reason() == AlpmPackage::Explicit)?tr("Make it dependent"):tr("Make it explicit"));
    }
    setTextCursor(QTextCursor(document()));
    ensureCursorVisible();
    repaint();

    (new PackageTextObject(this,pkg->name()))->insert(table->cellAt(0,1).firstCursorPosition());
    cell = table->cellAt(7,1);
    for (i=0;i<pkg->groups().count();i++) {
        (new GroupTextObject(this,pkg->groups()[i]))->insert(cell.lastCursorPosition());
        insertText(cell.lastCursorPosition()," ");
    }
    setTextCursor(QTextCursor(document()));
    ensureCursorVisible();
    repaint();
    cell = table->cellAt(8,1);
    for (i=0;i<pkg->provides().count();i++) {
        (new PackageTextObject(this,pkg->provides()[i]))->insert(cell.lastCursorPosition());
        insertText(cell.lastCursorPosition()," ");
    }
    setTextCursor(QTextCursor(document()));
    ensureCursorVisible();
    repaint();
    cell = table->cellAt(9,1);
    for (i=0;i<pkg->depends().count();i++) {
        (new PackageTextObject(this,pkg->depends()[i]))->insert(cell.lastCursorPosition());
        insertText(cell.lastCursorPosition()," ");
    }
    setTextCursor(QTextCursor(document()));
    ensureCursorVisible();
    repaint();
    cell = table->cellAt(10,1);
    for (i=0;i<pkg->optdepends().count();i++) {
        (new PackageTextObject(this,pkg->optdepends()[i]))->insert(cell.lastCursorPosition());
        if ((i+1)<pkg->optdepends().count()) cell.lastCursorPosition().insertBlock();
    }
    setTextCursor(QTextCursor(document()));
    ensureCursorVisible();
    repaint();
    cell = table->cellAt(11,1);
    for (i=0;i<pkg->requiredby().count();i++) {
        (new PackageTextObject(this,pkg->requiredby()[i]))->insert(cell.lastCursorPosition());
        insertText(cell.lastCursorPosition()," ");
    }
    setTextCursor(QTextCursor(document()));
    ensureCursorVisible();
    repaint();
    cell = table->cellAt(12,1);
    for (i=0;i<pkg->optionalfor().count();i++) {
        (new PackageTextObject(this,pkg->optionalfor()[i]))->insert(cell.lastCursorPosition());
        insertText(cell.lastCursorPosition()," ");
    }
    setTextCursor(QTextCursor(document()));
    ensureCursorVisible();
    repaint();
    cell = table->cellAt(13,1);
    for (i=0;i<pkg->conflicts().count();i++) {
        (new PackageTextObject(this,pkg->conflicts()[i]))->insert(cell.lastCursorPosition());
        insertText(cell.lastCursorPosition()," ");
    }
    setTextCursor(QTextCursor(document()));
    ensureCursorVisible();
    repaint();
    cell = table->cellAt(14,1);
    for (i=0;i<pkg->replaces().count();i++) {
        (new PackageTextObject(this,pkg->replaces()[i]))->insert(cell.lastCursorPosition());
        insertText(cell.lastCursorPosition()," ");
    }

    setTextCursor(QTextCursor(document()));
    ensureCursorVisible();
}

const QString PacmanInfoBrowser::fieldNames[20] = {QObject::tr("Name"),QObject::tr("Version"),QObject::tr("Description"),
                                                   QObject::tr("Architecture"),QObject::tr("URL"),QObject::tr("Licenses"),
                                                   QObject::tr("Groups"),QObject::tr("Provides"),QObject::tr("Depends On"),
                                                   QObject::tr("Optional Deps"),QObject::tr("Required By"),QObject::tr("Optional For"),
                                                   QObject::tr("Conflicts With"),QObject::tr("Replaces"),QObject::tr("Installed Size"),
                                                   QObject::tr("Packager"),QObject::tr("Build Date"),QObject::tr("Install Date"),
                                                   QObject::tr("Install Reason"),QObject::tr("Repository")};

int PacmanInfoBrowser::calculateFirstFolumnFidth(int count) {
    QFontMetrics fm = QApplication::fontMetrics();
    int width = 0;
    for (int i=0;i<count;i++) {
        width = qMax(width,fm.horizontalAdvance(fieldNames[i]));
    }
    return (width + 10);
}

bool PacmanInfoBrowser::isMailAddress(const QString & str) {
    QUrl url("http://mailto:"+str);
    if (!url.isValid()) return false;

    int index = str.indexOf('@');
    if (index == -1) return false;

    return ((url.userName() == "mailto") && (url.password() == str.left(index)));
}

const QString PacmanInfoBrowser::modifyMailAddress(const QString & str) {
    return "mailto:"+str;
}
