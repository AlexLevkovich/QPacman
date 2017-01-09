/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanentry.h"
#ifdef IS_QPACMAN_CLIENT
#include "static.h"
#include <QFontMetrics>
#include <QMainWindow>
#endif
#include "byteshumanizer.h"

#ifndef max
#define max(a,b) ((a>b)?a:b)
#endif

#include <QUrl>

static QString escape(const QString& plain) {
    QString rich;
    rich.reserve(int(plain.length() * double(1.1)));
    for (int i = 0; i < plain.length(); ++i) {
        if (plain.at(i) == QLatin1Char('<'))
            rich += QLatin1String("&lt;");
        else if (plain.at(i) == QLatin1Char('>'))
            rich += QLatin1String("&gt;");
        else if (plain.at(i) == QLatin1Char('&'))
            rich += QLatin1String("&amp;");
        else if (plain.at(i) == QLatin1Char('"'))
            rich += QLatin1String("&quot;");
        else
            rich += plain.at(i);
    }
    return rich;
}

PacmanEntry::PacmanEntry() {
    _flag = unknown_flag;
    m_isChosen = false;
    m_isUpdate = false;
    status = NOT_INSTALLED;
    change_status = DO_INSTALL;
}

PacmanEntry::PacmanEntry(const QString & name,const QString & ver) {
    this->name = name;
    version = ver;
    _flag = unknown_flag;
    m_isChosen = false;
    m_isUpdate = false;
    status = NOT_INSTALLED;
    change_status = DO_INSTALL;
}

PacmanEntry::PacmanEntry(const QString & name) {
    this->name = name;
    _flag = unknown_flag;
    m_isChosen = false;
    m_isUpdate = false;
    status = NOT_INSTALLED;
    change_status = DO_INSTALL;
}

QString PacmanEntry::toString() const {
    return name+"="+version;
}

const QStringList PacmanEntry::entriesListToStringList(const QList<PacmanEntry> & list) {
    QStringList ret;
    for (int i=0;i<list.count();i++) ret.append(list[i].toString());
    return ret;
}

const QStringList PacmanEntry::entriesListToNamesStringList(const QList<PacmanEntry> & list) {
    QStringList ret;
    for (int i=0;i<list.count();i++) ret.append(list[i].getName());
    return ret;
}

const QString PacmanEntry::afterColon(const QString & str) {
    int index = str.indexOf(':');
    if (index == -1) return QString();

    return str.mid(index+1).simplified();
}

const QString PacmanEntry::afterEqualSign(const QString & str) {
    int index = str.indexOf(" =");
    if (index == -1) return QString();

    return str.mid(index+2).simplified();
}

void PacmanEntry::afterBeforeColon(const QString & line,QString & str_before,QString & str_after) {
    int index = line.indexOf(':');
    if (index == -1) {
        str_before = line.simplified();
        str_after = "";
        return;
    }

    str_before= line.mid(0,index).simplified();
    str_after = line.mid(index+1).simplified();
}

const QStringList PacmanEntry::parseStrList(const QString & str) {
    QStringList list = afterColon(str).split(" ",QString::SkipEmptyParts);
    if ((list.count() == 1) && (list[0] == "None")) list.clear();
    return list;
}

const QStringList PacmanEntry::parseStrList2(const QString & str) {
    QStringList list = str.split(" ",QString::SkipEmptyParts);
    if ((list.count() == 1) && (list[0] == "None")) list.clear();
    return list;
}

