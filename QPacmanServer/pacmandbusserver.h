/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef PACMANDBUSSERVER_H
#define PACMANDBUSSERVER_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include "pacmanentry.h"
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
#include <QUrl>

class PacmanProcessReader;
class PacmanDownloader;
class PacmanInstallPackagesReader;
class PacmanRemovePackagesReader;

class PacmanDBusServer : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.alexl.PacmanDBusServer")
    Q_CLASSINFO("D-Bus Introspection", ""
                "  <interface name=\"org.alexl.PacmanDBusServer\">\n"
                "    <signal name=\"command_finished\">\n"
                "      <arg name=\"errorMsg\" type=\"s\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"files_ready\">\n"
                "      <arg name=\"package\" type=\"s\" direction=\"out\"/>\n"
                "      <arg name=\"files\" type=\"as\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"packages_to_update\">\n"
                "      <arg name=\"entry\" type=\"as\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"package_ready\">\n"
                "      <arg name=\"entry\" type=\"ay\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"salt_ready\">\n"
                "      <arg name=\"salt\" type=\"ay\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"contents_length_found\">\n"
                "      <arg name=\"len\" type=\"i\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"download_progress\">\n"
                "      <arg name=\"percents\" type=\"i\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"all_downloads_completed\">\n"
                "    </signal>\n"
                "    <signal name=\"start_download\">\n"
                "      <arg name=\"url\" type=\"s\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"some_providers_available\">\n"
                "      <arg name=\"providers\" type=\"as\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"question_available\">\n"
                "      <arg name=\"providers\" type=\"s\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"show_tray_window\">\n"
                "    </signal>\n"
                "    <signal name=\"ready_to_process_install\">\n"
                "      <arg name=\"install_packages\" type=\"as\" direction=\"out\"/>\n"
                "      <arg name=\"remove_packages\" type=\"as\" direction=\"out\"/>\n"
                "      <arg name=\"local_install_packages\" type=\"as\" direction=\"out\"/>\n"
                "      <arg name=\"total_install_size\" type=\"d\" direction=\"out\"/>\n"
                "      <arg name=\"total_remove_size\" type=\"d\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"ready_to_process_remove\">\n"
                "      <arg name=\"remove_packages\" type=\"as\" direction=\"out\"/>\n"
                "      <arg name=\"total_remove_size\" type=\"d\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"start_installing\">\n"
                "      <arg name=\"name\" type=\"s\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"post_messages\">\n"
                "      <arg name=\"package\" type=\"s\" direction=\"out\"/>\n"
                "      <arg name=\"messages\" type=\"as\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"ready_to_process\">\n"
                "      <arg name=\"count\" type=\"i\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"start_removing\">\n"
                "      <arg name=\"name\" type=\"s\" direction=\"out\"/>\n"
                "    </signal>\n"
                "    <signal name=\"client_started\">\n"
                "    </signal>\n"
                "    <signal name=\"client_exited\">\n"
                "    </signal>\n"
                "    <method name=\"commandRequest\">\n"
                "      <arg type=\"s\" direction=\"out\"/>\n"
                "      <arg name=\"command\" type=\"ay\" direction=\"in\"/>\n"
                "    </method>\n"
                "    <method name=\"terminateRequest\">\n"
                "      <arg name=\"command\" type=\"ay\" direction=\"in\"/>\n"
                "    </method>\n"
                "    <method name=\"setPassword\">\n"
                "      <arg name=\"password\" type=\"ay\" direction=\"in\"/>\n"
                "    </method>\n"
                "    <method name=\"setFilePath\">\n"
                "      <arg name=\"file_path\" type=\"s\" direction=\"in\"/>\n"
                "    </method>\n"
                "    <method name=\"setPackageList\">\n"
                "      <arg name=\"packages\" type=\"as\" direction=\"in\"/>\n"
                "    </method>\n"
                "    <method name=\"setPackages\">\n"
                "      <arg name=\"packages\" type=\"s\" direction=\"in\"/>\n"
                "    </method>\n"
                "    <method name=\"setSelectedProvider\">\n"
                "      <arg name=\"provider\" type=\"s\" direction=\"in\"/>\n"
                "    </method>\n"
                "    <method name=\"setUrl\">\n"
                "      <arg name=\"url\" type=\"s\" direction=\"in\"/>\n"
                "    </method>\n"
                "    <method name=\"setChosenProviderIds\">\n"
                "      <arg name=\"providerIds\" type=\"ai\" direction=\"in\"/>\n"
                "      <annotation name=\"org.qtproject.QtDBus.QtTypeName.In0\" value=\"QList&lt;int&gt;\"/>\n"
                "    </method>\n"
                "    <method name=\"setDependance\">\n"
                "      <arg name=\"doDeps\" type=\"b\" direction=\"in\"/>\n"
                "    </method>\n"
                "    <method name=\"sendAnswer\">\n"
                "      <arg name=\"question\" type=\"i\" direction=\"in\"/>\n"
                "    </method>\n"
                "  </interface>\n"
                "")
