/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef PACKAGEVIEW_H
#define PACKAGEVIEW_H

#include <QTreeView>
#include <QIcon>
#include <QTimer>
#include "alpmpackage.h"
#include "libalpm.h"

class PackageView;
class QEvent;
class InstallButtonDelegate;

class PackageItemModel : public QAbstractItemModel {
    Q_OBJECT
public:
    PackageItemModel(PackageView * parent);
    bool setData(const QModelIndex & index,const QVariant & value,int role = Qt::EditRole);
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole ) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    void chooseRow(const QModelIndex & index,bool sel,AlpmPackage::UserChangeStatus status = AlpmPackage::DO_NOTHING);
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    AlpmPackage & row(const QModelIndex & index);
    bool removeRows(int row,int count,const QModelIndex &parent = QModelIndex());
    void appendRow(const AlpmPackage & pkg);
    AlpmPackage installedPackageByName(const QString & package) const;

    static const QString statusText(const AlpmPackage & pkg);
    static const QString statusTextHint(AlpmPackage::UserChangeStatus status);
    static const QIcon statusIcon(const AlpmPackage & pkg);

    static QString DO_INSTALL_STR;
    static QString DO_INSTALL_ASDEPS_STR;
    static QString DO_INSTALL_ASDEPS_FORCE_STR;
    static QString DO_INSTALL_FORCE_STR;
    static QString DO_REINSTALL_STR;
    static QString DO_REINSTALL_ASDEPS_STR;
    static QString DO_UNINSTALL_STR;
    static QString DO_UNINSTALL_ALL_STR;
    static QIcon install_icon;
    static QIcon reinstall_icon;
    static QIcon uninstall_icon;
    static QIcon installedIcon;
    static QIcon notinstalledIcon;
    static QIcon updatedIcon;

protected:
    QModelIndex indexByPackageNameVersion(const AlpmPackage & pkg) const;
    QModelIndex indexByPackageName(const AlpmPackage & pkg) const;
    QModelIndex parent(const QModelIndex & index) const;
    virtual QVariant headerData(int section,Qt::Orientation orientation,int role = Qt::DisplayRole) const;
    bool setHeaderData(int section,Qt::Orientation orientation,const QVariant & value,int role = Qt::EditRole);

private:
    static bool pkg_name_less(const AlpmPackage & pkg1,const AlpmPackage & pkg2);
    static bool pkg_name_version_less(const AlpmPackage & pkg1,const AlpmPackage & pkg2);
    static const QString status_text(AlpmPackage::UserChangeStatus status);
    static const QIcon status_icon(AlpmPackage::UserChangeStatus status);

    enum Columns {
        Name = 0,
        Description,
        Version,
        Repository,
        Action
    };

    QStringList m_groups;
    QMap<AlpmPackage::Dependence,QVector<qint64> > m_provides;
    QList<AlpmPackage> rows;
    AlpmPackage null_pkg;
    PackageView * m_parent;
    QSize m_icon_size;

    static bool first_time_init;

    friend class PackageView;
    friend class ReinstallOptionsMenu;
    friend class UninstallOptionsMenu;
    friend class InstallOptionsMenu;
    friend class OptionsWidget;
};

class PackageView : public QTreeView {
    Q_OBJECT
public:
    class SelectionState {
    public:
        SelectionState(const SelectionState & state);
        SelectionState(const QString & name = QString(),AlpmPackage::SearchFieldType fieldType = AlpmPackage::NAME,AlpmPackage::PackageFilter filter = AlpmPackage::IS_ALL,const QString & group = QString(),const QString & repo = QString());
        void setPackage(const AlpmPackage & pkg);
        AlpmPackage package() const;
        bool operator==(const SelectionState & state) const;
        SelectionState & operator=(const SelectionState & state);
    private:
        QString m_name;
        AlpmPackage::SearchFieldType m_fieldType;
        AlpmPackage::PackageFilter m_filter;
        QString m_group;
        QString m_repo;
        AlpmPackage m_pkg;

        friend class PackageView;
    };

    PackageView(QWidget *parent = NULL);
    bool isSelectPrevPossible();
    bool isSelectNextPossible();
    void refreshRows(const QString & name,AlpmPackage::SearchFieldType fieldType,AlpmPackage::PackageFilter filter,const QString & group,const QString & repo);
    void revertPackageReason(const QString & pkgname);
    void selectPackageByDep(const AlpmPackage::Dependence & dep);
    void markPackageByNameToInstall(const QString & pkgname);

public slots:
    void clear();
    void refreshRows();
    void markAll();
    void resetAll();
    void selectPrev();
    void selectNext();

protected:
    void setModel(QAbstractItemModel * model);
    QModelIndex selectedRow() const;
    void selectionChanged(const QItemSelection & selected,const QItemSelection & deselected);

private slots:
    void selectTimeout();
    void package_queried(const AlpmPackage & pkg);
    void packages_queried(const QString & name,ThreadRun::RC rc);
    void refresh_needed();

signals:
    void selectionChanged(const AlpmPackage & pkg);
    void rowChoosingStateChanged(const QModelIndex &index);
    void refreshBeginning();
    void refreshCompleted();
    void search_changed(const QString &,AlpmPackage::SearchFieldType,AlpmPackage::PackageFilter,const QString &,const QString &);

private:
    void refreshRows(const SelectionState & state);
    void selectPackageByState(const SelectionState & state);
    void refreshState(const SelectionState & state);

    PackageItemModel * m_model;
    InstallButtonDelegate * delegate;
    QTimer selectTimer;
    bool m_is_refreshing;

    QList<SelectionState> history_items;
    bool history_disabled;
    int history_index;
    int history_count;
    SelectionState currentSelectionState;

    friend class InstallButtonDelegate;
};

#endif // PACKAGEVIEW_H
