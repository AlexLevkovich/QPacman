/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "optionsmenu.h"
#include <QDockWidget>
#include <QWidgetAction>
#include <QApplication>
#include <QVariant>
#include <QIcon>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QRadioButton>
#include <QSpacerItem>
#include <QToolButton>
#include <QDesktopWidget>
#include "optionswidget.h"
#include "themeicons.h"
#include "packageview.h"
#include "static.h"
#include <QDebug>

OptionsMenu::OptionsMenu(AlpmPackage::UserChangeStatus status,QWidget *parent) : QMenu(parent) {
    m_status = status;
    setFocusPolicy(Qt::StrongFocus);
    qApp->installEventFilter(this);
    m_init_ok = false;
    m_show_event_ok = false;
    QMetaObject::invokeMethod(this,"init",Qt::QueuedConnection);
}

bool OptionsMenu::hasTitleOkButton() const {
    return true;
}

bool OptionsMenu::hasTitleCancelButton() const {
    return true;
}

void OptionsMenu::show_event() {
    QList<OkCancelTitleBar *> titleBars = this->titleBars();
    for (int i=0;i<widgets().count();i++) {
        if (haveTitleRadios()) m_child_widgets.append(titleBars[i]->radioButton);
        QList<QWidget *> childs = widgets()[i]->findChildren<QWidget *>(QString(),Qt::FindChildrenRecursively);
        for (int j=0;j<childs.count();j++) {
            if (qobject_cast<QLabel *>(childs[j])) continue;
            m_child_widgets.append(childs[j]);
        }
    }
    if (hasTitleCancelButton() && titleBars.count() > 0) m_child_widgets.append(titleBars[0]->cancelButton);
    if (hasTitleOkButton() && titleBars.count() > 0) m_child_widgets.append(titleBars[0]->okButton);
    if (m_child_widgets.count() > 0) m_child_widgets[0]->setFocus();

    QRect screenRect = QApplication::desktop()->screenGeometry(this);
    QRect menuRect = geometry();
    if (screenRect.contains(menuRect)) return;
    if (!screenRect.contains(menuRect.topLeft())) move(0,0);
    else {
        int x = menuRect.x();
        int y = menuRect.y();
        QRect intersection = screenRect & menuRect;
        if (intersection.width() < menuRect.width()) {
            if (x < screenRect.x()) x += menuRect.width() - intersection.width();
            else x -= menuRect.width() - intersection.width();
        }
        if (intersection.height() < menuRect.height()) {
            if (y < screenRect.y()) y += menuRect.height() - intersection.height();
            else y -= menuRect.height() - intersection.height();
        }
        move(x,y);
    }
}

bool OptionsMenu::eventFilter(QObject *obj, QEvent *e) {
    QWidget * widget = qobject_cast<QWidget *>(obj);
    int index = -1;
    if ((widget == this) && (e->type() == QEvent::Show)) {
        m_show_event_ok = true;
        if (m_show_event_ok && m_init_ok) QMetaObject::invokeMethod(this,"show_event",Qt::QueuedConnection);
        return QMenu::eventFilter(obj,e);
    }
    if ((e->type() != QEvent::KeyPress && e->type() != QEvent::KeyRelease) || widget != this) return QMenu::eventFilter(obj,e);

    QKeyEvent * event = (QKeyEvent *)e;
    int i = -1;
    widget = NULL;
    for (i=0;i<m_child_widgets.count();i++) {
        if (m_child_widgets[i]->hasFocus()) {
            widget = m_child_widgets[i];
            index = i;
            break;
        }
    }

    if ((widget != NULL) && (e->type() == QEvent::KeyRelease)) {
        QCoreApplication::postEvent(widget,new QKeyEvent(event->type(),event->key(),event->modifiers(),event->nativeScanCode(),event->nativeVirtualKey(),event->nativeModifiers(),event->text()));
        return true;
    }
    else if ((widget != NULL) && (e->type() == QEvent::KeyPress)) {
        bool is_edit_widget = widget->inputMethodQuery(Qt::ImCursorPosition).isValid();
        if ((event->key() == Qt::Key_Tab) && (is_edit_widget?(event->modifiers() & Qt::ShiftModifier):1)) {
            if ((index+1) >= m_child_widgets.count()) index = -1;
            int i = -1;
            for (i=(index+1);i<m_child_widgets.count();i++) {
                if(m_child_widgets[i]->isEnabled()) break;
            }
            if (i == m_child_widgets.count()) {
                for (i=0;i<=index;i++) {
                    if(m_child_widgets[i]->isEnabled()) break;
                }
                if (i == (index+1)) i = -1;
            }
            if (i >= 0) m_child_widgets[i]->setFocus();
        }
        else if (event->key() == Qt::Key_Escape) {
            close();
        }
        else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
            okClicked();
        }
        else {
            QCoreApplication::postEvent(widget,new QKeyEvent(event->type(),event->key(),event->modifiers(),event->nativeScanCode(),event->nativeVirtualKey(),event->nativeModifiers(),event->text()));
            return true;
        }
    }
    return QMenu::eventFilter(obj,e);
}

