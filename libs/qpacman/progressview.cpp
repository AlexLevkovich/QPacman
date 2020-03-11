/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "progressview.h"
#include <QApplication>
#include <QPainter>
#include <QHeaderView>
#include <QDebug>
#include <QAbstractItemView>
#include "themeicons.h"

#ifdef USE_KDE
#include <KColorScheme>
#endif

ProgressDelegate::ProgressDelegate(ProgressView * parent) : QStyledItemDelegate(parent) {}

ProgressView * ProgressDelegate::view() const {
    return ((ProgressView *)parent());
}

void ProgressDelegate::paint(QPainter *painter,const QStyleOptionViewItem &option,const QModelIndex &index) const {
    SimpleItem * item = dynamic_cast<SimpleItem *>(((QStandardItemModel *)index.model())->itemFromIndex(index));
    QString text = index.data(Qt::DisplayRole).toString();
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    int decorationSize = option.fontMetrics.height() + 4;
    if (item != NULL && item->type() <= QStandardItem::UserType) QStyledItemDelegate::paint(painter,option,index);
    else if (item != NULL && (item->type() & SimpleItem::Progress)) {
        QWidget * viewport = view()->viewport();
        if (viewport != NULL) {
            SimpleProgressItem * progress_item = (SimpleProgressItem *)item;
            progress_item->m_renderer.resize(option.rect.width(),option.rect.height());
            QPalette pal = progress_item->m_renderer.palette();
            pal.setColor(QPalette::Background,viewport->palette().color(QPalette::Base));
            progress_item->m_renderer.setPalette(pal);
            progress_item->m_renderer.setRange(progress_item->minimum(),progress_item->maximum());
            bool isNumber;
            int number = text.toInt(&isNumber);
            progress_item->m_renderer.setValue(isNumber?number:0);
            progress_item->m_renderer.setTextVisible(progress_item->isPercentsShown());
            painter->save();
            painter->translate(option.rect.topLeft());
            progress_item->m_renderer.render(painter);
            painter->restore();
        }
    }
    else if (item != NULL && (item->type() & SimpleItem::Text)) {
        if (!icon.isNull()) painter->drawPixmap(option.rect.x()+1,option.rect.y(),icon.pixmap(decorationSize));
        QRect textRect(option.rect);
        textRect.setX(textRect.x()+1);
        if (!icon.isNull()) textRect.setX(textRect.x()+decorationSize+1);
        QPen pen = painter->pen();
        pen.setColor(index.data(Qt::ForegroundRole).value<QBrush>().color());
        painter->setPen(pen);
        painter->drawText(textRect,Qt::AlignLeft|Qt::AlignVCenter,text);
    }
    else if (item != NULL && (item->type() & SimpleItem::Flat)) {
        QRect rect(option.rect);
        int height = option.fontMetrics.height();
        rect.setTop(rect.y()+((rect.height()-height)/2));
        rect.setHeight(height);
        painter->fillRect(rect,index.data(Qt::ForegroundRole).value<QBrush>());
    }
    else QStyledItemDelegate::paint(painter,option,index);
}

QSize ProgressDelegate::sizeHint(const QStyleOptionViewItem &option,const QModelIndex &index) const {
    ProgressModel * model = (ProgressModel *)index.model();
    SimpleItem * item = dynamic_cast<SimpleItem *>(model->itemFromIndex(index));
    int decorationSize = option.fontMetrics.height() + 4;
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    if (item != NULL && item->type() > QStandardItem::UserType) {
        if (item != NULL && (item->type() & SimpleItem::Progress)) {
            return QSize(option.fontMetrics.horizontalAdvance(model->headerData(index.column(),Qt::Horizontal).toString())+10,decorationSize+2);
        }
        else if (item != NULL && (item->type() & SimpleItem::Text)) {
            return QSize(option.fontMetrics.horizontalAdvance(index.data(Qt::DisplayRole).toString())+(icon.isNull()?0:decorationSize)+4,decorationSize+2);
        }
    }
    return QSize(QStyledItemDelegate::sizeHint(option,index).width(),decorationSize+2);
}

SimpleItem::SimpleItem(const QString & text) : QStandardItem(text) {}

SimpleItem::SimpleItem(const QIcon & icon,const QString & text) : QStandardItem(icon,text) {}

ProgressModel * SimpleItem::progressModel() {
    return dynamic_cast<ProgressModel *>(model());
}

ProgressView * SimpleItem::progressView() {
    ProgressModel * model = progressModel();
    if (model == NULL) return NULL;
    return dynamic_cast<ProgressView *>(model->parent());
}

