/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#include "settingstextbrowser.h"
#include <QTextTable>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QShowEvent>
#include <QDesktopServices>
#include <QWheelEvent>
#include <QPoint>
#include <QRadioButton>
#include <QButtonGroup>
#include <QListView>
#include <QToolButton>
#include <QFrame>
#include <QGridLayout>
#include <QAbstractButton>
#include <QPushButton>
#include <QSpacerItem>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QApplication>
#include <QStyledItemDelegate>
#include <QMap>
#include "themeicons.h"

class NoWheelCombo :public QComboBox {
public:
    using QComboBox::QComboBox;

protected:
    void wheelEvent(QWheelEvent *) {}
};

class NoWheelSpin :public QSpinBox {
public:
    using QSpinBox::QSpinBox;

protected:
    void wheelEvent(QWheelEvent *) {}
};

class RepoButtonTextObject : public ButtonTextObject {
public:
    RepoButtonTextObject(QTextEdit *parent,const QIcon & icon,const QString & text,const QString & repo) : ButtonTextObject(parent,icon,text) {
        m_repo = repo;
    }

    QString repo() const {
        return m_repo;
    }
private:

    QString m_repo;
};

class RepoItemDelegate :public QStyledItemDelegate {
public:
    RepoItemDelegate(QAbstractItemModel * model) : QStyledItemDelegate() {
        connect(model,&QAbstractItemModel::rowsRemoved,this,[=](const QModelIndex &, int first, int last) {
            for (int row=first;row<=last;row++) {
                m_editor_active.remove(model->index(row,0,QModelIndex()));
            }
        });
    }

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt,index);
        if (opt.text.isEmpty() && !editor_active(index)) {
            opt.text = QObject::tr("Double click to edit...");
            opt.font.setItalic(true);
        }
        QStyle *style = option.widget?option.widget->style():QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem,&opt,painter,option.widget);
    }
    QWidget * createEditor(QWidget *parent,const QStyleOptionViewItem &option,const QModelIndex &index) const {
        ((RepoItemDelegate *)this)->m_editor_active[index] = true;
        return QStyledItemDelegate::createEditor(parent,option,index);
    }
    void destroyEditor(QWidget *editor,const QModelIndex &index) const {
        ((RepoItemDelegate *)this)->m_editor_active[index] = false;
        QStyledItemDelegate::destroyEditor(editor,index);
    }

private:
    bool editor_active(const QModelIndex &index) const {
        if (m_editor_active.isEmpty()) return false;
        if (!m_editor_active.contains(index)) return false;
        return m_editor_active[index];
    }

    QMap<QModelIndex,bool> m_editor_active;
};

RepoSettingsTextObject::RepoSettingsTextObject(QTextEdit * edit,const Alpm::Repo & repo) : WidgetTextObject(edit) {
    m_repo = repo;
    init();
}

RepoSettingsTextObject::RepoSettingsTextObject(QTextEdit * edit) : WidgetTextObject(edit) {
    init();
}

