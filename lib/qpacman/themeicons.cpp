/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#include "themeicons.h"
#include "libalpm.h"

const QIcon ThemeIcons::get(ThemeIcons::Icon id) {
    if (!Alpm::instance()->useSystemIcons()) return QIcon(name(id));

    QIcon icon;
    switch (id) {
        case SERVER:
            icon = QIcon::fromTheme("network-server");
            break;
        case LIST_ADD:
            icon = QIcon::fromTheme("list-add");
            break;
        case CHANGES_DLG:
            icon = QIcon::fromTheme("document-edit");
            break;
        case DOWNLOAD_AVERAGE:
            icon = QIcon::fromTheme("globe");
            break;
        case FIND:
            icon = QIcon::fromTheme("edit-find");
            break;
        case IMPORTANT:
            icon = QIcon::fromTheme("help-about");
            break;
        case LOG_VIEW:
            icon = QIcon::fromTheme("dialog-messages");
            break;
        case PKG_NONINSTALLED_MARK:
            icon = QIcon::fromTheme("package-available");
            break;
        case DOWNLOAD_ITEM:
            icon = QIcon::fromTheme("edit-download");
            break;
        case PKG_INSTALLED:
            icon = QIcon::fromTheme("archive-insert");
            break;
        case PKG_REMOVED:
            icon = QIcon::fromTheme("archive-remove");
            break;
        case PKG:
            icon = QIcon::fromTheme("package-x-generic");
            break;
        case ERROR_ITEM:
            icon = QIcon::fromTheme("dialog-error");
            break;
        case HOOK_AVERAGE_ITEM:
            icon = QIcon::fromTheme("run-build-configure");
            break;
        case INSTALL_AVERAGE_ITEM:
            icon = QIcon::fromTheme("run-build-install");
            break;
        case REMOVE_AVERAGE_ITEM:
            icon = QIcon::fromTheme("run-build-prune");
            break;
        case HOOK_ITEM:
        case INSTALL:
            icon = QIcon::fromTheme("run-build");
            break;
        case ROOT_DLG:
            icon = QIcon::fromTheme("security-low");
            break;
        case PROVIDER_DLG:
            icon = QIcon::fromTheme("edit-select");
            break;
        case PKG_REINSTALL_MARK:
            icon = QIcon::fromTheme("package-reinstall");
            break;
        case QUIT:
            icon = QIcon::fromTheme("application-exit");
            break;
        case CONFIGURE:
            icon = QIcon::fromTheme("configure");
            break;
        case PKG_REMOVE_MARK:
            icon = QIcon::fromTheme("package-remove");
            break;
        case CANCEL:
            icon = QIcon::fromTheme("dialog-cancel");
            break;
        case OK:
            icon = QIcon::fromTheme("dialog-ok");
            break;
        case PKG_INSTALL_MARK:
            icon = QIcon::fromTheme("package-install");
            break;
        case PKG_INSTALLED_MARK:
            icon = QIcon::fromTheme("package-installed-updated");
            break;
        case COPY:
            icon = QIcon::fromTheme("edit-copy");
            break;
        case SELECT_ALL:
            icon = QIcon::fromTheme("edit-select-all");
            break;
        case PKG_SELECT_ALL:
            icon = QIcon::fromTheme("show-all-effects");
            break;
        case UNDO_OR_RESET:
            icon = QIcon::fromTheme("edit-undo");
            break;
        case ALL_PKGS:
            icon = QIcon::fromTheme("package-x-generic");
            break;
        case SYNC:
            icon = QIcon::fromTheme("state-download");
            break;
        case NEXT:
            icon = QIcon::fromTheme("go-next");
            break;
        case PREV:
            icon = QIcon::fromTheme("go-previous");
            break;
        case HELP:
            icon = QIcon::fromTheme("help-about");
            break;
        case DELETE:
        case CLEAN_CACHE:
             icon = QIcon::fromTheme("edit-delete");
            break;
        case PKG_GROUP:
            icon = QIcon::fromTheme("folder");
            break;
        case UPDATE_ITEM:
            icon = QIcon::fromTheme("package-new");
            break;
        case UPDATE_REPOS:
            icon = QIcon::fromTheme("state-sync");
            break;
        case REPO:
            icon = QIcon::fromTheme("repository");
            break;
        case FILTER:
            icon = QIcon::fromTheme("view-filter");
            break;
        case REFRESH:
            icon = QIcon::fromTheme("view-refresh");
            break;
        case DLG_PASSWORD:
            icon = QIcon::fromTheme("dialog-password");
            break;
        case LOCKED:
            icon = QIcon::fromTheme("emblem-locked");
            break;
        default:
            break;
    }

    return (icon.isNull())?QIcon(ThemeIcons::name(id)):icon;
}

