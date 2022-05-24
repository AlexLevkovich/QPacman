/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef PROGRESSMODEL_H
#define PROGRESSMODEL_H

#include <QStandardItemModel>
#include <QTreeView>
#include <QProgressBar>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QFlags>

class QAbstractItemView;
class ProgressModel;
class ProgressView;

class ProgressDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    ProgressDelegate(ProgressView * parent);
    void paint(QPainter *painter,const QStyleOptionViewItem &option,const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,const QModelIndex &index) const;

private:
    ProgressView * view() const;

    friend class ProgressView;
};

class SimpleItem : public QStandardItem {
public:
    enum Type {
        Average = 1024,
        Download = 2048,
        Event = 4096,
        Hook = 8192,
        Text = 16384,
        Flat = 32768,
        Install = 65536,
        Remove = 131072,
        Progress = 262144
    };
    Q_DECLARE_FLAGS(Types,Type)

    SimpleItem(const QString & text = QString());
    SimpleItem(const QIcon & icon,const QString & text = QString());
    virtual int type() const = 0;
    ProgressModel * progressModel();
    ProgressView * progressView();
};
Q_DECLARE_OPERATORS_FOR_FLAGS(SimpleItem::Types)

class SimpleTextItem : public SimpleItem {
public:
    SimpleTextItem(const QString & text = QString());
    SimpleTextItem(const QIcon & icon,const QString & text = QString());
    int type() const;
};

class FlatItem : public SimpleItem {
public:
    FlatItem();
    int type() const;
};

class SimpleProgressItem : 	public SimpleItem {
public:
    SimpleProgressItem(int min = 0,int max = 0,int value = 0);
    int minimum() const;
    int maximum() const;
    int value() const;
    void setValue(int value);
    void setMinimum(int min);
    void setMaximum(int max);
    void setRange(int min,int max);
    void setParameters(int value,int min = 0,int max = 0);
    void setMax();
    bool isDefined() const;
    bool isBusy() const;
    int type() const;
    void setType(int type);
    QString message() const;
    void setMessage(const QString & message);
    void setShowPercents(bool show);
    bool isPercentsShown() const;

private:
    int m_min;
    int m_max;
    bool no_blank;
    bool show_percents;
    SimpleItem::Types m_type;
    QTimer m_timer;
    QProgressBar m_renderer;

    friend class ProgressDelegate;
};

class ProgressModel : public QStandardItemModel {
    Q_OBJECT
public:
    ProgressModel(ProgressView *parent);
};

class ProgressView : public QTreeView {
    Q_OBJECT
public:
    ProgressView(QWidget *parent = nullptr,bool clear_on_hide = true);
    void setProgressColumnWidth(quint64 width);
    SimpleTextItem     * appendHookRow(const QString & text);
    SimpleProgressItem * appendAverageHookProgressRow(const QString & text,int min = 0,int max = 0,int value = 0);
    SimpleProgressItem * appendEventProgressRow(const QString & text,int min = 0,int max = 0,int value = 0);
    SimpleProgressItem * appendDownloadProgressRow(const QString & text,int min = 0,int max = 0,int value = 0);
    SimpleProgressItem * appendAverageDownloadProgressRow(const QString & text,int min = 0,int max = 0,int value = 0);
    SimpleProgressItem * appendInstallProgressRow(const QString & text,int min = 0,int max = 0,int value = 0);
    SimpleProgressItem * appendAverageInstallProgressRow(const QString & text,int min = 0,int max = 0,int value = 0);
    SimpleProgressItem * appendRemoveProgressRow(const QString & text,int min = 0,int max = 0,int value = 0);
    SimpleProgressItem * appendAverageRemoveProgressRow(const QString & text,int min = 0,int max = 0,int value = 0);
    SimpleTextItem * appendInformationRow(const QString & text);
    SimpleTextItem * appendErrorRow(const QString & text);
    bool moveRowAtEnd(QStandardItem * item);
    SimpleProgressItem * previousProgressItem(QStandardItem * item);
    void clear();
    bool doClearOnHide() const;
    void setClearOnHide(bool flag);

signals:
    void rootItemsCountChanged(quint64 count);
    void rowAdded(const QModelIndex & item0,const QModelIndex & item1);

private slots:
    void rowsChanged();
    void rowsInserted(const QModelIndex & parent,int first,int last);
    void itemChanged(QStandardItem * item);

protected:
    void hideEvent(QHideEvent * event);
    void setModel(QAbstractItemModel *model);

private:
    SimpleProgressItem * appendProgressRow(const QString & text,int min = 0,int max = 0,int value = 0,const QIcon & icon = QIcon());
    static QList<QStandardItem *> createProgressRow(const QString & text,int min = 0,int max = 0,int value = 0,const QIcon & icon = QIcon());

    ProgressModel * m_model;
    quint64 old_rows_count;
    bool m_clear_on_hide;
};

#endif // PROGRESSMODEL_H