void RepoSettingsTextObject::init() {
    QVBoxLayout * verticalLayout_2 = new QVBoxLayout();
    QHBoxLayout * horizontalLayout = new QHBoxLayout();
    QLabel * label = new QLabel(tr("Name"));
    horizontalLayout->addWidget(label);
    nameEdit = new QLineEdit();
    horizontalLayout->addWidget(nameEdit);
    verticalLayout_2->addLayout(horizontalLayout);
    QHBoxLayout * horizontalLayout_7 = new QHBoxLayout();
    QLabel * label_8 = new QLabel(tr("Architecture"));
    horizontalLayout_7->addWidget(label_8);
    line = new QFrame();
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(line->sizePolicy().hasHeightForWidth());
    line->setSizePolicy(sizePolicy);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    horizontalLayout_7->addWidget(line);
    verticalLayout_2->addLayout(horizontalLayout_7);
    QHBoxLayout * horizontalLayout_6 = new QHBoxLayout();
    buttonGroup_6 = new QButtonGroup(verticalLayout_2);
    buttonGroup_6->setExclusive(false);
    QCheckBox * checkbox;
    for (QString & arch: Alpm::instance()->arches()) {
        checkbox = new QCheckBox(arch);
        checkbox->setChecked(true);
        buttonGroup_6->addButton(checkbox);
        horizontalLayout_6->addWidget(checkbox);
    }
    verticalLayout_2->addLayout(horizontalLayout_6);
    QHBoxLayout * horizontalLayout_8 = new QHBoxLayout();
    QLabel * label_9 = new QLabel(tr("Servers"));
    horizontalLayout_8->addWidget(label_9);
    line = new QFrame();
    line->setSizePolicy(sizePolicy);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    horizontalLayout_8->addWidget(line);
    verticalLayout_2->addLayout(horizontalLayout_8);
    mirrorRadio = new QRadioButton(tr("Use mirrorlist file as a source of servers"));
    QButtonGroup * buttonGroup = new QButtonGroup(verticalLayout_2);
    buttonGroup->addButton(mirrorRadio,0);
    verticalLayout_2->addWidget(mirrorRadio);
    serversRadio = new QRadioButton(tr("Set the servers directly"));
    buttonGroup->addButton(serversRadio,1);
    verticalLayout_2->addWidget(serversRadio);
    QHBoxLayout * horizontalLayout_2 = new QHBoxLayout();
    serversList = new QListView();
    serversList->setToolTip(tr("<html><head/><body><p>You can use <span style=\" font-weight:600;\">$repo</span> and <span style=\" font-weight:600;\">$arch</span> variables in URLs:</p><p><span style=\" font-weight:600;\">$repo: </span>to use the name of the repo you entered above</p><p><span style=\" font-weight:600;\">$arch: </span>to use the architectures you picked above</p></body></html>"));
    QAbstractItemModel * old_model = serversList->model();
    serversList->setModel(new QStandardItemModel());
    serversList->setItemDelegate(new RepoItemDelegate(serversList->model()));
    if (old_model != NULL) delete old_model;
    serversList->setSelectionMode(QAbstractItemView::SingleSelection);
    serversList->setDragEnabled(false);
    serversList->setAcceptDrops(false);
    serversList->setDropIndicatorShown(false);
    serversList->setViewMode(QListView::ListMode);
    horizontalLayout_2->addWidget(serversList);
    QVBoxLayout * verticalLayout = new QVBoxLayout();
    addButton = new QToolButton();
    addButton->setText("");
    addButton->setIcon(ThemeIcons::get(ThemeIcons::LIST_ADD));
    verticalLayout->addWidget(addButton);
    delButton = new QToolButton();
    delButton->setText("");
    delButton->setIcon(ThemeIcons::get(ThemeIcons::DELETE));
    verticalLayout->addWidget(delButton);
    QSpacerItem * verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout->addItem(verticalSpacer);
    horizontalLayout_2->addLayout(verticalLayout);
    verticalLayout_2->addLayout(horizontalLayout_2);
    QHBoxLayout * horizontalLayout_3 = new QHBoxLayout();
    QLabel * label_2 = new QLabel(tr("Signature level"));
    horizontalLayout_3->addWidget(label_2);
    line = new QFrame();
    line->setSizePolicy(sizePolicy);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    horizontalLayout_3->addWidget(line);
    verticalLayout_2->addLayout(horizontalLayout_3);
    QGridLayout * gridLayout = new QGridLayout();
    packageCheck = new QCheckBox(tr("Package"));
    gridLayout->addWidget(packageCheck, 0, 0, 1, 1);
    neverPackageRadio = new QRadioButton(tr("Never"));
    buttonGroup_2 = new QButtonGroup(verticalLayout_2);
    buttonGroup_3 = new QButtonGroup(verticalLayout_2);
    buttonGroup_4 = new QButtonGroup(verticalLayout_2);
    buttonGroup_5 = new QButtonGroup(verticalLayout_2);
    buttonGroup_2->addButton(neverPackageRadio,0);
    gridLayout->addWidget(neverPackageRadio, 0, 1, 1, 1);
    optionalPackageRadio = new QRadioButton(tr("Optional"));
    buttonGroup_2->addButton(optionalPackageRadio,1);
    gridLayout->addWidget(optionalPackageRadio, 0, 2, 1, 1);
    requiredPackageRadio = new QRadioButton(tr("Required"));
    buttonGroup_2->addButton(requiredPackageRadio,2);
    gridLayout->addWidget(requiredPackageRadio, 0, 3, 1, 1);
    trustedonlyPackageRadio = new QRadioButton(tr("Trusted Only"));
    buttonGroup_3->addButton(trustedonlyPackageRadio,0);
    gridLayout->addWidget(trustedonlyPackageRadio, 0, 4, 1, 1);
    trustallPackageRadio = new QRadioButton(tr("Trust All"));
    buttonGroup_3->addButton(trustallPackageRadio,1);
    gridLayout->addWidget(trustallPackageRadio, 0, 5, 1, 1);
    databaseCheck = new QCheckBox(tr("Database"));
    gridLayout->addWidget(databaseCheck, 1, 0, 1, 1);
    neverDatabaseRadio = new QRadioButton(tr("Never"));
    buttonGroup_4->addButton(neverDatabaseRadio,0);
    gridLayout->addWidget(neverDatabaseRadio, 1, 1, 1, 1);
    optionalDatabaseRadio = new QRadioButton(tr("Optional"));
    buttonGroup_4->addButton(optionalDatabaseRadio,1);
    gridLayout->addWidget(optionalDatabaseRadio, 1, 2, 1, 1);
    requiredDatabaseRadio = new QRadioButton(tr("Required"));
    buttonGroup_4->addButton(requiredDatabaseRadio,2);
    gridLayout->addWidget(requiredDatabaseRadio, 1, 3, 1, 1);
    trustedonlyDatabaseRadio = new QRadioButton(tr("Trusted Only"));
    buttonGroup_5->addButton(trustedonlyDatabaseRadio,0);
    gridLayout->addWidget(trustedonlyDatabaseRadio, 1, 4, 1, 1);
    trustallDatabaseRadio = new QRadioButton(tr("Trust All"));
    buttonGroup_5->addButton(trustallDatabaseRadio,1);
    gridLayout->addWidget(trustallDatabaseRadio, 1, 5, 1, 1);
    verticalLayout_2->addLayout(gridLayout);
    QHBoxLayout * horizontalLayout_4 = new QHBoxLayout();
    QLabel * label_3 = new QLabel(tr("Usage"));
    horizontalLayout_4->addWidget(label_3);
    QFrame * line_2 = new QFrame();
    sizePolicy.setHeightForWidth(line_2->sizePolicy().hasHeightForWidth());
    line_2->setSizePolicy(sizePolicy);
    line_2->setFrameShape(QFrame::HLine);
    line_2->setFrameShadow(QFrame::Sunken);
    horizontalLayout_4->addWidget(line_2);
    verticalLayout_2->addLayout(horizontalLayout_4);
    QHBoxLayout * horizontalLayout_5 = new QHBoxLayout();
    allCheck = new QCheckBox(tr("All"));
    horizontalLayout_5->addWidget(allCheck);
    syncCheck = new QCheckBox(tr("Sync"));
    horizontalLayout_5->addWidget(syncCheck);
    searchCheck = new QCheckBox(tr("Search"));
    horizontalLayout_5->addWidget(searchCheck);
    installCheck = new QCheckBox(tr("Install"));
    horizontalLayout_5->addWidget(installCheck);
    upgradeCheck = new QCheckBox(tr("Upgrade"));
    horizontalLayout_5->addWidget(upgradeCheck);
    verticalLayout_2->addLayout(horizontalLayout_5);
    buttonBox = new QDialogButtonBox();
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    verticalLayout_2->addWidget(buttonBox);
    setLayout(verticalLayout_2);

    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    mirrorRadio->setChecked(true);
    allCheck->setChecked(true);
    optionalPackageRadio->setChecked(true);
    optionalDatabaseRadio->setChecked(true);
    trustedonlyPackageRadio->setChecked(true);
    trustedonlyDatabaseRadio->setChecked(true);
    delButton->setEnabled(false);
    update_states();

    connect(serversList->selectionModel(),&QItemSelectionModel::selectionChanged,this,&RepoSettingsTextObject::selection_repolist_state);
    connect(addButton,&QToolButton::clicked,this,&RepoSettingsTextObject::add_repolist_item);
    connect(delButton,&QToolButton::clicked,this,&RepoSettingsTextObject::delete_repolist_item);
    connect(allCheck,&QCheckBox::clicked,this,&RepoSettingsTextObject::usage_update_states);
    connect(packageCheck,&QCheckBox::clicked,this,&RepoSettingsTextObject::package_siglevel_update_states);
    connect(databaseCheck,&QCheckBox::clicked,this,&RepoSettingsTextObject::database_siglevel_update_states);
    connect(buttonGroup,&QButtonGroup::idClicked,this,&RepoSettingsTextObject::servers_update_states);
    connect(buttonGroup_6,&QButtonGroup::idClicked,this,&RepoSettingsTextObject::ok_button_state);
    connect(nameEdit,&QLineEdit::textChanged,this,&RepoSettingsTextObject::ok_button_state);
    connect((QStandardItemModel *)serversList->model(),&QStandardItemModel::itemChanged,this,&RepoSettingsTextObject::ok_button_state);
    connect((QStandardItemModel *)serversList->model(),&QAbstractItemModel::rowsRemoved,this,&RepoSettingsTextObject::ok_button_state);
    connect(buttonGroup,&QButtonGroup::idClicked,this,&RepoSettingsTextObject::ok_button_state);
    connect(buttonBox,&QDialogButtonBox::accepted,this,[=] { emit result(true); });
    connect(buttonBox,&QDialogButtonBox::rejected,this,[=] { emit result(false); });

    if (m_repo.isValid()) QMetaObject::invokeMethod(this,"apply_repo",Qt::QueuedConnection);
}

