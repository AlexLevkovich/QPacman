/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef OPTIONALDEPSDLG_H
#define OPTIONALDEPSDLG_H

#include <QDialog>
#include <QMap>

class QAbstractButton;
class QTreeWidgetItem;
typedef QMap<QString,QString > StringStringMap;

namespace Ui {
class OptionalDepsDlg;
}

class OptionalDepsDlg : public QDialog {
    Q_OBJECT

public:
    OptionalDepsDlg();
    ~OptionalDepsDlg();
    void fill(const QString & pkgname,const StringStringMap & installed_deps,const StringStringMap & pending_deps);
    void setProcessingEnabled();

private slots:
    void rowsChanged();
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_depsWidget_itemChanged(QTreeWidgetItem *item, int column);

signals:
    void selected(const QStringList & pkgs);

private:
    void emit_selected();
    QStringList listedPackages() const;

    Ui::OptionalDepsDlg *ui;
    bool m_opt_deps_enabled;
    bool m_processing_completed;
};

#endif // OPTIONALDEPSDLG_H
