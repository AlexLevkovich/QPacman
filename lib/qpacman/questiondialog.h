/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef QUESTIONDIALOG_H
#define QUESTIONDIALOG_H

#include <QMessageBox>
#include "libalpm.h"

class QuestionDialog : public QMessageBox {
    Q_OBJECT
public:
    QuestionDialog();

private slots:
    void on_question(const QString & str);
};

#endif // QUESTION_H