void OptionsMenu::init() {
    checked_index = 0;
    int count = widgets().count();
    int i;
    for (i=0;i<count;i++) {
        if (widgets()[i]->setStatus(m_status)) checked_index = i;
    }
    OkCancelTitleBar * titlebar;
    for (i=0;i<count;i++) {
        QWidgetAction * action = new QWidgetAction(this);
        QDockWidget * widget = new QDockWidget(this);

        titlebar = new OkCancelTitleBar(i,widget);
        titlebar->setRadioButtonVisible(haveTitleRadios());
        titlebar->setChecked(i == checked_index);
        titlebar->setText(titleTexts()[i]);
        titlebar->setOkButtonVisible((i == 0) && hasTitleOkButton());
        titlebar->setCancelButtonVisible((i == 0) && hasTitleCancelButton());
        titlebar->setIcon(this->widgets()[i]->currentStatusIcon());
        widget->setTitleBarWidget(titlebar);
        widget->setWidget(this->widgets()[i]);
        widget->setFeatures(QDockWidget::NoDockWidgetFeatures);
        widgets()[i]->setEnabled(i == checked_index);
        action->setDefaultWidget(widget);
        addAction(action);

        if (i == 0) {
            connect(titlebar,SIGNAL(okClicked()),SLOT(okClicked()));
            connect(titlebar,SIGNAL(cancelClicked()),SLOT(close()));
        }
        connect(titlebar,SIGNAL(checkStateChanged(bool)),SLOT(titleCheckStateChanged(bool)));
    }
    m_init_ok = true;
    if (m_show_event_ok && m_init_ok) QMetaObject::invokeMethod(this,"show_event",Qt::QueuedConnection);
}

QList<OkCancelTitleBar *> OptionsMenu::titleBars() const {
    QList<OkCancelTitleBar *> ret;
    QList<QAction * > actions = this->actions();
    for (int i=0;i<actions.count();i++) {
        if (!actions[i]->inherits("QWidgetAction")) continue;
        if (!((QWidgetAction *)actions[i])->defaultWidget()->inherits("QDockWidget")) continue;
        QDockWidget * widget = (QDockWidget *)((QWidgetAction *)actions[i])->defaultWidget();
        if (!widget->titleBarWidget()->inherits("OkCancelTitleBar")) continue;
        ret.append((OkCancelTitleBar *)widget->titleBarWidget());
    }
    return ret;
}

void OptionsMenu::titleCheckStateChanged(bool state) {
    OkCancelTitleBar * titlebar = (OkCancelTitleBar *)QObject::sender();
    if (!state) {
        titlebar->setChecked(!state);
        return;
    }

    checked_index = titlebar->index();
    QList<OkCancelTitleBar *> titlebars = titleBars();
    for (int i=0;i<titlebars.count();i++) {
        if (titlebars[i] == titlebar) {
            widgets()[i]->setEnabled(true);
            continue;
        }
        titlebars[i]->setChecked(!state);
        widgets()[i]->setEnabled(false);
    }
}

void OptionsMenu::okClicked() {
    QAction * action = new QAction(this);
    action->setText(widgets()[checked_index]->currentStatusText());
    action->setToolTip(widgets()[checked_index]->currentStatusTextHint());
    action->setIcon(widgets()[checked_index]->currentStatusIcon());
    action->setData((int)widgets()[checked_index]->currentStatus());
    emit triggered(action);
    close();
}

InstallOptionsMenu::InstallOptionsMenu(AlpmPackage::UserChangeStatus status,QWidget *parent) : OptionsMenu(status,parent) {
    m_widget = new InstallOptionsWidget(this);
}

QStringList InstallOptionsMenu::titleTexts() const {
    return QStringList() << PackageItemModel::status_text(AlpmPackage::DO_INSTALL);
}

bool InstallOptionsMenu::haveTitleRadios() const {
    return false;
}

QList<OptionsWidget *> InstallOptionsMenu::widgets() const {
    return QList<OptionsWidget *>() << m_widget;
}

ReinstallOptionsMenu::ReinstallOptionsMenu(AlpmPackage::UserChangeStatus status,QWidget *parent) : OptionsMenu(status,parent) {
    m_rewidget = new ReinstallOptionsWidget(this);
    m_unwidget = new UninstallOptionsWidget(this);
}

