/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef CONFSETTINGS_H
#define CONFSETTINGS_H

#include <QSettings>

class ConfSettings : public QSettings {
    Q_OBJECT
public:
    ConfSettings(const QString & confName,QObject *parent = 0);
    bool replaceXferCommand();
    bool copyFromFile(const QString & fileName);
    bool copyFromPacmanConf();

    static const QString createTempConfName();
private:
    static QSettings::Format ConfFormat;

    static bool readConfFile(QIODevice & device, QSettings::SettingsMap & map);
    static bool writeConfFile(QIODevice & device, const QSettings::SettingsMap & map);
};

#endif // CONFSETTINGS_H
