/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "infoview.h"
#include <QAbstractTextDocumentLayout>
#include <QTextCursor>
#include <QTextTable>
#include <QSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QComboBox>
#include <QHideEvent>
#include "libalpm.h"
#include <QDebug>

LabelLineEditTextObject::LabelLineEditTextObject(QTextEdit * edit,const QString & labelText,const QString & text) : WidgetTextObject(edit) {
    label = new QLabel(labelText);
    lineEdit = new QLineEdit(text);

    QLayout * layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(label);
    layout->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum));
    layout->addWidget(lineEdit);
    setLayout(layout);

    expandToBlockWidth(true);
}

void LabelLineEditTextObject::setEchoMode(QLineEdit::EchoMode mode) {
    lineEdit->setEchoMode(mode);
}

QString LabelLineEditTextObject::lineText() const {
    return lineEdit->text();
}

QString LabelLineEditTextObject::text() const {
    return label->text()+ " " + lineEdit->text();
}

bool LabelLineEditTextObject::isEnabled() const {
    return lineEdit->isEnabled();
}

void LabelLineEditTextObject::setEnabled(bool flag) {
    lineEdit->setEnabled(flag);
}

LabelSpinTextObject::LabelSpinTextObject(QTextEdit * edit,const QString & text,int min,int max,int value,const QString & suffix) : WidgetTextObject(edit) {
    label = new QLabel(text);
    spin = new QSpinBox();
    spin->setMinimum(min);
    spin->setMaximum(max);
    spin->setValue(value);
    spin->setSuffix(suffix);

    QLayout * layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(label);
    layout->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum));
    layout->addWidget(spin);
    setLayout(layout);

    expandToBlockWidth(true);
}

QString LabelSpinTextObject::text() const {
    return label->text() + QString(": %1").arg(spin->value());
}

int LabelSpinTextObject::value() const {
    return spin->value();
}

bool LabelSpinTextObject::isEnabled() const {
    return spin->isEnabled();
}

void LabelSpinTextObject::setEnabled(bool flag) {
    spin->setEnabled(flag);
}

ProxyTypeObject::ProxyTypeObject(QTextEdit * edit,const QString & text,const QNetworkProxy & proxy) : WidgetTextObject(edit) {
    label = new QLabel(text);
    combo = new QComboBox();
    combo->addItem(tr("No proxy"),QNetworkProxy::NoProxy);
    combo->addItem("HTTP",QNetworkProxy::HttpCachingProxy);
    combo->addItem("CONNECT",QNetworkProxy::HttpProxy);
    combo->addItem("FTP",QNetworkProxy::FtpCachingProxy);
    for (int i=0;i<combo->count();i++) {
        if (combo->itemData(i) == proxy.type()) {
            combo->setCurrentIndex(i);
            break;
        }
    }

    QLayout * layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(label);
    layout->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum));
    layout->addWidget(combo);
    setLayout(layout);

    expandToBlockWidth(true);
    setLayout(layout);

    connect(combo,SIGNAL(activated(int)),SLOT(proxyActivated(int)));
}

QNetworkProxy::ProxyType ProxyTypeObject::value() const {
    return (QNetworkProxy::ProxyType)combo->currentData().toInt();
}

QString ProxyTypeObject::text() const {
    return label->text() + " " + combo->currentText();
}

void ProxyTypeObject::proxyActivated(int index) {
    emit activated(combo->itemData(index).toInt());
}

DBExtObject::DBExtObject(QTextEdit * edit,const QString & text) : WidgetTextObject(edit) {
    label = new QLabel(text);
    combo = new QComboBox();

    for (QString & ext: Alpm::instance()->dbExtensions()) {
        combo->addItem(ext);
    }

    for (int i=0;i<combo->count();i++) {
        if (combo->itemText(i) == Alpm::instance()->dbExtension()) {
            combo->setCurrentIndex(i);
            break;
        }
    }

    QLayout * layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(label);
    layout->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum));
    layout->addWidget(combo);
    setLayout(layout);

    expandToBlockWidth(true);
    setLayout(layout);

    connect(combo,SIGNAL(activated(int)),SLOT(extActivated(int)));
}

QString DBExtObject::value() const {
    return combo->currentText();
}

