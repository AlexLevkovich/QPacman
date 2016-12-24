/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "confsettings.h"
#include <QTextStream>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QStringList>

extern QString pacman_conf;
QSettings::Format ConfSettings::ConfFormat = QSettings::InvalidFormat;

ConfSettings::ConfSettings(const QString & confName,QObject *parent) : QSettings(confName,(ConfSettings::ConfFormat == QSettings::InvalidFormat)?QSettings::registerFormat("conf",ConfSettings::readConfFile,ConfSettings::writeConfFile):ConfSettings::ConfFormat,parent) {
    ConfSettings::ConfFormat = format();
}

bool ConfSettings::readConfFile(QIODevice & device, QSettings::SettingsMap & map) {
    QString section;
    QTextStream stream(&device);

    while (!stream.atEnd()) {
        QString line = stream.readLine().simplified();

        // Skip comments and empty lines
        if (line.startsWith('#') || line.isEmpty())
            continue;

        // Section
        if (line.startsWith('[') && line.endsWith(']')) {
            section = line.mid(1, line.length()-2);
            continue;
        }

        QString key = line.section('=', 0, 0).trimmed();
        QString value = line.section('=', 1).trimmed();

        if (key.isEmpty()) continue;
        if (section.isEmpty()) return false;
        if (key.simplified() == "VerbosePkgLists") continue;

        key.prepend("/");
        key.prepend(section);

        if (line.contains('=')) map.insert(key, value);
        else map.insert(key, QString());
    }

    return true;
}

/*! See readDesktopFile
 */
bool ConfSettings::writeConfFile(QIODevice & device, const QSettings::SettingsMap & map) {
    QTextStream stream(&device);
    QString section;

    foreach (QString key, map.keys()) {
        if (! map.value(key).canConvert<QString>()) return false;

        QString thisSection = key.section("/", 0, 0);
        if (thisSection.isEmpty()) return false;

        if (thisSection != section) {
            stream << "[" << thisSection << "]" << "\n";
            section = thisSection;
        }

        QString remainingKey = key.section("/", 1, -1);

        if (remainingKey.isEmpty()) return false;

        QString value = map.value(key).toString();
        if (value.isNull()) stream << remainingKey << "\n";
        else stream << remainingKey << "=" << value << "\n";
    }

    return true;
}

bool ConfSettings::replaceXferCommand() {
    setValue("options/XferCommand",QString("%1 --passive-ftp --progress=bar:force -c -O %o %u -T 30").arg(WGET_BIN));
    return true;
}

const QString ConfSettings::createTempConfName() {
    QTemporaryFile tempFile;
    if (!tempFile.open()) return "";
    return tempFile.fileName();
}

bool ConfSettings::copyFromFile(const QString & fileName) {
    if (!QFileInfo(fileName).exists()) return false;

    ConfSettings file_settings(fileName);

    const QStringList keys = file_settings.allKeys();
    Q_FOREACH(QString key, keys) {
        setValue(key,file_settings.value(key));
    }

    return true;
}

bool ConfSettings::copyFromPacmanConf() {
    return copyFromFile(pacman_conf);
}