void RepoSettingsTextObject::apply_repo() {
    nameEdit->setReadOnly(true);
    nameEdit->setText(m_repo.name());
    serversRadio->setChecked(true);
    serversRadio->setEnabled(true);
    mirrorRadio->setEnabled(true);
    for (QString & server: m_repo.servers()) {
        ((QStandardItemModel *)serversList->model())->appendRow(new QStandardItem(ThemeIcons::get(ThemeIcons::SERVER),server));
    }
    for (Alpm::Repo::SigLevel & level: m_repo.siglevel()) {
        packageCheck->setChecked(level.object() == Alpm::Repo::Both || level.object() == Alpm::Repo::Package);
        databaseCheck->setChecked(level.object() == Alpm::Repo::Both || level.object() == Alpm::Repo::Database);
        neverPackageRadio->setChecked(packageCheck->isChecked() && level.check() == Alpm::Repo::Never);
        neverDatabaseRadio->setChecked(databaseCheck->isChecked() && level.check() == Alpm::Repo::Never);
        optionalPackageRadio->setChecked(packageCheck->isChecked() && level.check() == Alpm::Repo::Optional);
        optionalDatabaseRadio->setChecked(databaseCheck->isChecked() && level.check() == Alpm::Repo::Optional);
        requiredPackageRadio->setChecked(packageCheck->isChecked() && level.check() == Alpm::Repo::Required);
        requiredDatabaseRadio->setChecked(databaseCheck->isChecked() && level.check() == Alpm::Repo::Required);
        trustedonlyPackageRadio->setChecked(packageCheck->isChecked() && level.allowed() == Alpm::Repo::TrustedOnly);
        trustedonlyDatabaseRadio->setChecked(databaseCheck->isChecked() && level.allowed() == Alpm::Repo::TrustedOnly);
        trustallPackageRadio->setChecked(packageCheck->isChecked() && level.allowed() == Alpm::Repo::TrustAll);
        trustallDatabaseRadio->setChecked(databaseCheck->isChecked() && level.allowed() == Alpm::Repo::TrustAll);
    }
    if (m_repo.usage().isAll()) allCheck->setChecked(true);
    else {
        syncCheck->setChecked(m_repo.usage().isSync());
        searchCheck->setChecked(m_repo.usage().isSearch());
        installCheck->setChecked(m_repo.usage().isInstall());
        upgradeCheck->setChecked(m_repo.usage().isUpgrade());
    }
    QStringList arches = Alpm::instance()->arches();
    for (QAbstractButton * & button: buttonGroup_6->buttons()) {
        button->setChecked(arches.contains(button->text()));
    }
    update_states();
}

