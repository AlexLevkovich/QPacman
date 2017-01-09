/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "searchlineedit.h"
#include <QTimer>

SearchLineEdit::SearchLineEdit(QWidget *parent) : QLineEdit(parent) {
    connect(this,SIGNAL(textEdited(const QString &)),this,SLOT(onTextEdited(const QString &)));
    connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
}

void SearchLineEdit::onTextEdited(const QString & /*str*/) {
    timer.stop();
    timer.start(1000);
}

void SearchLineEdit::onTimeout() {
    emit changed(text());
    timer.stop();
}