SimpleTextItem::SimpleTextItem(const QString & text) : SimpleItem(text) {
    setEditable(false);
}

SimpleTextItem::SimpleTextItem(const QIcon & icon,const QString & text) : SimpleItem(icon,text) {
    setEditable(false);
}

int SimpleTextItem::type() const {
    return (int)SimpleItem::Text;
}

FlatItem::FlatItem() : SimpleItem() {}

int FlatItem::type() const {
    return (int)SimpleItem::Flat;
}

SimpleProgressItem::SimpleProgressItem(int min,int max,int value) : SimpleItem() {
    setMinimum(min);
    setMaximum(max);
    setValue(value);

    m_type = SimpleItem::Download;
    no_blank = true;
    show_percents = false;
    setEditable(false);

    int anim_timeout = QApplication::style()->styleHint(QStyle::SH_Widget_Animation_Duration);
    if (anim_timeout > 0) {
        QObject::connect(&m_timer,&QTimer::timeout,[&]() { ProgressModel * model = progressModel();
                                                           if (model == NULL) return;
                                                           QModelIndex index = model->indexFromItem(this);
                                                           if (!index.isValid()) return;
                                                           emit model->dataChanged(index,index);
                                                         });
        m_timer.start(anim_timeout);
    }
}

int SimpleProgressItem::type() const {
    return (int)m_type;
}

void SimpleProgressItem::setType(int type) {
    m_type = (SimpleItem::Types)(type | SimpleItem::Progress);
}

int SimpleProgressItem::minimum() const {
    return m_min;
}

int SimpleProgressItem::maximum() const {
    return m_max;
}

int SimpleProgressItem::value() const {
    return text().toInt();
}

void SimpleProgressItem::setValue(int _value) {
    int value = (_value > m_max)?m_max:_value;
    value = (_value < m_min)?m_min:_value;
    setText(no_blank?QString("%1 ").arg(value):QString("%1").arg(value));
    no_blank = !no_blank;
}

void SimpleProgressItem::setShowPercents(bool show) {
    show_percents = show;
}

bool SimpleProgressItem::isPercentsShown() const {
    return show_percents;
}

void SimpleProgressItem::setMinimum(int min) {
    m_min = min;
    if (m_min > m_max) m_max = m_min;
    setValue(value());
}

void SimpleProgressItem::setMaximum(int max) {
    m_max = max;
    if (m_max < m_min) m_min = m_max;
    if (m_max == 1 && m_min == 0) m_max = 0;
    setValue(value());
}

void SimpleProgressItem::setParameters(int value,int min,int max) {
    setMinimum(min);
    setMaximum(max);
    setValue(value);
}

void SimpleProgressItem::setMax() {
    if (m_max <= 0) {
        m_max = 100;
        m_min = 0;
    }
    setValue(m_max);
}

void SimpleProgressItem::setRange(int min,int max) {
    setMinimum(min);
    setMaximum(max);
    setValue(value());
}

bool SimpleProgressItem::isDefined() const {
    return (m_min < m_max);
}

bool SimpleProgressItem::isBusy() const {
    return (m_min == 0 && m_max == 0);
}

QString SimpleProgressItem::message() const {
    QStandardItem * item = parent()->child(row(),0);
    return (item == NULL)?QString():item->text();
}

void SimpleProgressItem::setMessage(const QString & message) {
    QStandardItem * parent = this->parent();
    if (parent == NULL) {
        QStandardItemModel * model = this->model();
        if (model != NULL) {
            QStandardItem * item = model->itemFromIndex(model->index(row(),0));
            if (item != NULL) item->setText(message);
        }
    }
    else {
        QStandardItem * item = parent->child(row(),0);
        if (item != NULL) item->setText(message);
    }
}

ProgressModel::ProgressModel(ProgressView *parent) :  QStandardItemModel(0,2,parent) {
    setHorizontalHeaderItem(0,new QStandardItem(tr("Description")));
    setHorizontalHeaderItem(1,new QStandardItem(tr("Progress")));
}