Alpm::Repo::SigCheck RepoSettingsTextObject::sigcheck(bool package) const {
    switch(package?buttonGroup_2->checkedId():buttonGroup_4->checkedId()) {
    case 0:
        return Alpm::Repo::Never;
    case 1:
        return Alpm::Repo::Optional;
    case 2:
        return Alpm::Repo::Required;
    }
    return Alpm::Repo::Default;
}

Alpm::Repo::SigAllowed RepoSettingsTextObject::sigallowed(bool package) const {
    switch(package?buttonGroup_3->checkedId():buttonGroup_5->checkedId()) {
    case 0:
        return Alpm::Repo::TrustedOnly;
    case 1:
        return Alpm::Repo::TrustAll;
    }
    return Alpm::Repo::Nothing;
}

Alpm::Repo RepoSettingsTextObject::repo(bool & usemirror) const {
    usemirror = mirrorRadio->isChecked();
    QStringList servers;
    QString server;
    if (serversRadio->isChecked()) {
        for (int row=0;row < serversList->model()->rowCount();row++) {
            server = serversList->model()->data(serversList->model()->index(row,0,QModelIndex()),Qt::DisplayRole).toString();
            if (!server.isEmpty()) servers.append(server);
        }
    }
    Alpm::Repo::ListSigLevel siglevels;
    if (packageCheck->isChecked()) siglevels.append(Alpm::Repo::SigLevel(Alpm::Repo::Package,sigcheck(true),sigallowed(true)));
    if (databaseCheck->isChecked()) siglevels.append(Alpm::Repo::SigLevel(Alpm::Repo::Database,sigcheck(false),sigallowed(false)));
    return Alpm::Repo(nameEdit->text(),servers,checked_arches(),siglevels,allCheck->isChecked()?Alpm::Repo::Usage(true,true,true,true):Alpm::Repo::Usage(syncCheck->isChecked(),searchCheck->isChecked(),installCheck->isChecked(),upgradeCheck->isChecked()));
}

QStringList RepoSettingsTextObject::checked_arches() const {
    QStringList ret;
    for (QAbstractButton * & button: buttonGroup_6->buttons()) {
        if (button->isChecked()) ret.append(button->text());
    }
    return ret;
}