const QString ThemeIcons::name(ThemeIcons::Icon id) {
    switch (id) {
        case SERVER:
            return QString::fromLatin1("://pics/network-server.svg");
        case LIST_ADD:
            return QString::fromLatin1("://pics/list-add.svg");
        case CHANGES_DLG:
            return QString::fromLatin1("://pics/document-edit.svg");
        case DOWNLOAD_AVERAGE:
            return QString::fromLatin1("://pics/globe.svg");
        case FIND:
            return QString::fromLatin1("://pics/edit-find.svg");
        case IMPORTANT:
            return QString::fromLatin1("://pics/help-about.svg");
        case LOG_VIEW:
            return QString::fromLatin1("://pics/dialog-messages.svg");
        case PKG_NONINSTALLED_MARK:
            return QString::fromLatin1("://pics/package-available.svg");
        case DOWNLOAD_ITEM:
            return QString::fromLatin1("://pics/edit-download.svg");
        case PKG_INSTALLED:
            return QString::fromLatin1("://pics/archive-insert.svg");
        case PKG_REMOVED:
            return QString::fromLatin1("://pics/archive-remove.svg");
        case PKG:
            return QString::fromLatin1("://pics/package-x-generic.svg");
        case ERROR_ITEM:
            return QString::fromLatin1("://pics/dialog-error.svg");
        case HOOK_AVERAGE_ITEM:
            return QString::fromLatin1("://pics/run-build-configure.svg");
        case INSTALL_AVERAGE_ITEM:
            return QString::fromLatin1("://pics/run-build-install.svg");
        case REMOVE_AVERAGE_ITEM:
            return QString::fromLatin1("://pics/run-build-prune.svg");
        case HOOK_ITEM:
        case INSTALL:
            return QString::fromLatin1("://pics/run-build.svg");
        case ROOT_DLG:
            return QString::fromLatin1("://pics/security-low.svg");
        case PROVIDER_DLG:
            return QString::fromLatin1("://pics/edit-select.svg");
        case PKG_REINSTALL_MARK:
            return QString::fromLatin1("://pics/package-reinstall.svg");
        case QUIT:
            return QString::fromLatin1("://pics/application-exit.svg");
        case CONFIGURE:
            return QString::fromLatin1("://pics/configure.svg");
        case PKG_REMOVE_MARK:
            return QString::fromLatin1("://pics/package-remove.svg");
        case CANCEL:
            return QString::fromLatin1("://pics/dialog-cancel.svg");
        case OK:
            return QString::fromLatin1("://pics/dialog-ok.svg");
        case PKG_INSTALL_MARK:
            return QString::fromLatin1("://pics/package-install.svg");
        case PKG_INSTALLED_MARK:
            return QString::fromLatin1("://pics/package-installed-updated.svg");
        case COPY:
            return QString::fromLatin1("://pics/edit-copy.svg");
        case SELECT_ALL:
            return QString::fromLatin1("://pics/edit-select-all.svg");
        case PKG_SELECT_ALL:
            return QString::fromLatin1("://pics/show-all-effects.svg");
        case UNDO_OR_RESET:
            return QString::fromLatin1("://pics/edit-undo.svg");
        case ALL_PKGS:
            return QString::fromLatin1("://pics/package-x-generic.svg");
        case SYNC:
            return QString::fromLatin1("://pics/state-download.svg");
        case NEXT:
            return QString::fromLatin1("://pics/go-next.svg");
        case PREV:
            return QString::fromLatin1("://pics/go-previous.svg");
        case HELP:
            return QString::fromLatin1("://pics/help-about.svg");
        case DELETE:
        case CLEAN_CACHE:
            return QString::fromLatin1("://pics/edit-delete.svg");
        case PKG_GROUP:
            return QString::fromLatin1("://pics/folder.svg");
        case UPDATE_ITEM:
            return QString::fromLatin1("://pics/package-new.svg");
        case UPDATE_REPOS:
            return QString::fromLatin1("://pics/state-sync.svg");
        case REPO:
            return QString::fromLatin1("://pics/repository.svg");
        case FILTER:
            return QString::fromLatin1("://pics/view-filter.svg");
        case REFRESH:
            return QString::fromLatin1("://pics/view-refresh.svg");
        case WARNING:
            return QString::fromLatin1("://pics/warning.svg");
        case QPACMANTRAY:
            return QString::fromLatin1("://pics/qpacmantray.svg");
        case QPACMAN:
            return QString::fromLatin1("://pics/qpacman.svg");
        case ARCHLINUX:
            return QString::fromLatin1("://pics/Archlinux-icon-crystal-64.svg");
        case CHECKING_MOVIE:
            return QString::fromLatin1("://pics/checking.svg");
        case WAITING_MOVIE:
            return QString::fromLatin1("://pics/waiting.svg");
        case DLG_PASSWORD:
            return QString::fromLatin1("://pics/dialog-password.svg");
        case NO_ICON:
            return QString::fromLatin1("://pics/noicon.png");
        case REFRESH_BTN:
            return QString::fromLatin1("://pics/refresh-button.png");
        case REFRESH_BTN_SHADOW:
            return QString::fromLatin1("://pics/refresh-button_shadow.png");
        case LOCKED:
            return QString::fromLatin1("://pics/emblem-locked.svg");
        default:
            break;
    }

    return QString();
}