ProgressView::ProgressView(QWidget *parent,bool clear_on_hide) : QTreeView(parent) {
    old_rows_count = 0;
    m_clear_on_hide = clear_on_hide;

    QTreeView::setModel((m_model = new ProgressModel(this)));
    setRootIsDecorated(false);
    setItemsExpandable(false);
    header()->setSectionsMovable(false);
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(0,QHeaderView::Fixed);
    header()->setSectionResizeMode(1,QHeaderView::Fixed);

    QAbstractItemDelegate * old_delegate = itemDelegate();
    setItemDelegate(new ProgressDelegate(this));
    if (old_delegate != NULL) old_delegate->deleteLater();

    connect(m_model,SIGNAL(rowsInserted(const QModelIndex &,int,int)),this,SLOT(rowsChanged()));
    connect(m_model,SIGNAL(rowsInserted(const QModelIndex &,int,int)),this,SLOT(rowsInserted(const QModelIndex &,int,int)),Qt::QueuedConnection);
    connect(m_model,SIGNAL(rowsRemoved(const QModelIndex &,int,int)),this,SLOT(rowsChanged()));
    connect(m_model,SIGNAL(itemChanged(QStandardItem *)),this,SLOT(itemChanged(QStandardItem *)));
}

SimpleProgressItem * ProgressView::appendProgressRow(const QString & text,int min,int max,int value,const QIcon & icon) {
    QList<QStandardItem *> items;
    m_model->appendRow((items = createProgressRow(text,min,max,value,icon)));
    expandAll();
    scrollTo(m_model->index(m_model->rowCount()-1,0));
    return (SimpleProgressItem *)items.at(1);
}

SimpleTextItem * ProgressView::appendHookRow(const QString & text) {
    SimpleTextItem * ret;
    m_model->appendRow(QList<QStandardItem *>() << (ret = new SimpleTextItem(ThemeIcons::get(ThemeIcons::HOOK_ITEM),text)) << new FlatItem());
    return ret;
}

SimpleProgressItem * ProgressView::appendAverageHookProgressRow(const QString & text,int min,int max,int value) {
    SimpleProgressItem * item = appendProgressRow(text,min,max,value,ThemeIcons::get(ThemeIcons::HOOK_AVERAGE_ITEM));
    item->setType(SimpleItem::Hook|SimpleItem::Average);
    return item;
}

SimpleProgressItem * ProgressView::appendEventProgressRow(const QString & text,int min,int max,int value) {
    SimpleProgressItem * item = appendProgressRow(text,min,max,value,ThemeIcons::get(ThemeIcons::IMPORTANT));
    item->setType(SimpleItem::Event);
    return item;
}

SimpleProgressItem * ProgressView::appendDownloadProgressRow(const QString & text,int min,int max,int value) {
    SimpleProgressItem * item = appendProgressRow(text,min,max,value,ThemeIcons::get(ThemeIcons::DOWNLOAD_ITEM));
    item->setType(SimpleItem::Download);
    return item;
}

SimpleProgressItem * ProgressView::appendAverageDownloadProgressRow(const QString & text,int min,int max,int value) {
    SimpleProgressItem * item = appendProgressRow(text,min,max,value,ThemeIcons::get(ThemeIcons::DOWNLOAD_AVERAGE));
    item->setType(SimpleItem::Download|SimpleItem::Average);
    return item;
}

SimpleProgressItem * ProgressView::appendInstallProgressRow(const QString & text,int min,int max,int value) {
    SimpleProgressItem * item = appendProgressRow(text,min,max,value,ThemeIcons::get(ThemeIcons::PKG_INSTALLED));
    item->setType(SimpleItem::Install);
    return item;
}

SimpleProgressItem * ProgressView::appendAverageInstallProgressRow(const QString & text,int min,int max,int value) {
    SimpleProgressItem * item = appendProgressRow(text,min,max,value,ThemeIcons::get(ThemeIcons::INSTALL_AVERAGE_ITEM));
    item->setType(SimpleItem::Install|SimpleItem::Average);
    return item;
}

SimpleProgressItem * ProgressView::appendRemoveProgressRow(const QString & text,int min,int max,int value) {
    SimpleProgressItem * item = appendProgressRow(text,min,max,value,ThemeIcons::get(ThemeIcons::PKG_REMOVED));
    item->setType(SimpleItem::Remove);
    return item;
}

SimpleProgressItem * ProgressView::appendAverageRemoveProgressRow(const QString & text,int min,int max,int value) {
    SimpleProgressItem * item = appendProgressRow(text,min,max,value,ThemeIcons::get(ThemeIcons::REMOVE_AVERAGE_ITEM));
    item->setType(SimpleItem::Remove|SimpleItem::Average);
    return item;
}

QList<QStandardItem *> ProgressView::createProgressRow(const QString & text,int min,int max,int value,const QIcon & icon) {
    return QList<QStandardItem *>() << new SimpleTextItem(icon,text) << new SimpleProgressItem(min,max,value);
}