const QString C_MONTHS[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static int indexMonth(const QString & month) {
    for (int i=0;i<12;i++) {
        if (C_MONTHS[i] == month) return i+1;
    }
    return -1;
}

bool PacmanEntry::isMailAddress(const QString & str) {
    QUrl url("http://mailto:"+str);
    if (!url.isValid()) return false;

    int index = str.indexOf('@');
    if (index == -1) return false;

    return ((url.userName() == "mailto") && (url.password() == str.left(index)));
}

const QString PacmanEntry::modifyMailAddress(const QString & str) {
    return "mailto:"+str;
}

const QDateTime PacmanEntry::parseDate(const QString & str) {
    QStringList parts = str.split(" ",QString::SkipEmptyParts);
    if (parts.count() != 5) {
        if (parts.count() == 6) {
            parts.removeAt(4);
        } else return QDateTime();
    }
    int month = indexMonth(parts[1]);
    if (month == -1) return QDateTime();
    bool ok;
    int year = parts[4].toInt(&ok);
    if (!ok) return QDateTime();
    int day = parts[2].toInt(&ok);
    if (!ok) return QDateTime();
    QDate date(year,month,day);
    if (!date.isValid()) return QDateTime();
    QTime time = QTime::fromString(parts[3]);
    if (!time.isValid()) return QDateTime();

    return QDateTime(date,time);
}

PacmanEntry::ParseCode PacmanEntry::parseLine(const QByteArray & array) {
    QString line = QString::fromLocal8Bit(array);
    if (line.endsWith("\n")) line = line.left(line.length()-1);
    if (line.length() <= 0) return EMPTY;

    if (!line.startsWith("  ")) _flag = unknown_flag;
    if (line.startsWith("Name")) name = afterColon(line);
    else if (line.startsWith("pkgname =")) name = afterEqualSign(line);
    else if (line.startsWith("Repository")) repo = afterColon(line);
    else if (line.startsWith("Version")) version = afterColon(line);
    else if (line.startsWith("pkgver =")) version = afterEqualSign(line);
    else if (line.startsWith("Description")) desc = afterColon(line);
    else if (line.startsWith("pkgdesc =")) desc = afterEqualSign(line);
    else if (line.startsWith("Architecture")) arch = afterColon(line);
    else if (line.startsWith("arch =")) arch = afterEqualSign(line);
    else if (line.startsWith("URL")) url = escape(afterColon(line));
    else if (line.startsWith("url =")) url = escape(afterEqualSign(line));
    else if (line.startsWith("Licenses")) lic = afterColon(line);
    else if (line.startsWith("license =")) lic += afterEqualSign(line)+" ";
    else if (line.startsWith("Groups")) groups = parseStrList(line);
    else if (line.startsWith("group =")) groups.append(afterEqualSign(line));
    else if (line.startsWith("Provides")) provides = parseStrList(line);
    else if (line.startsWith("provides =")) provides.append(afterEqualSign(line));
    else if (line.startsWith("Depends On ")) dependon = parseStrList(line);
    else if (line.startsWith("depend =")) dependon.append(afterEqualSign(line));
    else if (line.startsWith("Optional Deps")) {
        Deps dep;
        line = afterColon(line);
        afterBeforeColon(line,dep.package,dep.desc);
        if (dep.package != "None") {
            if (dep.desc.isEmpty() && dep.package.endsWith(" [installed]")) {
                dep.package = dep.package.left(dep.package.length()-12);
            }
            optionaldeps.append(dep);
            _flag = optionaldeps_flag;
        }
    }
    else if (line.startsWith("optdepend =")) {
        Deps dep;
        line = afterEqualSign(line);
        afterBeforeColon(line,dep.package,dep.desc);
        optionaldeps.append(dep);
    }
    else if (line.startsWith("Required By")) requiredby = parseStrList(line);
    else if (line.startsWith("Optional For")) optionalfor = parseStrList(line);
    else if (line.startsWith("Conflicts With")) conflicts = parseStrList(line);
    else if (line.startsWith("conflict =")) conflicts.append(afterEqualSign(line));
    else if (line.startsWith("Replaces")) replaces = parseStrList(line);
    else if (line.startsWith("replaces =")) replaces.append(afterEqualSign(line));
    else if (line.startsWith("Installed Size")) instsize = BytesHumanizer(afterColon(line)).value();
    else if (line.startsWith("size =")) instsize = (double)afterEqualSign(line).toDouble();
    else if (line.startsWith("Packager")) packager = escape(afterColon(line));
    else if (line.startsWith("packager =")) packager = escape(afterEqualSign(line));
    else if (line.startsWith("Build Date")) {
        builddate = parseDate(afterColon(line));
        if (!builddate.isValid()) {
            warnings.append(QString("Pacman's wrong output for date: %1").arg(line));
            return WARNINGS;
        }
    }
    else if (line.startsWith("builddate =")) builddate = QDateTime::fromTime_t((uint)afterEqualSign(line).toUInt());
    else if (line.startsWith("Install Date")) {
        installdate = parseDate(afterColon(line));
        if (!installdate.isValid()) {
            warnings.append(QString("Pacman's wrong output for date: %1").arg(line));
            return WARNINGS;
        }
    }
    else if (line.startsWith("Install Reason")) {
        if (afterColon(line).startsWith("Explicitly installed")) status = INSTALLED_EXPLICITLY;
        else status = INSTALLED_AS_DEP;
        change_status = DO_UNINSTALL_ALL;
    }
    else if (line.startsWith("Validated By")) {
         if (repo.isEmpty()) repo = "aur";
    }
    else if (_flag == optionaldeps_flag) {
        if (line.startsWith("  ")) {
            Deps dep;
            afterBeforeColon(line.simplified(),dep.package,dep.desc);
            if (dep.desc.isEmpty() && dep.package.endsWith(" [installed]")) {
                dep.package = dep.package.left(dep.package.length()-12);
            }
            optionaldeps.append(dep);
        }
        else _flag = unknown_flag;
    }

    return OK;
}

int PacmanEntry::versionIndex(const QString & str) {
    int index = str.indexOf(">");
    if (index == -1) {
        index = str.indexOf("<");
        if (index == -1) {
            index = str.indexOf("=");
        }
    }

    return -1;
}

const QString PacmanEntry::removeVersion(const QString & str) {
    int index = versionIndex(str);
    if (index == -1) return str.simplified();

    return str.mid(0,index).simplified();
}

const QString PacmanEntry::pacmanDepToUrlParms(const QString & str) {
    QString name;
    QString ver;
    CompareOper oper = parseNameVersion(str,name,ver);
    if (oper == UNKNOWN) return name;
    return name+QString("?oper=%1&ver=%2").arg(oper).arg(ver);
}

const QString PacmanEntry::urlParmsToPacmanDep(const QString & str) {
    int index = str.lastIndexOf('?');
    if (index == -1) return str;
    QString name = str.mid(0,index);
    QStringList parms = str.mid(index+1).split('&',QString::SkipEmptyParts);
    QString oper;
    QString ver;
    for (int i=0;i<parms.count();i++) {
        if (parms.at(i).startsWith("oper=")) {
            oper = compareOperToString((CompareOper)parms.at(i).mid(5).toInt());
        }
        else if (parms.at(i).startsWith("ver=")) {
            ver = parms.at(i).mid(4);
        }
    }
    return name+oper+ver;
}

bool PacmanEntry::splitname_ver(const QString & target,QString & name,QString & version) {
    int index = target.lastIndexOf('-');
    if (index == -1) return false;

    bool ok;
    target.mid(index+1).toDouble(&ok);
    if (!ok) return false;

    index = target.lastIndexOf('-',index-1);
    if (index == -1) return false;
    version = target.mid(index+1);
    name = target.mid(0,index);

    return true;
}

PacmanEntry::CompareOper PacmanEntry::parseNameVersion(const QString & str,QString & name,QString & ver) {
    int count = 2;
    PacmanEntry::CompareOper oper = UNKNOWN;
    int index = str.indexOf(">=");
    if (index == -1) {
        index = str.indexOf("<=");
        if (index == -1) {
            index = str.indexOf("=");
            if (index == -1) {
                index = str.indexOf(">");
                if (index == -1) {
                    index = str.indexOf("<");
                    if (index == -1) {
                        if (splitname_ver(str,name,ver)) return EQUAL;
                        else {
                            name = str.simplified();
                            ver = "";
                            return oper;
                        }
                    }
                    else {
                        count = 1;
                        oper = LESS;
                    }
                }
                else {
                    count = 1;
                    oper = MORE;
                }
            }
            else {
                count = 1;
                oper = EQUAL;
            }
        }
        else oper = LESS_OR_EQUAL;
    }
    else oper = MORE_OR_EQUAL;

    name = str.mid(0,index).simplified();
    ver = str.mid(index+count).simplified();

    return oper;
}

const QString PacmanEntry::parsePackageFileNameVersion(const QString & package_file) {
    int index = package_file.indexOf(".pkg.tar");
    if (index == -1) return QString();

    index = package_file.lastIndexOf('-',index);
    if (index == -1) return QString();

    QString name;
    QString ver;
    parseNameVersion(package_file.left(index),name,ver);
    if (ver.isEmpty()) return QString();

    return name+"-"+ver;
}

const QString PacmanEntry::compareOperToString(PacmanEntry::CompareOper oper) {
    switch (oper) {
        case MORE:
            return ">";
        case MORE_OR_EQUAL:
            return ">=";
        case LESS:
            return "<";
        case LESS_OR_EQUAL:
            return "<=";
        case EQUAL:
            return "=";
        case UNKNOWN:
        default:
            break;
    }

    return QString();
}

bool PacmanEntry::ownedByGroup(const QString & group) {
    return groups.contains(group);
}

#ifndef IS_QPACMAN_SERVER
bool PacmanEntry::containsText(const QString & text,CategoryToolButton::ItemId cItemId) {
    if (text.isEmpty()) return true;
    if (cItemId == CategoryToolButton::IS_NAME) return name.contains(text,Qt::CaseInsensitive);
    else if (cItemId == CategoryToolButton::IS_DESC) return desc.contains(text,Qt::CaseInsensitive);
    else if (cItemId == CategoryToolButton::IS_FILE_NAME) {
        if (!isInstalled()) return false;
        if (files.count() <= 0) return false;
        for (int i=0;i<files.count();i++) {
            if (files[i].contains(text,Qt::CaseInsensitive)) return true;
        }
        return false;
    }
    else if (cItemId == CategoryToolButton::IS_PROVIDER) return provides.contains(text,Qt::CaseInsensitive);

    return true;
}
#endif

const QString PacmanEntry::removeInstalledWord(const QString & str,bool * result) {
    int index = str.indexOf(" [installed]");
    if (index != -1) {
        if (result != NULL) *result = true;
        return str.left(index);
    }

    if (result != NULL) *result = false;
    return str;
}

#ifdef IS_QPACMAN_CLIENT
const QString PacmanEntry::fieldNames[19] = {QObject::tr("Name"),QObject::tr("Version"),QObject::tr("Description"),
                                             QObject::tr("Architecture"),QObject::tr("URL"),QObject::tr("Licenses"),
                                             QObject::tr("Groups"),QObject::tr("Provides"),QObject::tr("Depends On"),
                                             QObject::tr("Optional Deps"),QObject::tr("Required By"),QObject::tr("Optional For"),
                                             QObject::tr("Conflicts With"),QObject::tr("Replaces"),QObject::tr("Installed Size"),
                                             QObject::tr("Packager"),QObject::tr("Build Date"),QObject::tr("Install Date"),
                                             QObject::tr("Install Reason")};

QString PacmanEntry::toHtml() const {
    int i;

    QFontMetrics fm(Static::findMainWindow()->font());
    int width = 0;
    for (int i=0;i<19;i++) {
        width = max(width,fm.width(fieldNames[i]));
    }
    width += 10;

    QString first_col_color = QPalette().button().color().name();

    QString ret = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
                    "<html>"
                    "<head>"
                    "<meta name=\"qrichtext\" content=\"1\" />"
                    "</head><body dir=\"ltr\" style=\"margin-top:0.1cm; margin-bottom:0.1cm; margin-left:0.1cm; margin-right:0.1cm; writing-mode:lr-tb; \">"
                    "<table width=\"100%\" border=\"0.5\" cellspacing=\"0\" cellpadding=\"0\" >";

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%3\" style=\"text-align:left; \" width=\"%2\"><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >").arg(QObject::tr("Name")).arg(width).arg(first_col_color);
    ret +=  QString("<a href=\"qpc://pack/%1\"><img src=\"qpc://pack/%1\"></a>").arg(pacmanDepToUrlParms(getName())) + "</p></td></tr>";

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%3\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >%2</p></td>"
                    "</tr>").arg(QObject::tr("Version")).arg(getVersion()).arg(first_col_color);

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%3\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >%2</p></td>"
                    "</tr>").arg(QObject::tr("Description")).arg(getDescription()).arg(first_col_color);

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%3\" style=\"text-align:left; display: inline;\" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >%2</p></td>"
                    "</tr>").arg(QObject::tr("Architecture")).arg(getArch()).arg(first_col_color);

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%4\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p ><a href=\"%2\">%3</a></p></td>"
                    "</tr>").arg(QObject::tr("URL")).arg(isMailAddress(getURL())?modifyMailAddress(getURL()):getURL()).arg(getURL()).arg(first_col_color);

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%3\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >%2</p></td>"
                    "</tr>").arg(QObject::tr("Licenses")).arg(getLicense()).arg(first_col_color);

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%3\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >").arg(QObject::tr("Groups")).arg(first_col_color);
    for (i=0;i<groups.count();i++) ret += QString("<a href=\"qpc://group.%1\">%2</a>%3 ").arg(groups[i]).arg(groups[i]).arg((groups.count() == (i+1))?"":",");
    ret += "</p></td></tr>";

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%3\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >").arg(QObject::tr("Provides")).arg(first_col_color);
    for (i=0;i<provides.count();i++) ret += QString("<a href=\"qpc://pack/%1\"><img src=\"qpc://pack/%1\"></a>%2 ").arg(pacmanDepToUrlParms(provides[i])).arg((provides.count() == (i+1))?"":",");
    ret += "</p></td></tr>";

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%2\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >").arg(QObject::tr("Depends On")).arg(first_col_color);
    for (i=0;i<dependon.count();i++) {
        ret += QString("<a href=\"qpc://pack/%1\"><img src=\"qpc://pack/%1\"></a>%2 ").arg(pacmanDepToUrlParms(dependon[i])).arg((dependon.count() == (i+1))?"":",");
    }
    ret += "</p></td></tr>";

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%2\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" >").arg(QObject::tr("Optional Deps")).arg(first_col_color);
    for (i=0;i<optionaldeps.count();i++) {
        QString _desc = removeInstalledWord(optionaldeps[i].desc).simplified();
        ret += QString("<li >") + QString("<a href=\"qpc://pack/%1\"><img src=\"qpc://pack/optional/%1\"></a> ").arg(pacmanDepToUrlParms(optionaldeps[i].package)) + QString("%1 %2</li>").arg(_desc.isEmpty()?"":":").arg(_desc);
    }
    ret += "</td></tr>";

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%2\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >").arg(QObject::tr("Required By")).arg(first_col_color);
    for (i=0;i<requiredby.count();i++) ret += QString("<a href=\"qpc://pack/%1\"><img src=\"qpc://pack/%1\"></a>%2 ").arg(pacmanDepToUrlParms(requiredby[i])).arg((requiredby.count() == (i+1))?"":",");
    ret += "</p></td></tr>";

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%2\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >").arg(QObject::tr("Optional For")).arg(first_col_color);
    for (i=0;i<optionalfor.count();i++) ret += QString("<a href=\"qpc://pack/%1\"><img src=\"qpc://pack/%1\"></a>%2 ").arg(pacmanDepToUrlParms(optionalfor[i])).arg((optionalfor.count() == (i+1))?"":",");
    ret += "</p></td></tr>";

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%2\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >").arg(QObject::tr("Conflicts With")).arg(first_col_color);
    for (i=0;i<conflicts.count();i++) ret += QString("<a href=\"qpc://pack/%1\"><img src=\"qpc://pack/%1\"></a>%2 ").arg(pacmanDepToUrlParms(conflicts[i])).arg((conflicts.count() == (i+1))?"":",");
    ret += "</p></td></tr>";

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%2\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >").arg(QObject::tr("Replaces")).arg(first_col_color);
    for (i=0;i<replaces.count();i++) ret += QString("<a href=\"qpc://pack/%1\"><img src=\"qpc://pack/%1\"></a>%2 ").arg(pacmanDepToUrlParms(replaces[i])).arg((replaces.count() == (i+1))?"":",");
    ret += "</p></td></tr>";

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%3\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >%2</p></td>"
                    "</tr>").arg(QObject::tr("Installed Size")).arg(BytesHumanizer(getInstallationSize()).toString()).arg(first_col_color);

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%3\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >%2</p></td>"
                    "</tr>").arg(QObject::tr("Packager")).arg(getPackager()).arg(first_col_color);

    ret +=  QString("<tr>"
                    "<td bgcolor=\"%3\" style=\"text-align:left; \" ><p >%1</p></td>"
                    "<td style=\"text-align:left; \" ><p >%2</p></td>"
                    "</tr>").arg(QObject::tr("Build Date")).arg(builddate.toString("ddd MMM dd yyyy hh:mm:ss")).arg(first_col_color);

    if (isInstalled()) {
        ret +=  QString("<tr>"
                        "<td bgcolor=\"%3\" style=\"text-align:left; \" ><p >%1</p></td>"
                        "<td style=\"text-align:left; \" ><p >%2</p></td>"
                        "</tr>").arg(QObject::tr("Install Date")).arg(installdate.toString("ddd MMM dd yyyy hh:mm:ss")).arg(first_col_color);
    }

    if (isInstalled()) {
         ret +=  QString("<tr>"
                         "<td bgcolor=\"%5\" style=\"text-align:left; \" ><p >%1</p></td>"
                         "<td style=\"text-align:left; \" ><p >%2 (<a href=\"qpc://reason.%3\">%4</a>)</p></td>"
                         "</tr>").arg(QObject::tr("Install Reason")).arg(isExplicitly()?QObject::tr("Explicitly installed"):QObject::tr("Installed as a dependency for another package")).arg(name).arg(isExplicitly()?QObject::tr("Make it dependent"):QObject::tr("Make it explicit")).arg(first_col_color);
    }

    ret +=  "</table><p > </p></body></html>";

    return ret;
}