public:
    explicit PacmanDBusServer(QObject *parent = 0);
    virtual ~PacmanDBusServer();

    static const QString path() {
        return "/org/alexl/PacmanDBusServer";
    }
    static const QString service() {
        return "org.alexl.PacmanDBusServer";
    }

public Q_SLOTS:
    QString commandRequest(const QByteArray & command);
    void terminateRequest(const QByteArray & command = QByteArray());
    void setPassword(const QByteArray & password);
    void setUrl(const QString & url);
    void setFilePath(const QString & file_path);
    void setPackageList(const QStringList & packages);
    void setPackages(const QString & packages);
    void setSelectedProvider(const QString & provider);
    void setDependance(bool doDeps);
    void beginInstall();
    void cancelInstall();
    void beginRemove();
    void cancelRemove();
    void sendAnswer(int answer);
private slots:
    void _completed(QObject * obj,int code,const QString & error);
    void read_finished(PacmanProcessReader * ptr);
    void refresher_finished(PacmanProcessReader * ptr);
    void simple_updates_finished(PacmanProcessReader * ptr);
    void package_info_finished(PacmanProcessReader * ptr);
    void install_packages_finished(PacmanProcessReader * ptr);
    void read_remove_packages_finished(PacmanProcessReader * ptr);

    //signal senders
    void emit_package_ready(const PacmanEntry & entry);
    void emit_command_finished(const QByteArray & command,const QString &errorMsg);
    void emit_files_ready(const QString & package,const QStringList & files);
    void emit_packages_to_update(const QStringList & entry);
    void emit_salt_ready(const QByteArray & salt);
    void emit_contents_length_found(int len);
    void emit_download_progress(int percents);
    void emit_start_download(const QString & url);
    void emit_all_downloads_completed();
    void emit_start_installing(const QString & name);
    void emit_post_messages(const QString & package,const QStringList & messages);
    void emit_ready_to_process_install(int);
    void emit_ready_to_process_remove(int);
    void emit_start_removing(const QString & name);
    void emit_some_providers_available(const QStringList & providers);
    void emit_question_available(const QString & question);
    void emit_show_tray_window();
    void emit_client_started();
    void emit_client_exited();

private:
    QByteArray m_password;
    QUrl m_url;
    QString m_file_path;
    QStringList m_packages_list;
    QString m_packages;
    int m_doDeps;
    QStringList m_client_names;
    QMap<QObject *,QByteArray> m_commands;

    PacmanInstallPackagesReader * installreader;
    PacmanInstallPackagesReader * localinstallreader;
    PacmanRemovePackagesReader * removereader;

    void _init();
    QString invalidPasswordError(const QByteArray & password) const;
};

#endif // PACMANDBUSSERVER_H
