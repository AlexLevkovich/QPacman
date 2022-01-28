/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "searchlineedit.h"
#include <QTimer>

SearchLineEdit::SearchLineEdit(QWidget *parent) : QLineEdit(parent) {
    connect(this,&QLineEdit::textEdited,this,&SearchLineEdit::onTextEdited);
    connect(&timer,&QTimer::timeout,this,&SearchLineEdit::onTimeout);
}

void SearchLineEdit::onTextEdited(const QString & /*str*/) {
    timer.stop();
    timer.start(1000);
}

void SearchLineEdit::onTimeout() {
    emit changed(text());
    timer.stop();
}
