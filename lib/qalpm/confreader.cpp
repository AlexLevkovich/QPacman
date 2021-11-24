#include "confreader.h"
#include <QFile>
#include <QLatin1Char>

template<class ForwardIt, class T, class Compare> static ForwardIt binary_search_ex(ForwardIt first, ForwardIt last, const T& value, Compare comp) {
    ForwardIt it = std::lower_bound(first, last, value, comp);
    if ((it == last) || comp(value,*it)) it = last;
    return it;
}

ConfReader::ConfReader(const QString & filename) {
    m_sync_is_needed = false;
    m_sort_is_needed = false;
    m_file_name = filename;

    parse();
}

ConfReader::~ConfReader() {
    sync();
}

QString ConfReader::fileName() const {
    return m_file_name;
}

int ConfReader::index_of_key(const QString &key) const {
    int sorted_index;
    return index_of_key(key,sorted_index);
}

int ConfReader::index_of_key(const QString &key,int & sorted_index) const {
    if (m_sort_is_needed) {
        ((ConfReader *)this)->m_sort_is_needed = false;
        ((ConfReader *)this)->sort();
    }
    QList<KeyValue>::const_iterator it = binary_search_ex(m_sorted_key_values.begin(),m_sorted_key_values.end(),key,only_key_cmp);
    if (it == m_sorted_key_values.end()) return -1;
    sorted_index = it-m_sorted_key_values.begin();
    return m_sorted_key_values[sorted_index].index;
}

const QString ConfReader::section_from_key(const QString &key) {
    int index = key.indexOf('/');
    if (index < 0) return QString();

    return key.left(index);
}

const QString ConfReader::item_from_key(const QString &key) {
    int index = key.indexOf('/');
    if (index < 0) return key;

    return key.mid(index+1);
}

QList<int> ConfReader::index_of_section(const QString &section) const {
    QList<int> sorted_indexes;
    return index_of_section(section,sorted_indexes);
}

QList<int> ConfReader::index_of_section(const QString &section,QList<int> & sorted_indexes) const {
    sorted_indexes.clear();
    if (m_sort_is_needed) {
        ((ConfReader *)this)->m_sort_is_needed = false;
        ((ConfReader *)this)->sort();
    }
    QList<KeyValue>::const_iterator it = binary_search_ex(m_sorted_key_values.begin(),m_sorted_key_values.end(),section+"/value",only_section_cmp);
    if (it == m_sorted_key_values.end()) return QList<int>();
    int sorted_index = it-m_sorted_key_values.begin();
    QList<int> ret;
    for (int i = sorted_index;m_sorted_key_values[i].section == section;i++) {
        sorted_indexes.append(i);
        ret.append(m_sorted_key_values[i].index);
    }
    return ret;
}

bool ConfReader::only_key_cmp(const KeyValue & item1, const KeyValue & item2) {
    int ret;
    return ((ret = item1.section.compare(item2.section)) != 0)?(ret < 0):((item1.item.compare(item2.item)) < 0);
}

bool ConfReader::only_section_cmp(const KeyValue & item1, const KeyValue & item2) {
    return (item1.section.compare(item2.section) < 0);
}

QString ConfReader::value(const QString &key, const QString &defaultValue) const {
    int index = index_of_key(key);
    if (index == -1) return defaultValue;

    QString value = m_key_values[index].value;
    return value.isEmpty()?defaultValue:value;
}

QString ConfReader::value(const QString &key, const char * defaultValue) const {
    int index = index_of_key(key);
    if (index == -1) return defaultValue;

    QString value = m_key_values[index].value;
    return value.isEmpty()?QString::fromLocal8Bit(defaultValue):value;
}

int ConfReader::value(const QString &key, int defaultValue) const {
    int index = index_of_key(key);
    if (index == -1) return defaultValue;

    bool ok;
    int value = m_key_values[index].value.toInt(&ok);
    return ok?value:defaultValue;
}

bool ConfReader::value(const QString &key, bool defaultValue) const {
    int index = index_of_key(key);
    if (index == -1) return defaultValue;

    bool ok;
    bool value = (bool)m_key_values[index].value.toInt(&ok);
    return ok?value:defaultValue;
}

void ConfReader::set_new_key_value(const QString & key,const QString & value) {
    KeyValue key_value(key);
    key_value.index = m_key_values.count();
    if (!key.isEmpty()) {
        m_sorted_key_values.append(key_value);
        m_sort_is_needed = true;
    }
    key_value.value = value;
    m_key_values.append(key_value);
}

bool ConfReader::setValue(const QString &key, const QString &value) {
    m_error.clear();
    QString section = section_from_key(key);
    if (section.isEmpty() && item_from_key(key).isEmpty()) {
        m_error = "ConfReader::setValue: " + QObject::tr("wrong input key!");
        return false;
    }

    QString key2 = key;
    if (section.isEmpty()) key2 = "/" + key2;

    int sorted_index;
    int index = index_of_key(key2,sorted_index);
    if (index == -1) {
        set_new_key_value(key2,value);
        m_sync_is_needed = true;
        m_sections.append(section);
        m_sections.removeDuplicates();
        return true;
    }

    m_key_values[index].value = value;
    m_sync_is_needed = true;
    return true;
}

