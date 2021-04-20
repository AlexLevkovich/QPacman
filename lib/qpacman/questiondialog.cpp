/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "questiondialog.h"
#include <QMainWindow>
#include "singleapplication.h"

QuestionDialog::QuestionDialog() : QMessageBox(SingleApplication::findMainWindow()) {
    setWindowTitle(tr("Question..."));
    setDefaultButton(addButton(QMessageBox::Yes));
    setEscapeButton((QAbstractButton *)addButton(QMessageBox::No));

    connect(Alpm::instance(),SIGNAL(question(const QString &)),this,SLOT(on_question(const QString &)));
}

void QuestionDialog::on_question(const QString & str) {
    setText(str);
    Alpm::instance()->answer((exec() == -1)?false:(standardButton(clickedButton()) == QMessageBox::Yes));
}
