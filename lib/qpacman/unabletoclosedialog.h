/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef UNABLETOCLOSEDIALOG_H
#define UNABLETOCLOSEDIALOG_H

#include <QDialog>

class QCloseEvent;
class QKeyEvent;

class UnableToCloseDialog : public QDialog {
    Q_OBJECT
public:
    explicit UnableToCloseDialog(QWidget *parent = nullptr);

public slots:
    void accept();
    void done(int code);
    int  exec();
    void open();
    void reject();

protected slots:
    virtual void reject_requested();

protected:
    void closeEvent(QCloseEvent *e);
    void keyPressEvent(QKeyEvent *e);

private:
    bool m_close;
};

#endif // UNABLETOCLOSEDIALOG_H