QString DBExtObject::text() const {
    return label->text() + " " + combo->currentText();
}

void DBExtObject::extActivated(int index) {
    emit activated(combo->itemText(index));
}


InfoView::InfoView(QWidget *parent) : CustomPopupTextBrowser(parent) {
    document()->setDocumentMargin(20);
    QTextTable * table = insertTable(textCursor(),1,3,0);
    insertImage(table->cellAt(0,0).firstCursorPosition(),"://pics/qpacman.svg",QSize(100,117));
    insertText(table->cellAt(0,1).firstCursorPosition(),QString::fromLatin1("   "));
    QTextTableCell cell = table->cellAt(0,2);
    insertText(cell.firstCursorPosition(),"QPacman",QFont::Bold);
    insertText(cell.lastCursorPosition()," "+tr("is GUI frontend to")+" ");
    insertLink(cell.lastCursorPosition(),"https://www.archlinux.org/","Pacman");
    insertText(cell.lastCursorPosition()," ("+tr("Archlinux's package manager")+").");
    cell.lastCursorPosition().insertBlock();
    insertText(cell.lastCursorPosition(),(tr("QPacman's version is")+" " QPACMAN_VERSION)+".");
    cell.lastCursorPosition().insertBlock();
    insertText(cell.lastCursorPosition(),tr("Developer: Alex Levkovich")+" (");
    insertLink(cell.lastCursorPosition(),"mailto:alevkovich@tut.by","alevkovich@tut.by");
    insertText(cell.lastCursorPosition(),")");
    cell.lastCursorPosition().insertBlock();
    insertText(cell.lastCursorPosition(),tr("License")+": GPL-3");
    textCursor().insertBlock();
    textCursor().insertBlock();
    insertText(textCursor(),"qpacman ",QFont::Bold);
    insertText(textCursor(),tr("is GUI frontend for ArchLinux's pacman command. Really there is one more application:"));
    textCursor().insertBlock();
    insertText(textCursor(),"qpacmantray ",QFont::Bold);
    insertText(textCursor(),tr("is update checker, checks them automatically and shows all information in tray icon. You can setup the checking interval."));
    textCursor().insertBlock();
    textCursor().insertBlock();
    insertText(textCursor(),tr("You can use the following keyboad shortcuts")+":");
    textCursor().insertBlock();
    table = insertTable(textCursor(),11,2,1,Qt::AlignLeft,true);
    insertText(table->cellAt(0,0).firstCursorPosition(),tr("Shortcut"),QFont::Bold);
    insertText(table->cellAt(0,1).firstCursorPosition(),tr("Description"),QFont::Bold);
    insertText(table->cellAt(1,0).firstCursorPosition(),"Alt+A");
    insertText(table->cellAt(1,1).firstCursorPosition(),tr("Applies the changes"));
    insertText(table->cellAt(2,0).firstCursorPosition(),"Alt+M");
    insertText(table->cellAt(2,1).firstCursorPosition(),tr("Marks all visible packages"));
    insertText(table->cellAt(3,0).firstCursorPosition(),"Alt+R");
    insertText(table->cellAt(3,1).firstCursorPosition(),tr("Resets the visible changes"));
    insertText(table->cellAt(4,0).firstCursorPosition(),"F5");
    insertText(table->cellAt(4,1).firstCursorPosition(),tr("Rereads/Refreshes the package list"));
    insertText(table->cellAt(5,0).firstCursorPosition(),"F6");
    insertText(table->cellAt(5,1).firstCursorPosition(),tr("Downloads the package db and refresh the package list"));
    insertText(table->cellAt(6,0).firstCursorPosition(),"Alt+L");
    insertText(table->cellAt(6,1).firstCursorPosition(),tr("Shows the window with log messages"));
    insertText(table->cellAt(7,0).firstCursorPosition(),"Alt+C");
    insertText(table->cellAt(7,1).firstCursorPosition(),tr("Clears the cache of the packages"));
    insertText(table->cellAt(8,0).firstCursorPosition(),"Shift+Enter");
    insertText(table->cellAt(8,1).firstCursorPosition(),tr("Marks/Unmarks the selected package"));
    insertText(table->cellAt(9,0).firstCursorPosition(),"Shift+Right");
    insertText(table->cellAt(9,1).firstCursorPosition(),tr("Shows tool button's menu"));
    insertText(table->cellAt(10,0).firstCursorPosition(),"F1");
    insertText(table->cellAt(10,1).firstCursorPosition(),tr("This info page"));
    setupTableHeader(table,palette().color(QPalette::Active,QPalette::Mid),palette().buttonText().color());
    setTableRowColors(table,palette().button().color(),viewport()->palette().color(QPalette::Active,QPalette::Base));
    textCursor().insertBlock();
    textCursor().insertBlock();

    insertText(textCursor(),tr("The following system variables are used")+":");
    table = insertTable(textCursor(),18,2,1,Qt::AlignLeft,true);
    insertText(table->cellAt(0,0).firstCursorPosition(),tr("Variable"),QFont::Bold);
    insertText(table->cellAt(0,1).firstCursorPosition(),tr("Value"),QFont::Bold);
    insertText(table->cellAt(1,0).firstCursorPosition(),"RootDir");
    insertText(table->cellAt(1,1).firstCursorPosition(),Alpm::instance()->rootDir());
    insertText(table->cellAt(2,0).firstCursorPosition(),"DBPath");
    insertText(table->cellAt(2,1).firstCursorPosition(),Alpm::instance()->dbPath());
    insertText(table->cellAt(3,0).firstCursorPosition(),"GPGDir");
    insertText(table->cellAt(3,1).firstCursorPosition(),Alpm::instance()->gpgDir());
    insertText(table->cellAt(4,0).firstCursorPosition(),"LogFile");
    insertText(table->cellAt(4,1).firstCursorPosition(),Alpm::instance()->logFileName());
    insertText(table->cellAt(5,0).firstCursorPosition(),"Architecture");
    insertText(table->cellAt(5,1).firstCursorPosition(),Alpm::instance()->arch());
    insertText(table->cellAt(6,0).firstCursorPosition(),"UseSyslog");
    insertText(table->cellAt(6,1).firstCursorPosition(),Alpm::instance()->doUseSysLog()?"1":"0");
    insertText(table->cellAt(7,0).firstCursorPosition(),"DisableDownloadTimeout");
    insertText(table->cellAt(7,1).firstCursorPosition(),Alpm::instance()->doDisableDownloadTimeout()?"1":"0");
    insertText(table->cellAt(8,0).firstCursorPosition(),"SigLevel");
    insertText(table->cellAt(8,1).firstCursorPosition(),Alpm::instance()->sigLevel().join(" "));
    insertText(table->cellAt(9,0).firstCursorPosition(),"LocalFileSigLevel");
    insertText(table->cellAt(9,1).firstCursorPosition(),Alpm::instance()->localFileSigLevel().join(" "));
    insertText(table->cellAt(10,0).firstCursorPosition(),"RemoteFileSigLevel");
    insertText(table->cellAt(10,1).firstCursorPosition(),Alpm::instance()->remoteFileSigLevel().join(" "));
    insertText(table->cellAt(11,0).firstCursorPosition(),"HoldPkg");
    insertText(table->cellAt(11,1).firstCursorPosition(),Alpm::instance()->holdPkgs().join(" "));
    insertText(table->cellAt(12,0).firstCursorPosition(),"CacheDir");
    insertText(table->cellAt(12,1).firstCursorPosition(),Alpm::instance()->cacheDirs().join("\n"));
    insertText(table->cellAt(13,0).firstCursorPosition(),"HookDir");
    insertText(table->cellAt(13,1).firstCursorPosition(),Alpm::instance()->hookDirs().join("\n"));
    insertText(table->cellAt(14,0).firstCursorPosition(),"IgnoreGroup");
    insertText(table->cellAt(14,1).firstCursorPosition(),Alpm::instance()->ignoreGroups().join(" "));
    insertText(table->cellAt(15,0).firstCursorPosition(),"IgnorePkg");
    insertText(table->cellAt(15,1).firstCursorPosition(),Alpm::instance()->ignorePkgs().join(" "));
    insertText(table->cellAt(16,0).firstCursorPosition(),"NoExtract");
    insertText(table->cellAt(16,1).firstCursorPosition(),Alpm::instance()->noExtractPkgs().join(" "));
    insertText(table->cellAt(17,0).firstCursorPosition(),"NoUpgrade");
    insertText(table->cellAt(17,1).firstCursorPosition(),Alpm::instance()->noUpgradePkgs().join(" "));
    setupTableHeader(table,palette().color(QPalette::Active,QPalette::Mid),palette().buttonText().color());
    setTableRowColors(table,palette().button().color(),viewport()->palette().color(QPalette::Active,QPalette::Base));
    textCursor().insertBlock();
    textCursor().insertBlock();

    insertText(textCursor(),tr("The following user variables are used")+":");
    table = insertTable(textCursor(),10,1,1,Qt::AlignLeft,true);
    insertText(table->cellAt(0,0).firstCursorPosition(),tr("Variables"),QFont::Bold);
    useSysIconsBox = new CheckBoxTextObject(this,tr("Use the system icons"),Alpm::instance()->useSystemIcons());
    useSysIconsBox->insert(table->cellAt(1,0).firstCursorPosition());
    extObj = new DBExtObject(this,tr("Database files' suffix (restart of qpacman_service is needed)"));
    extObj->insert(table->cellAt(2,0).firstCursorPosition());
    threadsObj = new LabelSpinTextObject(this,tr("Count of the threads for each download"),1,20,Alpm::instance()->downloaderThreadCount());
    threadsObj->insert(table->cellAt(3,0).firstCursorPosition());
    timeoutObj = new LabelSpinTextObject(this,tr("Connection timeout"),0,300,Alpm::instance()->downloaderTimeout()/1000,tr(" secs"));
    timeoutObj->insert(table->cellAt(4,0).firstCursorPosition());
    QNetworkProxy proxy = Alpm::instance()->downloaderProxy();
    proxyType = new ProxyTypeObject(this,tr("Proxy type"),proxy);
    connect(proxyType,SIGNAL(activated(int)),SLOT(proxyActivated(int)));
    proxyType->insert(table->cellAt(5,0).firstCursorPosition());
    proxyAddress = new LabelLineEditTextObject(this,tr("Proxy address"),proxy.hostName());
    proxyAddress->insert(table->cellAt(6,0).firstCursorPosition());
    proxyPort = new LabelSpinTextObject(this,tr("Proxy port"),1,65535,proxy.port());
    proxyPort->insert(table->cellAt(7,0).firstCursorPosition());
    proxyUser = new LabelLineEditTextObject(this,tr("Proxy user"),proxy.user());
    proxyUser->insert(table->cellAt(8,0).firstCursorPosition());
    proxyPassword = new LabelLineEditTextObject(this,tr("Proxy password"),proxy.password());
    proxyPassword->setEchoMode(QLineEdit::Password);
    proxyPassword->insert(table->cellAt(9,0).firstCursorPosition());

    proxyActivated(proxyType->value());

    setupTableHeader(table,palette().color(QPalette::Active,QPalette::Mid),palette().buttonText().color());
    setTableRowColors(table,palette().button().color(),viewport()->palette().color(QPalette::Active,QPalette::Base));
    moveCursor(QTextCursor::Start);
    ensureCursorVisible();
}

void InfoView::proxyActivated(int index) {
    bool enabled = (index != QNetworkProxy::NoProxy);
    proxyAddress->setEnabled(enabled);
    proxyPort->setEnabled(enabled);
    proxyUser->setEnabled(enabled);
    proxyPassword->setEnabled(enabled);
}

void InfoView::hideEvent(QHideEvent *event) {
    QNetworkProxy proxy;
    proxy.setType(proxyType->value());
    proxy.setHostName(proxyAddress->lineText());
    proxy.setPort(proxyPort->value());
    proxy.setUser(proxyUser->lineText());
    proxy.setPassword(proxyPassword->lineText());
    Alpm::instance()->setDownloaderProxy(proxy);
    Alpm::instance()->setDownloaderTimeout(timeoutObj->value()*1000);
    Alpm::instance()->setDownloaderThreads(threadsObj->value());
    Alpm::instance()->setUsingSystemIcons(useSysIconsBox->isChecked());
    Alpm::instance()->setDBExtension(extObj->value());

    CustomPopupTextBrowser::hideEvent(event);
}
