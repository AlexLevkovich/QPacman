/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PacmanView_H
#define PacmanView_H

#include <QTreeView>
#include <QTimer>
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
    ~PacmanView();
    void selectPackageByName(const QString & package);
    void selectPackageByEntry(const PacmanEntry & entry);
    void showAllRows();
    void markRow(const QModelIndex & index);
    void resetRow(const QModelIndex & index);
    void markAll();
    void resetAll();
    QList<PacmanEntry> markedPackagesToInstall() const;
    QList<PacmanEntry> markedPackagesToRemoveAll() const;
    QList<PacmanEntry> markedPackagesToRemove() const;
    PacmanEntry row(const QModelIndex & index) const;
    QModelIndex selectedRow() const;
    int visibleRowIndex(const QModelIndex & index) const;
    bool isVisibleIndex(const QModelIndex & index) const;
    QList<PacmanEntry::InstalledStatus> findPackagesInstalledStatus(const QStringList & packages) const;
    QStringList files(const QString & package);
    void revertReason(const QString & package);
    void checkReplaces();
    QStringList updateNamesList() const;
    QModelIndex indexByPackageNameVersion(const QString & name,const QString & ver) const;
    QModelIndex installedIndexByPackageName(const QString & package) const;
    void setWaitView(QGraphicsView * view) {
        waitView = view;
    }

public slots:
    void refresh();

protected:
    void keyPressEvent(QKeyEvent * event);

protected slots:
    void read_package(const PacmanEntry & item);
    void read_packages_finished(PacmanProcessReader * ptr);
    void read_files_finished(PacmanProcessReader * ptr);
    void files_ready(const QString & package,const QStringList & files);
    void selectionChanged(const QItemSelection & selected,const QItemSelection & deselected);
    void onScrollBarReleased();
    void onScrollBarPressed();
    void onScrollBarValueChanged(int value);
    void scrollTimeout();

public slots:
    void onSearchChanged(const QString & text,CategoryToolButton::ItemId cItemId,FilterToolButton::ItemId fItemId,const QString & group,const QString & repo);

signals:
    void canFillFilters(const QStringList & groups);
    void canFillRepos(const QStringList & groups);
    void selectionChanged(const QModelIndex & index);
    void rowChoosingStateChanged(const QModelIndex &index);
    void refreshCompleted();
    void enableActions(bool flag);

private:
    InstallButtonDelegate * delegate;
    PacmanItemModel * model;
    QItemSelectionModel * prev_sel_model;
    PacmanItemModel * prev_model;
    bool scrollPressed;
    PacmanEntry _selEntry;
    QTimer scrollTimer;
    QGraphicsView * waitView;

    void setHeaderSections();
};

#endif // PacmanView_H
