/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2020
** License:    GPL
********************************************************************************/

#ifndef THEMEICONS_H
#define THEMEICONS_H

#include <QIcon>

class ThemeIcons {
public:
    enum Icon {
        CHANGES_DLG,
        DOWNLOAD_AVERAGE,
        FIND,
        IMPORTANT,
        LOG_VIEW,
        PKG_NONINSTALLED_MARK,
        DOWNLOAD_ITEM,
        PKG_INSTALLED,
        PKG_REMOVED,
        PKG,
        ERROR_ITEM,
        HOOK_AVERAGE_ITEM,
        INSTALL_AVERAGE_ITEM,
        REMOVE_AVERAGE_ITEM,
        HOOK_ITEM,
        ROOT_DLG,
        PROVIDER_DLG,
        PKG_REINSTALL_MARK,
        QUIT,
        CONFIGURE,
        PKG_REMOVE_MARK,
        CANCEL,
        OK,
        PKG_INSTALL_MARK,
        PKG_INSTALLED_MARK,
        COPY,
        SELECT_ALL,
        UNDO_OR_RESET,
        ALL_PKGS,
        SYNC,
        NEXT,
        PREV,
        HELP,
        CLEAN_CACHE,
        PKG_GROUP,
        UPDATE_ITEM,
        UPDATE_REPOS,
        REPO,
        FILTER,
        REFRESH,
        INSTALL,
        PKG_SELECT_ALL,
        WARNING,
        QPACMANTRAY,
        QPACMAN,
        CHECKING_MOVIE,
        WAITING_MOVIE,
        DLG_PASSWORD,
        NO_ICON,
        REFRESH_BTN,
        REFRESH_BTN_SHADOW,
        ARCHLINUX
    };
    static const QIcon get(ThemeIcons::Icon id);
    static const QString name(ThemeIcons::Icon id);
};

#endif // THEMEICONS_H
