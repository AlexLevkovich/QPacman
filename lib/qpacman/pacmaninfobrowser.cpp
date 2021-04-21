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
#include <QProcess>
#include <QDebug>

class PackageTextObject : public SimpleLabelTextObject {
public:
    PackageTextObject(QTextEdit * parent,const AlpmPackage::Dependence & dep) : SimpleLabelTextObject(parent,dep.toString(),icon(dep.isInstalled()),dep.url(QString::fromLatin1("package")),dep.description()) {}
    PackageTextObject(QTextEdit * parent,const AlpmPackage & pkg) : SimpleLabelTextObject(parent,pkg.name(),icon(pkg.isInstalled()),AlpmPackage::Dependence(pkg.name()).url(QString::fromLatin1("package"))) {}
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

    fieldNames = {QObject::tr("Name"),QObject::tr("Version"),QObject::tr("Description"),
                  QObject::tr("Architecture"),QObject::tr("URL"),QObject::tr("Licenses"),
                  QObject::tr("Groups"),QObject::tr("Provides"),QObject::tr("Depends On"),
                  QObject::tr("Optional Deps"),QObject::tr("Required By"),QObject::tr("Optional For"),
                  QObject::tr("Conflicts With"),QObject::tr("Replaces"),QObject::tr("Installed Size"),
                  QObject::tr("Packager"),QObject::tr("Build Date"),QObject::tr("Install Date"),
                  QObject::tr("Install Reason"),QObject::tr("Repository")};
}

void PacmanInfoBrowser::runProgram(const QString & name,const QStringList & args) {
    QProcess process;
    process.setProgram(name);
    process.setArguments(args);
    process.startDetached();
}

void PacmanInfoBrowser::openMailUrl(const QUrl & url) {
    runProgram("xdg-email",QStringList() << QString::fromLatin1(url.toEncoded().constData()));
}

