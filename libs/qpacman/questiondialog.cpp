/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "questiondialog.h"
#include <QMainWindow>
#include "static.h"

QuestionDialog::QuestionDialog() : QMessageBox(Static::pApp()->findMainWindow()) {
    setWindowTitle(Static::Question_Str);
    setDefaultButton(addButton(QMessageBox::Yes));
    setEscapeButton((QAbstractButton *)addButton(QMessageBox::No));

    connect(Alpm::instance(),SIGNAL(question(const QString &,bool *)),this,SLOT(on_question(const QString &,bool *)));
}

void QuestionDialog::on_question(const QString & str,bool * answer) {
    setText(str);
    if (exec() == -1) *answer = false;
    else *answer = (standardButton(clickedButton()) == QMessageBox::Yes);
}
