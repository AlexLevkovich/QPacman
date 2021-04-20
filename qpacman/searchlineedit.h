/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QLineEdit>
#include <QTimer>

class SearchLineEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit SearchLineEdit(QWidget *parent = 0);

protected slots:
    void onTextEdited(const QString & str);
    void onTimeout();

signals:
    void changed(const QString &);

private:
    QTimer timer;
};

#endif // SEARCHLINEEDIT_H
