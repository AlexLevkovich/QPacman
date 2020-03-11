/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanView_H
#define PacmanView_H

#include <QTreeView>
#include <QTimer>
#include <QVector>
#include "pacmanitemmodel.h"

class InstallButtonDelegate;
class PacmanProcessReader;
class QMainWindow;
class QGraphicsView;

#include "filtertoolbutton.h"
#include "categorytoolbutton.h"

class PacmanView : public QTreeView {
    Q_OBJECT
public:
    explicit PacmanView(QWidget *parent = 0);
    void selectPackage(const AlpmPackage::Dependence & pkg);
    void selectPrev();
    void selectNext();
    bool isSelectPrevPossible();
    bool isSelectNextPossible();
    void selectPackageByNameVersion(const QString & name,const QString & version);
    void showAllRows();
    void markRow(const QModelIndex & index);
    void resetRow(const QModelIndex & index);
    void markAll();
    void resetAll();
    bool isRefreshing() const;
    void markedPackages(QList<AlpmPackage *> & install,QList<AlpmPackage *> & install_asdeps,QList<AlpmPackage *> & install_forced,QList<AlpmPackage *> & removeall,QList<AlpmPackage *> & remove);
    void markedPackages(QMap<AlpmPackage *,AlpmPackage::UserChangeStatus> & pkgs);
    QList<AlpmPackage *> markedPackagesToInstall() const;
    QList<AlpmPackage *> markedPackagesToInstallAsDeps() const;
    QList<AlpmPackage *> markedPackagesToInstallForced() const;
    QList<AlpmPackage *> markedPackagesToRemoveAll() const;
    QList<AlpmPackage *> markedPackagesToRemove() const;
    AlpmPackage * row(const QModelIndex & index);
    QModelIndex selectedRow() const;
    int visibleRowIndex(const QModelIndex & index) const;
    bool isVisibleIndex(const QModelIndex & index) const;
    QStringList files(const QString & package);
    void revertReason(const QString & package);
    QModelIndex indexByPackageNameVersion(const QString & name,const QString & ver) const;
    QModelIndex installedIndexByPackageName(const QString & package) const;
    void markPackageByNameToInstall(const QString & package);

public slots:
    void refresh(bool recover_operations = true);

protected slots:
    void read_packages_finished();
    void selectionChanged(const QItemSelection & selected,const QItemSelection & deselected);
    void onScrollBarReleased();
    void onScrollBarPressed();
    void onScrollBarValueChanged(int value);
    void scrollTimeout();

public slots:
    void applyFilter(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo);

signals:
    void selectionChanged(const QModelIndex & index);
    void rowChoosingStateChanged(const QModelIndex &index);
    void refreshMaybeStarted(bool * ok);
    void refreshBeginning();
    void refreshCompleted();

private:
    struct PkgParm {
        QString name;
        QString version;
        QString repo;

        bool operator<(const PkgParm & other) const {
            int ret;
            if ((ret = name.compare(other.name)) == 0) {
                if ((ret = version.compare(other.version)) == 0) {
                    return (version < other.version);
                }
            }

            return (ret < 0);
        }
    };


    InstallButtonDelegate * delegate;
    PacmanItemModel * m_model;
    bool scrollPressed;
    AlpmPackage::Dependence _selEntry;
    QTimer scrollTimer;
    QVector<AlpmPackage::Dependence> history_items;
    bool history_disabled;
    int history_index;
    bool m_is_refreshing;
    QMap<PkgParm,AlpmPackage::UserChangeStatus> m_pre_refresh_pkgs;
    bool m_recover_operations;

    void setHeaderSections();
    void recoverMarkedPackages();
    void saveMarkedPackages(const QMap<AlpmPackage *,AlpmPackage::UserChangeStatus> & pkgs);
};

#endif // PacmanView_H