#endif

bool PacmanEntry::isOrphaned() {
    return (isInstalled() && requiredby.isEmpty() && !isExplicitly());
}

QDataStream & operator<<(QDataStream & stream,const PacmanEntry::Deps & entry) {
    stream << entry.package;
    stream << entry.desc;
    return stream;
}

QDataStream & operator>>(QDataStream & stream,PacmanEntry::Deps & entry) {
    stream >> entry.package;
    stream >> entry.desc;
    return stream;
}

QDataStream & operator<<(QDataStream & stream,const PacmanEntry & entry) {
    stream << entry.name;
    stream << entry.repo;
    stream << entry.version;
    stream << entry.desc;
    stream << entry.arch;
    stream << entry.url;
    stream << entry.lic;
    stream << entry.groups;
    stream << entry.provides;
    stream << entry.dependon;
    stream << entry.optionaldeps;
    stream << entry.requiredby;
    stream << entry.optionalfor;
    stream << entry.conflicts;
    stream << entry.replaces;
    stream << entry.instsize;
    stream << entry.packager;
    stream << entry.builddate;
    stream << entry.installdate;
    stream << entry.warnings;
    stream << entry.files;
    stream << entry.m_isUpdate;
    stream << entry.m_isChosen;
    stream << (qint32)entry._flag;
    stream << (qint32)entry.status;
    stream << (qint32)entry.change_status;

    return stream;
}

QDataStream & operator>>(QDataStream & stream,PacmanEntry & entry) {
    stream >> entry.name;
    stream >> entry.repo;
    stream >> entry.version;
    stream >> entry.desc;
    stream >> entry.arch;
    stream >> entry.url;
    stream >> entry.lic;
    stream >> entry.groups;
    stream >> entry.provides;
    stream >> entry.dependon;
    stream >> entry.optionaldeps;
    stream >> entry.requiredby;
    stream >> entry.optionalfor;
    stream >> entry.conflicts;
    stream >> entry.replaces;
    stream >> entry.instsize;
    stream >> entry.packager;
    stream >> entry.builddate;
    stream >> entry.installdate;
    stream >> entry.warnings;
    stream >> entry.files;
    stream >> entry.m_isUpdate;
    stream >> entry.m_isChosen;
    qint32 _flag;
    stream >> _flag;
    entry._flag = (PacmanEntry::StringListFlags)_flag;
    qint32 status;
    stream >> status;
    entry.status = (PacmanEntry::InstalledStatus)status;
    stream >> status;
    entry.change_status = (PacmanEntry::UserChangeStatus)status;

    return stream;
}