SimpleTextItem * ProgressView::appendInformationRow(const QString & text) {
    QList<QStandardItem *> ret;
    QBrush color(QColor(230,126,34),Qt::Dense6Pattern);
#ifdef USE_KDE
    color = KColorScheme(QPalette::Active).foreground(KColorScheme::NeutralText);
    color.setStyle(Qt::Dense6Pattern);
#endif
    ret << new SimpleTextItem(ThemeIcons::get(ThemeIcons::IMPORTANT),text) << new FlatItem();
    ((SimpleTextItem *)ret[0])->setForeground(color);
    m_model->appendRow(ret);
    expandAll();
    scrollTo(m_model->index(m_model->rowCount()-1,0));
    return (SimpleTextItem *)ret.at(0);
}

SimpleTextItem * ProgressView::appendErrorRow(const QString & text) {
    QList<QStandardItem *> ret;
    QBrush color(QColor(191,3,3),Qt::Dense6Pattern);
#ifdef USE_KDE
    color = KColorScheme(QPalette::Active).foreground(KColorScheme::NegativeText);
    color.setStyle(Qt::Dense6Pattern);
#endif

    ret << new SimpleTextItem(ThemeIcons::get(ThemeIcons::ERROR_ITEM),text) << new FlatItem();
    ((SimpleTextItem *)ret[0])->setForeground(color);
    m_model->appendRow(ret);
    expandAll();
    scrollTo(m_model->index(m_model->rowCount()-1,0));
    return (SimpleTextItem *)ret.at(0);
}

void ProgressView::setProgressColumnWidth(quint64 width) {
    setColumnWidth(1,width);
}

bool ProgressView::moveRowAtEnd(QStandardItem * item) {
    if (item == NULL) return false;

    if (item->index().parent().isValid()) {
        item->parent()->appendRow(item->parent()->takeRow(item->row()));
    }
    else m_model->appendRow(m_model->takeRow(item->row()));

    return true;
}

void ProgressView::clear() {
    if(!m_model->hasChildren()) return;
    m_model->removeRows(0, m_model->rowCount());
    header()->resizeSection(0,header()->defaultSectionSize());
}

void ProgressView::rowsChanged() {
    if (old_rows_count != (quint64)m_model->rowCount()) {
        old_rows_count = (quint64)m_model->rowCount();
        emit rootItemsCountChanged(old_rows_count);
    }
}

SimpleProgressItem * ProgressView::previousProgressItem(QStandardItem * item) {
    if (item == NULL) return NULL;
    if (item->row() <= 0) return NULL;
    if (item->column() != 1) return NULL;

    do {
        item = m_model->itemFromIndex(m_model->index(item->row()-1,item->column(),(item->parent() == NULL)?QModelIndex():item->parent()->index()));
        if ((item != NULL) && (item->type() > QStandardItem::UserType) && (item->type() & SimpleItem::Progress)) return (SimpleProgressItem *)item;
    } while (item != NULL);

    return NULL;
}

void ProgressView::rowsInserted(const QModelIndex & parent,int first,int last) {
    int section0 = header()->sectionSize(0);
    ProgressDelegate * delegate = (ProgressDelegate *)itemDelegate();
    QModelIndex index;
    QStyleOptionViewItem option;
    for (int i=first;i<=last;i++) {
        index = m_model->index(i,0,parent);
        delegate->initStyleOption(&option,index);
        section0 = qMax(section0,delegate->sizeHint(option,index).width());
        emit rowAdded(m_model->itemFromIndex(m_model->index(i,0,parent)),m_model->itemFromIndex(m_model->index(i,1,parent)));
    }
    if (section0 > header()->sectionSize(0)) header()->resizeSection(0,section0);
}

void ProgressView::itemChanged(QStandardItem * item) {
    ProgressDelegate * delegate = (ProgressDelegate *)itemDelegate();
    QStyleOptionViewItem option;
    QModelIndex index = m_model->indexFromItem(item);
    delegate->initStyleOption(&option,index);
    int section0 = qMax(header()->sectionSize(0),delegate->sizeHint(option,index).width());
    if (section0 > header()->sectionSize(0)) header()->resizeSection(0,section0);
}

void ProgressView::hideEvent(QHideEvent * event) {
    if (m_clear_on_hide) clear();
    QTreeView::hideEvent(event);
}

void ProgressView::setModel(QAbstractItemModel *) {}