bool ConfReader::setValue(const QString &key, int value) {
    return setValue(key,QString("%1").arg(value));
}

bool ConfReader::setValue(const QString &key, const char * value) {
    return setValue(key,QString::fromLocal8Bit(value));
}

bool ConfReader::setValue(const QString &key, bool value) {
    return setValue(key,QString("%1").arg(value?1:0));
}

void ConfReader::remove(const QString &key) {
    QString section = section_from_key(key);
    if ((section + "/") == key) {
        QList<int> sorted_indexes;
        QList<int> indexes = index_of_section(section,sorted_indexes);
        for (int & idx: indexes) {
            m_key_values[idx].setUnusable();
        }
        for (int & idx: sorted_indexes) {
            m_sorted_key_values.removeAt(idx);
        }
        m_sync_is_needed = true;
        m_sections.removeAll(section);
        return;
    }
    int sorted_index;
    int index = index_of_key(key,sorted_index);
    if (index == -1) return;
    m_key_values[index].setUnusable();
    m_sorted_key_values.removeAt(sorted_index);
    QList<int> indexes = index_of_section(section);
    if (indexes.isEmpty()) m_sections.removeAll(section);
    m_sync_is_needed = true;
}

void ConfReader::clear() {
    m_key_values.clear();
    m_sorted_key_values.clear();
    m_sections.clear();
}

void ConfReader::sort() {
    std::sort(m_sorted_key_values.begin(),m_sorted_key_values.end(),only_key_cmp);
}

bool ConfReader::sync() {
    m_error.clear();
    if (!m_sync_is_needed) return true;

    QFile file(m_file_name);
    if (!file.open(QIODevice::WriteOnly)) {
        m_error = "ConfReader::sync: " + file.errorString();
        return false;
    }
    QString section = "";
    QByteArray line;
    for (KeyValue & key_value: m_key_values) {
        if (key_value.isUnusable()) continue;

        if (key_value.isComment()) {
            line = key_value.value.toLocal8Bit()+"\n";
        }
        else if (key_value.section.isEmpty() || section == key_value.section) {
            line = (key_value.item + " = " + key_value.value).toLocal8Bit()+"\n";
        }
        else {
            line = ("["+key_value.section+"]").toLocal8Bit()+"\n";
            line += (key_value.item + " = " + key_value.value).toLocal8Bit()+"\n";
            section = key_value.section;
        }
        if (file.write(line) != line.length()) {
            m_error = "ConfReader::sync: " + file.errorString();
            return false;
        }
    }

    m_sync_is_needed = false;
    return true;
}

QString ConfReader::lastError() const {
    return m_error;
}

bool ConfReader::read_conf_line(QFile * file,QString & line,int & begin,int & end) {
    m_error.clear();
    line = QString::fromLocal8Bit(file->readLine());
    if (file->error() != QFileDevice::NoError) {
        m_error = "ConfReader::read_conf_line: " + file->errorString();
        return false;
    }
    if ((line.isNull() || line.isEmpty()) && file->atEnd()) return false;

    if (line.endsWith(QLatin1Char('\n'))) line = line.left(line.length()-1);

    for (begin=0;begin<line.length();begin++) {
        if (!line.at(begin).isSpace()) break;
    }
    for (end=(line.length()-1);end >= 0;end--) {
        if (!line.at(end).isSpace()) break;
    }

    return true;
}

bool ConfReader::parse() {
    m_error.clear();

    QFile file(m_file_name);
    if (!file.open(QIODevice::ReadOnly)) {
        m_error = "ConfReader::parse: " + file.errorString();
        return false;
    }

    QString line;
    int begin;
    int end;
    int equal_sign;
    QString section = "";
    QString item = "";
    while (read_conf_line(&file,line,begin,end)) {
        if (line.isNull() || line.trimmed().isEmpty() || line.at(begin) == QLatin1Char('#')) set_new_key_value("",line);
        else if (line.at(begin) == QLatin1Char('[') && line.at(end) == QLatin1Char(']')) {
            section = line.mid(begin+1,(end - begin) - 1);
            if (!section.isNull() && !section.isEmpty()) m_sections.append(section);
        }
        else if ((equal_sign = line.indexOf(QLatin1Char('='),begin)) <= begin) {
            set_new_key_value("",line);
        }
        else {
            item = line.left(equal_sign).trimmed();
            set_new_key_value(section+"/"+item,line.mid(equal_sign+1).trimmed());
        }
    }
    m_sections.removeDuplicates();

    return true;
}

QStringList ConfReader::sections() const {
    return m_sections;
}

QStringList ConfReader::allKeys() const {
    QStringList ret;
    for (const KeyValue & key_value: m_sorted_key_values) {
        ret.append(key_value.section+"/"+key_value.item);
    }
    return ret;
}

QStringList ConfReader::items(const QString & section) const {
    QStringList ret;
    for (int & index: index_of_section(section)) {
        ret.append(m_key_values[index].item);
    }
    return ret;
}