bool RepoSettingsTextObject::at_least_one_arch_picked() const {
    for (QAbstractButton * & button: buttonGroup_6->buttons()) {
        if (button->isChecked()) return true;
    }
    return false;
}

void RepoSettingsTextObject::ok_button_state() {
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!nameEdit->text().trimmed().isEmpty() && nameEdit->text().trimmed() != "local" && at_least_one_arch_picked() && (mirrorRadio->isChecked() || is_servers_set()));
}

bool RepoSettingsTextObject::is_servers_set() const {
    for (int row=0;row < serversList->model()->rowCount();row++) {
        if (!serversList->model()->data(serversList->model()->index(row,0,QModelIndex()),Qt::DisplayRole).toString().isEmpty()) return true;
    }

    return false;
}

void RepoSettingsTextObject::add_repolist_item() {
    ((QStandardItemModel *)serversList->model())->appendRow(new QStandardItem(ThemeIcons::get(ThemeIcons::SERVER),""));
}

void RepoSettingsTextObject::delete_repolist_item() {
    serversList->model()->removeRow(serversList->selectionModel()->selectedIndexes()[0].row());
}

void RepoSettingsTextObject::selection_repolist_state() {
    delButton->setEnabled(!serversList->selectionModel()->selectedIndexes().isEmpty());
}

void RepoSettingsTextObject::package_siglevel_update_states() {
    neverPackageRadio->setEnabled(packageCheck->isChecked());
    optionalPackageRadio->setEnabled(packageCheck->isChecked());
    requiredPackageRadio->setEnabled(packageCheck->isChecked());
    trustedonlyPackageRadio->setEnabled(packageCheck->isChecked());
    trustallPackageRadio->setEnabled(packageCheck->isChecked());
}

void RepoSettingsTextObject::database_siglevel_update_states() {
    neverDatabaseRadio->setEnabled(databaseCheck->isChecked());
    optionalDatabaseRadio->setEnabled(databaseCheck->isChecked());
    requiredDatabaseRadio->setEnabled(databaseCheck->isChecked());
    trustedonlyDatabaseRadio->setEnabled(databaseCheck->isChecked());
    trustallDatabaseRadio->setEnabled(databaseCheck->isChecked());
}

void RepoSettingsTextObject::usage_update_states() {
    syncCheck->setEnabled(!allCheck->isChecked());
    searchCheck->setEnabled(!allCheck->isChecked());
    installCheck->setEnabled(!allCheck->isChecked());
    upgradeCheck->setEnabled(!allCheck->isChecked());
}

void RepoSettingsTextObject::servers_update_states() {
    serversList->setEnabled(serversRadio->isChecked());
    delButton->setEnabled(serversRadio->isChecked() && !serversList->selectionModel()->selectedIndexes().isEmpty());
    addButton->setEnabled(serversRadio->isChecked());
}

void RepoSettingsTextObject::update_states() {
    servers_update_states();
    package_siglevel_update_states();
    database_siglevel_update_states();
    usage_update_states();
    selection_repolist_state();
}

LabelLineEditTextObject::LabelLineEditTextObject(QTextEdit * edit,const QString & labelText,const QString & text) : WidgetTextObject(edit) {
    label = new QLabel(labelText);
    lineEdit = new QLineEdit(text);

    QLayout * layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(label);
    layout->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum));
    layout->addWidget(lineEdit);
    setLayout(layout);

    expandToBlockWidth(true);
}

void LabelLineEditTextObject::setEchoMode(QLineEdit::EchoMode mode) {
    lineEdit->setEchoMode(mode);
}

QString LabelLineEditTextObject::lineText() const {
    return lineEdit->text();
}

QString LabelLineEditTextObject::text() const {
    return label->text()+ " " + lineEdit->text();
}

void LabelLineEditTextObject::setLineText(const QString & text) {
    lineEdit->setText(text);
}

bool LabelLineEditTextObject::isEnabled() const {
    return lineEdit->isEnabled();
}

void LabelLineEditTextObject::setEnabled(bool flag) {
    lineEdit->setEnabled(flag);
}

LabelSpinTextObject::LabelSpinTextObject(QTextEdit * edit,const QString & text,int min,int max,int value,const QString & suffix) : WidgetTextObject(edit) {
    init(text,min,max,value,suffix);
}

LabelSpinTextObject::LabelSpinTextObject(QTextEdit * edit,const QString & text,int min,int max,const QString & suffix) : WidgetTextObject(edit) {
    init(text,min,max,min,suffix);
}

void LabelSpinTextObject::init(const QString & text,int min,int max,int value,const QString & suffix) {
    label = new QLabel(text);
    spin = new NoWheelSpin();
    spin->setMinimum(min);
    spin->setMaximum(max);
    spin->setValue(value);
    spin->setSuffix(suffix);

    QLayout * layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(label);
    layout->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum));
    layout->addWidget(spin);
    setLayout(layout);

    expandToBlockWidth(true);
}

