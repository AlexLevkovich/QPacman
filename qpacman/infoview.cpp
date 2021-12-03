/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "infoview.h"
#include <QAbstractTextDocumentLayout>
#include <QTextCursor>
#include <QTextTable>
#include <QShowEvent>
#include "libalpm.h"
#include <QDebug>


InfoView::InfoView(QWidget *parent) : CustomPopupTextBrowser(parent) {
    document()->setDocumentMargin(20);
    table = insertTable(textCursor(),1,3,0);
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
    insertText(textCursor(),tr("You can use the following keyboad shortcuts")+":",QFont::Bold);
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

    insertText(textCursor(),tr("The following system variables are used")+":",QFont::Bold);
    table = insertTable(textCursor(),18,2,1,Qt::AlignLeft,true);
    insertText(table->cellAt(0,0).firstCursorPosition(),tr("Variable"),QFont::Bold);
    insertText(table->cellAt(0,1).firstCursorPosition(),tr("Value"),QFont::Bold);
    insertText(table->cellAt(1,0).firstCursorPosition(),"RootDir");
    insertText(table->cellAt(2,0).firstCursorPosition(),"DBPath");
    insertText(table->cellAt(3,0).firstCursorPosition(),"GPGDir");
    insertText(table->cellAt(4,0).firstCursorPosition(),"LogFile");
    insertText(table->cellAt(5,0).firstCursorPosition(),"Architecture");
    insertText(table->cellAt(8,0).firstCursorPosition(),"SigLevel");
    insertText(table->cellAt(9,0).firstCursorPosition(),"LocalFileSigLevel");
    insertText(table->cellAt(10,0).firstCursorPosition(),"RemoteFileSigLevel");
    insertText(table->cellAt(11,0).firstCursorPosition(),"HoldPkg");
    insertText(table->cellAt(12,0).firstCursorPosition(),"CacheDir");
    insertText(table->cellAt(13,0).firstCursorPosition(),"HookDir");
    insertText(table->cellAt(14,0).firstCursorPosition(),"IgnoreGroup");
    insertText(table->cellAt(15,0).firstCursorPosition(),"IgnorePkg");
    insertText(table->cellAt(16,0).firstCursorPosition(),"NoExtract");
    insertText(table->cellAt(17,0).firstCursorPosition(),"NoUpgrade");
    setupTableHeader(table,palette().color(QPalette::Active,QPalette::Mid),palette().buttonText().color());
    setTableRowColors(table,palette().button().color(),viewport()->palette().color(QPalette::Active,QPalette::Base));
    textCursor().insertBlock();
    textCursor().insertBlock();

    moveCursor(QTextCursor::Start);
    ensureCursorVisible();
}

void InfoView::showEvent(QShowEvent *event) {
    CustomPopupTextBrowser::showEvent(event);
    setCellText(table,1,1,Alpm::instance()->rootDir());
    setCellText(table,2,1,Alpm::instance()->dbPath());
    setCellText(table,3,1,Alpm::instance()->gpgDir());
    setCellText(table,4,1,Alpm::instance()->logFileName());
    setCellText(table,5,1,Alpm::instance()->arches().join(" "));
    setCellText(table,6,0,"UseSyslog",!Alpm::instance()->doUseSysLog());
    setCellText(table,7,0,"DisableDownloadTimeout",!Alpm::instance()->doDisableDownloadTimeout());
    setCellText(table,8,1,Alpm::instance()->sigLevel().join(" "));
    setCellText(table,9,1,Alpm::instance()->localFileSigLevel().join(" "));
    setCellText(table,10,1,Alpm::instance()->remoteFileSigLevel().join(" "));
    setCellText(table,11,1,Alpm::instance()->holdPkgs().join(" "));
    setCellText(table,12,1,Alpm::instance()->cacheDirs().join("\n"));
    setCellText(table,13,1,Alpm::instance()->hookDirs().join("\n"));
    setCellText(table,14,1,Alpm::instance()->ignoreGroups().join(" "));
    setCellText(table,15,1,Alpm::instance()->ignorePkgs().join(" "));
    setCellText(table,16,1,Alpm::instance()->noExtractPkgs().join(" "));
    setCellText(table,17,1,Alpm::instance()->noUpgradePkgs().join(" "));
}
