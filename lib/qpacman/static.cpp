#include "static.h"
#include <QCoreApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QTemporaryFile>
#include <QDir>

const QString fixButtonText(const QString & label) {
    QString fixedLabel;
    int i = 0;
    int lastIndex = label.length() - 1;
    while(i < lastIndex) {
        if (label[i] == '&') {
            if (label[i + 1] == '&') {
                fixedLabel += label[i];
                ++i;
            }
        }
        else fixedLabel += label[i];
        ++i;
    }
    if (label[lastIndex] != '&') fixedLabel += label[lastIndex];

    return fixedLabel;
}

const QSize quadroSize(int dimension) {
    QSize size(dimension,dimension);
    return size;
}

void setupTranslations(const QString & mainName,const QString & installDir,const QString & mainLocalDir,const QString & alpmLocalDir,const QString & libLocalDir) {
    QTranslator * m_translator = new QTranslator(QCoreApplication::instance());
    if(!m_translator->load(QLocale::system(),mainName,"_",mainLocalDir)) m_translator->load(QLocale::system(),mainName,"_",installDir);
    QCoreApplication::installTranslator(m_translator);
    QTranslator * m_translator2 = new QTranslator(QCoreApplication::instance());
    if (m_translator2->load(QLocale::system(),"qt","_",QLibraryInfo::location(QLibraryInfo::TranslationsPath))) QCoreApplication::installTranslator(m_translator2);
    QTranslator * m_translator3 = new QTranslator(QCoreApplication::instance());
    if(!m_translator3->load(QLocale::system(),"libqpacman","_",libLocalDir)) m_translator3->load(QLocale::system(),"libqpacman","_",installDir);
    QCoreApplication::installTranslator(m_translator3);
    QTranslator * m_translator4 = new QTranslator(QCoreApplication::instance());
    if(!m_translator4->load(QLocale::system(),"libqalpm","_",alpmLocalDir)) m_translator4->load(QLocale::system(),"libqalpm","_",installDir);
    QCoreApplication::installTranslator(m_translator4);
}

void setIniValue(const QString & key,const QVariant & value) {
    QSettings settings;
    settings.setValue("settings/"+key,value);
}

const QString temporaryName(const QString & dir,const QString & basename) {
    QTemporaryFile tfile(dir+QDir::separator()+basename);
    if (!tfile.open()) return QString();
    return tfile.fileName();
}
