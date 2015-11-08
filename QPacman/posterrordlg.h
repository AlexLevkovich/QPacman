/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef POSTERRORDLG_H
#define POSTERRORDLG_H

#include <QObject>

class PostErrorDlg : public QObject {
    Q_OBJECT
public:
    explicit PostErrorDlg(const QString & error,const QString & command);

private slots:
    void _show_error(const QString & error,const QString & command);
};

#endif // POSTERRORDLG_H