QStringList ReinstallOptionsMenu::titleTexts() const {
    return QStringList() << PackageItemModel::status_text(AlpmPackage::DO_UNINSTALL) << PackageItemModel::status_text(AlpmPackage::DO_REINSTALL);
}

bool ReinstallOptionsMenu::haveTitleRadios() const {
    return true;
}

QList<OptionsWidget *> ReinstallOptionsMenu::widgets() const {
    return QList<OptionsWidget *>() << m_unwidget << m_rewidget;
}

UninstallOptionsMenu::UninstallOptionsMenu(AlpmPackage::UserChangeStatus status,QWidget *parent) : OptionsMenu(status,parent) {
    m_widget = new UninstallOptionsWidget(this);
}

QStringList UninstallOptionsMenu::titleTexts() const {
    return QStringList() << PackageItemModel::status_text(AlpmPackage::DO_UNINSTALL);
}

bool UninstallOptionsMenu::haveTitleRadios() const {
    return false;
}

QList<OptionsWidget *> UninstallOptionsMenu::widgets() const {
    return QList<OptionsWidget *>() << m_widget;
}

OkCancelTitleBar::OkCancelTitleBar(int index,QWidget *parent) : QWidget(parent) {
    horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    iconLabel = new QLabel(this);
    horizontalLayout->addWidget(iconLabel);
    horizontalSpacer_2 = new QSpacerItem(4, 2, QSizePolicy::Fixed, QSizePolicy::Minimum);
    horizontalLayout->addItem(horizontalSpacer_2);
    radioButton = new QRadioButton(this);
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(radioButton->sizePolicy().hasHeightForWidth());
    radioButton->setSizePolicy(sizePolicy);
    radioButton->setCheckable(true);
    horizontalLayout->addWidget(radioButton);
    windowText = new QLabel(this);
    horizontalLayout->addWidget(windowText);
    horizontalSpacer = new QSpacerItem(39, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalLayout->addItem(horizontalSpacer);
    cancelButton = new QToolButton(this);
    cancelButton->setIcon(ThemeIcons::get(ThemeIcons::CANCEL));
    cancelButton->setAutoRaise(true);
    horizontalLayout->addWidget(cancelButton);
    okButton = new QToolButton(this);
    okButton->setAutoRaise(true);
    okButton->setIcon(ThemeIcons::get(ThemeIcons::OK));
    horizontalLayout->addWidget(okButton);

    iconLabel->setText(QString());
    radioButton->setText(QString());
    windowText->setText(QString());
    cancelButton->setText(QString());
    okButton->setText(QString());

    m_index = index;
    setRadioButtonVisible(false);
    m_radio_visible = false;
    setText("");

    qApp->installEventFilter(this);

    connect(radioButton,SIGNAL(clicked(bool)),SIGNAL(checkStateChanged(bool)));
    connect(okButton,SIGNAL(clicked()),SIGNAL(okClicked()));
    connect(cancelButton,SIGNAL(clicked()),SIGNAL(cancelClicked()));
}

bool OkCancelTitleBar::isRadioButtonVisible() const {
    return m_radio_visible;
}

void OkCancelTitleBar::setRadioButtonVisible(bool visible) {
    radioButton->setVisible(visible);
    m_radio_visible = visible;
}

void OkCancelTitleBar::setText(const QString & text) {
    if (isRadioButtonVisible()) {
        radioButton->setText(text);
        windowText->setText("");
    }
    else {
        windowText->setText(text);
        radioButton->setText("");
    }
}

QString OkCancelTitleBar::text() const {
    return isRadioButtonVisible()?radioButton->text():windowText->text();
}

bool OkCancelTitleBar::isOkButtonVisible() const {
    return okButton->isVisible();
}

void OkCancelTitleBar::setOkButtonVisible(bool visible) {
    okButton->setVisible(visible);
}

bool OkCancelTitleBar::isCancelButtonVisible() const {
    return cancelButton->isVisible();
}

void OkCancelTitleBar::setCancelButtonVisible(bool visible) {
    cancelButton->setVisible(visible);
}

bool OkCancelTitleBar::isChecked() const {
    return radioButton->isChecked();
}

void OkCancelTitleBar::setChecked(bool checked) {
    if (isRadioButtonVisible()) radioButton->setChecked(checked);
}

void OkCancelTitleBar::setIcon(const QIcon & icon) {
    iconLabel->setPixmap(icon.pixmap(quadroSize(fontMetrics().height()+4)));
}

bool OkCancelTitleBar::eventFilter(QObject *obj, QEvent *e) {
    if (obj == this && (e->type() == QEvent::MouseButtonRelease || e->type() == QEvent::MouseButtonPress)) return true;
    return QWidget::eventFilter(obj,e);
}