QString LabelSpinTextObject::text() const {
    return label->text() + QString(": %1").arg(spin->value());
}

int LabelSpinTextObject::value() const {
    return spin->value();
}

void LabelSpinTextObject::setValue(int value) {
    spin->setValue(value);
}

bool LabelSpinTextObject::isEnabled() const {
    return spin->isEnabled();
}

void LabelSpinTextObject::setEnabled(bool flag) {
    spin->setEnabled(flag);
}

ProxyTypeObject::ProxyTypeObject(QTextEdit * edit,const QString & text) : WidgetTextObject(edit) {
    label = new QLabel(text);
    combo = new NoWheelCombo();
    combo->addItem(tr("No proxy"),QNetworkProxy::NoProxy);
    combo->addItem("HTTP",QNetworkProxy::HttpCachingProxy);
    combo->addItem("CONNECT",QNetworkProxy::HttpProxy);
    combo->addItem("FTP",QNetworkProxy::FtpCachingProxy);

    QLayout * layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(label);
    layout->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum));
    layout->addWidget(combo);
    setLayout(layout);

    expandToBlockWidth(true);
    setLayout(layout);

    connect(combo,SIGNAL(activated(int)),SLOT(proxyActivated(int)));
}

void ProxyTypeObject::setValue(QNetworkProxy::ProxyType type) {
    for (int i=0;i<combo->count();i++) {
        if (combo->itemData(i) == type) {
            combo->setCurrentIndex(i);
            break;
        }
    }
}

QNetworkProxy::ProxyType ProxyTypeObject::value() const {
    return (QNetworkProxy::ProxyType)combo->currentData().toInt();
}

QString ProxyTypeObject::text() const {
    return label->text() + " " + combo->currentText();
}

void ProxyTypeObject::proxyActivated(int index) {
    emit activated(combo->itemData(index).toInt());
}

DBExtObject::DBExtObject(QTextEdit * edit,const QString & text) : WidgetTextObject(edit) {
    label = new QLabel(text);
    combo = new NoWheelCombo();

    comboRefresh();

    QLayout * layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(label);
    layout->addItem(new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum));
    layout->addWidget(combo);
    setLayout(layout);

    expandToBlockWidth(true);
    setLayout(layout);

    connect(combo,SIGNAL(activated(int)),SLOT(extActivated(int)));
}

QString DBExtObject::value() const {
    return combo->currentText();
}

QString DBExtObject::text() const {
    return label->text() + " " + combo->currentText();
}

void DBExtObject::comboRefresh() {
    combo->clear();
    for (QString & ext: Alpm::instance()->dbExtensions()) {
        combo->addItem(ext);
    }

    for (int i=0;i<combo->count();i++) {
        if (combo->itemText(i) == Alpm::instance()->dbExtension()) {
            combo->setCurrentIndex(i);
            break;
        }
    }
}

void DBExtObject::extActivated(int index) {
    emit activated(combo->itemText(index));
}

SettingsTextBrowser::SettingsTextBrowser(QWidget *parent) : CustomPopupTextBrowser(parent) {
    document()->setDocumentMargin(20);
    emit_refresh_at_hide = false;

    qRegisterMetaType<QTextTable*>("QTextTable*");

    insertText(textCursor(),tr("The following user variables are used")+":",QFont::Bold);
    QTextTable * table = insertTable(textCursor(),10,1,1,Qt::AlignLeft,true);
    insertText(table->cellAt(0,0).firstCursorPosition(),tr("Variables"),QFont::Bold);
    useSysIconsBox = new CheckBoxTextObject(this,tr("Use the system icons"));
    useSysIconsBox->insert(table->cellAt(1,0).firstCursorPosition());
    extObj = new DBExtObject(this,tr("Database files' suffix"));
    extObj->insert(table->cellAt(2,0).firstCursorPosition());
    threadsObj = new LabelSpinTextObject(this,tr("Count of the threads for each download"),1,20);
    threadsObj->insert(table->cellAt(3,0).firstCursorPosition());
    timeoutObj = new LabelSpinTextObject(this,tr("Connection timeout"),0,300,tr(" secs"));
    timeoutObj->insert(table->cellAt(4,0).firstCursorPosition());
    proxyType = new ProxyTypeObject(this,tr("Proxy type"));
    connect(proxyType,SIGNAL(activated(int)),SLOT(proxyActivated(int)));
    proxyType->insert(table->cellAt(5,0).firstCursorPosition());
    proxyAddress = new LabelLineEditTextObject(this,tr("Proxy address"));
    proxyAddress->insert(table->cellAt(6,0).firstCursorPosition());
    proxyPort = new LabelSpinTextObject(this,tr("Proxy port"),1,65535);
    proxyPort->insert(table->cellAt(7,0).firstCursorPosition());
    proxyUser = new LabelLineEditTextObject(this,tr("Proxy user"));
    proxyUser->insert(table->cellAt(8,0).firstCursorPosition());
    proxyPassword = new LabelLineEditTextObject(this,tr("Proxy password"));
    proxyPassword->setEchoMode(QLineEdit::Password);
    proxyPassword->insert(table->cellAt(9,0).firstCursorPosition());
    proxyActivated(proxyType->value());
    setupTableHeader(table,palette().color(QPalette::Active,QPalette::Mid),palette().buttonText().color());
    setTableRowColors(table,palette().button().color(),viewport()->palette().color(QPalette::Active,QPalette::Base));

    moveCursor(QTextCursor::End);
    textCursor().insertBlock();
    textCursor().insertBlock();
    moveCursor(QTextCursor::End);
    repoStartPos = textCursor().position();

    moveCursor(QTextCursor::Start);
    ensureCursorVisible();
}

