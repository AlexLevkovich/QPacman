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

    connect(Alpm::instance(),&Alpm::question,this,&QuestionDialog::on_question);
}

void QuestionDialog::on_question(const QString & str) {
    setText(str);
    Alpm::instance()->answer((exec() == -1)?false:(standardButton(clickedButton()) == QMessageBox::Yes));
}