void PacmanInfoBrowser::openUrl(const QUrl & url) {
    runProgram("xdg-open",QStringList() << QString::fromLatin1(url.toEncoded().constData()));
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

void PacmanInfoBrowser::refill() {
    fillByInfo(m_pkg);
}

void PacmanInfoBrowser::fillByInfo(const AlpmPackage & pkg,bool readonly) {
    clear();
    repaint();
    if (!pkg.isValid()) return;

    m_pkg = pkg;

    int width = calculateFirstFolumnFidth(20);
    QColor color = palette().button().color();
    QColor textColor = palette().buttonText().color();

    QTextTable * table = insertTable(textCursor(),pkg.isInstalled()?20:18,2,0.5);
    QTextTableFormat tbfmt(table->format());
    tbfmt.setColumnWidthConstraints(QVector<QTextLength>() << QTextLength(QTextLength::FixedLength,width) << QTextLength(QTextLength::PercentageLength,100));
    table->setFormat(tbfmt);
    QUrl icon_url = pkg.iconUrl();
    if (!icon_url.isEmpty()) insertImage(table->cellAt(0,0).lastCursorPosition(),icon_url.toString(),QSize(64,64),pkg.url());
    else setCellText(table,0,0,fieldNames[0],color,textColor,true);
    setCellText(table,1,0,fieldNames[1],color,textColor,true);
    setCellText(table,2,0,fieldNames[19],color,textColor,true);
    setCellText(table,3,0,fieldNames[2],color,textColor,true);
    setCellText(table,4,0,fieldNames[3],color,textColor,true);
    setCellText(table,5,0,fieldNames[4],color,textColor,true);
    setCellText(table,6,0,fieldNames[5],color,textColor,true);
    setCellText(table,7,0,fieldNames[6],color,textColor,true);
    setCellText(table,8,0,fieldNames[7],color,textColor,true);
    setCellText(table,9,0,fieldNames[8],color,textColor,true);
    setCellText(table,10,0,fieldNames[9],color,textColor,true);
    setCellText(table,11,0,fieldNames[10],color,textColor,true);
    setCellText(table,12,0,fieldNames[11],color,textColor,true);
    setCellText(table,13,0,fieldNames[12],color,textColor,true);
    setCellText(table,14,0,fieldNames[13],color,textColor,true);
    setCellText(table,15,0,fieldNames[14],color,textColor,true);
    setCellText(table,16,0,fieldNames[15],color,textColor,true);
    setCellText(table,17,0,fieldNames[16],color,textColor,true);
    if (pkg.isInstalled()) {
        setCellText(table,18,0,fieldNames[17],color,textColor,true);
        setCellText(table,19,0,fieldNames[18],color,textColor,true);
    }

    setCellText(table,1,1,pkg.version());
    setCellText(table,2,1,pkg.isInstalled()?QString::fromLatin1("local"):pkg.repo());
    setCellText(table,3,1,pkg.description());
    setCellText(table,4,1,pkg.arch());
    setCellText(table,6,1,pkg.licenses().join(" "));
    QString tmp = pkg.url().toString();
    setCellLink(table,5,1,isMailAddress(tmp)?modifyMailAddress(tmp):tmp,tmp);
    setCellText(table,15,1,BytesHumanizer(pkg.installedSize()).toString());
    setCellText(table,16,1,pkg.packager());
    setCellText(table,17,1,pkg.buildDate().toString("ddd MMM dd yyyy hh:mm:ss"));
    QTextTableCell cell;
    if (pkg.isInstalled()) {
        setCellText(table,18,1,pkg.installDate().toString("ddd MMM dd yyyy hh:mm:ss"));
        setCellText(table,19,1,(pkg.reason() == AlpmPackage::Explicit)?tr("Explicitly installed"):tr("Installed as a dependency for another package"));
        cell = table->cellAt(19,1);
        insertText(cell.lastCursorPosition()," ");
        if (!readonly) insertLink(cell.lastCursorPosition(),QString("qpc://reason/%1").arg(pkg.name()),(pkg.reason() == AlpmPackage::Explicit)?tr("Make it dependent"):tr("Make it explicit"));
    }

    (new PackageTextObject(this,pkg))->insert(table->cellAt(0,1).firstCursorPosition());
    cell = table->cellAt(7,1);
    for (QString & group: pkg.groups()) {
        (new GroupTextObject(this,group))->insert(cell.lastCursorPosition());
        insertText(cell.lastCursorPosition()," ");
    }
    cell = table->cellAt(8,1);
    for (AlpmPackage::Dependence & dep: pkg.provides()) {
        (new PackageTextObject(this,dep))->insert(cell.lastCursorPosition());
        insertText(cell.lastCursorPosition()," ");
    }
    cell = table->cellAt(9,1);
    for (AlpmPackage::Dependence & dep: pkg.depends()) {
        (new PackageTextObject(this,dep))->insert(cell.lastCursorPosition());
        insertText(cell.lastCursorPosition()," ");
    }
    cell = table->cellAt(10,1);
    QList<AlpmPackage::Dependence> optdeps = pkg.optdepends();
    int i = 0;
    for (AlpmPackage::Dependence & dep: optdeps) {
        (new PackageTextObject(this,dep))->insert(cell.lastCursorPosition());
        if ((i+1)<optdeps.count()) cell.lastCursorPosition().insertBlock();
        i++;
    }
    if (pkg.isInstalled()) {
        cell = table->cellAt(11,1);
        for (QString & name: pkg.requiredby()) {
            (new PackageTextObject(this,name))->insert(cell.lastCursorPosition());
            insertText(cell.lastCursorPosition()," ");
        }
        cell = table->cellAt(12,1);
        for (QString & name: pkg.optionalfor()) {
            (new PackageTextObject(this,name))->insert(cell.lastCursorPosition());
            insertText(cell.lastCursorPosition()," ");
        }
    }
    cell = table->cellAt(13,1);
    for (AlpmPackage::Dependence & dep: pkg.conflicts()) {
        (new PackageTextObject(this,dep))->insert(cell.lastCursorPosition());
        insertText(cell.lastCursorPosition()," ");
    }
    cell = table->cellAt(14,1);
    for (AlpmPackage::Dependence & dep: pkg.replaces()) {
        (new PackageTextObject(this,dep))->insert(cell.lastCursorPosition());
        insertText(cell.lastCursorPosition()," ");
    }

    if (!pkg.isInstalled()) table->removeRows(11,2);

    setTextCursor(QTextCursor(document()));
    ensureCursorVisible();
}

int PacmanInfoBrowser::calculateFirstFolumnFidth(int count) const {
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