void SettingsTextBrowser::showEvent(QShowEvent *event) {
    CustomPopupTextBrowser::showEvent(event);
    useSysIconsBox->setChecked(Alpm::instance()->useSystemIcons());
    extObj->comboRefresh();
    threadsObj->setValue(Alpm::instance()->downloaderThreadCount());
    timeoutObj->setValue(Alpm::instance()->downloaderTimeout()/1000);

    QNetworkProxy proxy = Alpm::instance()->downloaderProxy();
    proxyType->setValue(proxy.type());
    proxyAddress->setLineText(proxy.hostName());
    proxyPort->setValue(proxy.port());
    proxyUser->setLineText(proxy.user());
    proxyPassword->setLineText(proxy.password());

    recreate_repo_table();
}

void SettingsTextBrowser::recreate_repo_table() {
    QTextCursor cursor(document());
    cursor.setPosition(repoStartPos);
    cursor.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.setPosition(repoStartPos);
    setTextCursor(cursor);
    insertText(textCursor(),tr("The following repositories Pacman has set up")+":",QFont::Bold);
    QTextTable * repoTable = insertTable(textCursor(),1,2,1,Qt::AlignLeft,true);
    insertText(repoTable->cellAt(0,0).firstCursorPosition(),tr("Name"),QFont::Bold);
    insertText(repoTable->cellAt(0,1).firstCursorPosition(),tr("Operations"),QFont::Bold);
    setupTableHeader(repoTable,palette().color(QPalette::Active,QPalette::Mid),palette().buttonText().color());
    ButtonTextObject * button;
    RepoButtonTextObject * repoButton;
    (button = new ButtonTextObject(this,ThemeIcons::get(ThemeIcons::LIST_ADD),tr("Add New Repository")))->insert(textCursor());
    connect(button,&ButtonTextObject::clicked,this,&SettingsTextBrowser::addnew_button_clicked);
    if (repoTable->rows() > 1) repoTable->removeRows(1,repoTable->rows()-1);
    for (Alpm::Repo & repo: Alpm::instance()->repos()) {
        repoTable->appendRows(1);
        (new SimpleLabelTextObject(this,repo.name(),ThemeIcons::get(ThemeIcons::REPO)))->insert(repoTable->cellAt(repoTable->rows()-1,0).firstCursorPosition());
        (repoButton = new RepoButtonTextObject(this,ThemeIcons::get(ThemeIcons::CHANGES_DLG),tr("Change"),repo.name()))->insert(repoTable->cellAt(repoTable->rows()-1,1).firstCursorPosition());
        connect(repoButton,&ButtonTextObject::clicked,this,[=]{ change_button_clicked(repoButton->repo()); });
        (repoButton = new RepoButtonTextObject(this,ThemeIcons::get(ThemeIcons::DELETE),tr("Remove"),repo.name()))->insert(repoTable->cellAt(repoTable->rows()-1,1).lastCursorPosition());
        connect(repoButton,&ButtonTextObject::clicked,this,[=]{ remove_button_clicked(repoButton->repo()); });
    }
    setTableRowColors(repoTable,palette().button().color(),viewport()->palette().color(QPalette::Active,QPalette::Base));
    QMetaObject::invokeMethod(this,"resize_addnew_button",Qt::QueuedConnection,Q_ARG(QTextTable*,repoTable),Q_ARG(ButtonTextObject*,button));
}

void SettingsTextBrowser::resize_addnew_button(QTextTable * table,ButtonTextObject * button) {
    button->resize(document()->documentLayout()->frameBoundingRect(table).width(),button->height());
}

