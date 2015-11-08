/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "toolbarrightwidget.h"
#include "ui_toolbarrightwidget.h"
#include <QMessageBox>
#include "pacmanhelpdialog.h"

extern const char * pacman_version;

ToolbarRightWidget::ToolbarRightWidget(QWidget *parent) : QWidget(parent), ui(new Ui::ToolbarRightWidget) {
    ui->setupUi(this);

    menu.addAction(QIcon(":/pics/help-hint.png"),tr("Help..."),this,SLOT(onHelp()));
    menu.addAction(QIcon(":/pics/Pacman-arch_logo.png"),tr("About..."),this,SLOT(onAboutDlg()));
    ui->toolButton->setMenu(&menu);
}

ToolbarRightWidget::~ToolbarRightWidget() {
    delete ui;
}

void ToolbarRightWidget::onHelp() {
    PacmanHelpDialog(this).exec();
}

void ToolbarRightWidget::onAboutDlg() {
    QMessageBox::about(this,tr("About QPacman..."),tr("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">QPacman</span> is GUI frontend to <a href=\"https://www.archlinux.org/\"><span style=\" text-decoration: underline; color:#0057ae;\">Archlinux</span></a>'s <a href=\"http://www.archlinux.org/pacman/\"><span style=\" font-weight:600; text-decoration: underline; color:#0057ae;\">Pacman</span></a><span style=\" font-weight:600;\"> </span>package manager.</p>"
                                                      "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">QPacman version is %1.</p>"
                                                      "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Developer: Alex Levkovich (<a href=\"mailto:alevkovich@tut.by\"><span style=\" text-decoration: underline; color:#0057ae;\">alevkovich@tut.by</span></a>)</p>"
                                                      "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">License: GPL</p>").arg(pacman_version));
}