void SettingsTextBrowser::change_button_clicked(const QString & repo) {
    QTextCursor cursor(document());
    cursor.setPosition(repoStartPos);
    setTextCursor(cursor);
    moveCursor(QTextCursor::End,QTextCursor::KeepAnchor);
    insertText(textCursor(),tr("Press OK to change the repository")+"...",QFont::Bold);
    textCursor().insertBlock();
    RepoSettingsTextObject * repo_settings = new RepoSettingsTextObject(this,this->repo(repo));
    connect(repo_settings,&RepoSettingsTextObject::result,this,&SettingsTextBrowser::changeSettingsResult);
    repo_settings->insert(textCursor());
}

void SettingsTextBrowser::remove_button_clicked(const QString & repo) {
    bool ret = Alpm::instance()->deleteRepo(repo);
    recreate_repo_table();
    if (!ret) {
        moveCursor(QTextCursor::End);
        textCursor().insertBlock();
        insertText(textCursor(),QString(tr("The repo %1 hasn't been removed!")).arg(repo)+":",QColor::fromRgb(255,0,0));
    }
    emit_refresh_at_hide = true;
}

void SettingsTextBrowser::addnew_button_clicked() {
    QTextCursor cursor(document());
    cursor.setPosition(repoStartPos);
    setTextCursor(cursor);
    moveCursor(QTextCursor::End,QTextCursor::KeepAnchor);
    insertText(textCursor(),tr("Fill the fields below and press OK to add the new repository")+"...",QFont::Bold);
    textCursor().insertBlock();
    RepoSettingsTextObject * repo_settings = new RepoSettingsTextObject(this);
    connect(repo_settings,&RepoSettingsTextObject::result,this,&SettingsTextBrowser::addNewSettingsResult);
    repo_settings->insert(textCursor());
}

void SettingsTextBrowser::proxyActivated(int index) {
    bool enabled = (index != QNetworkProxy::NoProxy);
    proxyAddress->setEnabled(enabled);
    proxyPort->setEnabled(enabled);
    proxyUser->setEnabled(enabled);
    proxyPassword->setEnabled(enabled);
}

void SettingsTextBrowser::hideEvent(QHideEvent *event) {
    QNetworkProxy proxy;
    proxy.setType(proxyType->value());
    proxy.setHostName(proxyAddress->lineText());
    proxy.setPort(proxyPort->value());
    proxy.setUser(proxyUser->lineText());
    proxy.setPassword(proxyPassword->lineText());
    Alpm::instance()->setDownloaderProxy(proxy);
    Alpm::instance()->setDownloaderTimeout(timeoutObj->value()*1000);
    Alpm::instance()->setDownloaderThreads(threadsObj->value());
    Alpm::instance()->setUsingSystemIcons(useSysIconsBox->isChecked());
    Alpm::instance()->setDBExtension(extObj->value());
    if (emit_refresh_at_hide) {
        emit_refresh_at_hide = false;
        emit dbsRefreshNeeded();
    }

    CustomPopupTextBrowser::hideEvent(event);
}

Alpm::Repo SettingsTextBrowser::repo(const QString & name) const {
    for (Alpm::Repo & repo: Alpm::instance()->repos()) {
        if (repo.name() == name) return repo;
    }
    return Alpm::Repo();
}

void SettingsTextBrowser::changeSettingsResult(bool result) {
    bool ret = true;
    if (result) {
        RepoSettingsTextObject * repo_settings = (RepoSettingsTextObject *)QObject::sender();
        bool usemirror;
        Alpm::Repo repo = repo_settings->repo(usemirror);
        ret = Alpm::instance()->deleteRepo(repo.name());
        if (ret) {
            if (usemirror) ret = Alpm::instance()->addMirrorRepo(repo);
            else ret = Alpm::instance()->addNewRepo(repo);
        }
        emit_refresh_at_hide = true;
    }
    recreate_repo_table();
    if (!ret) {
        moveCursor(QTextCursor::End);
        textCursor().insertBlock();
        insertText(textCursor(),tr("The repo hasn't been changed!")+":",QColor::fromRgb(255,0,0));
    }
}

void SettingsTextBrowser::addNewSettingsResult(bool result) {
    bool ret = true;
    if (result) {
        RepoSettingsTextObject * repo_settings = (RepoSettingsTextObject *)QObject::sender();
        bool usemirror;
        Alpm::Repo repo = repo_settings->repo(usemirror);
        if (usemirror) ret = Alpm::instance()->addMirrorRepo(repo);
        else ret = Alpm::instance()->addNewRepo(repo);
        emit_refresh_at_hide = true;
    }
    recreate_repo_table();
    if (!ret) {
        moveCursor(QTextCursor::End);
        textCursor().insertBlock();
        insertText(textCursor(),tr("The new repo hasn't been created!")+":",QColor::fromRgb(255,0,0));
    }
}
