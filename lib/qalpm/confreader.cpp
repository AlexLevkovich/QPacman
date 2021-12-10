/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/

#include "confreader.h"
#include <QFile>
#include <QChar>
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

    read();
}

ConfReader::ConfReader() {
    m_sync_is_needed = false;
    m_sort_is_needed = false;
}

ConfReader::~ConfReader() {
    sync();
}

void ConfReader::setFileName(const QString & fileName) {
    m_file_name = fileName;
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
    return m_sorted_key_values[sorted_index].index();
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
    for (int i = sorted_index;m_sorted_key_values[i].section() == section;i++) {
        sorted_indexes.append(i);
        ret.append(m_sorted_key_values[i].index());
    }
    return ret;
}

bool ConfReader::keyExists(const QString &key) const {
    return (index_of_key(key) != -1);
}

bool ConfReader::only_key_cmp(const KeyValue & item1, const KeyValue & item2) {
    int ret;
    return ((ret = item1.section().compare(item2.section())) != 0)?(ret < 0):((item1.item().compare(item2.item())) < 0);
}

bool ConfReader::only_section_cmp(const KeyValue & item1, const KeyValue & item2) {
    return (item1.section().compare(item2.section()) < 0);
}

void ConfReader::set_new_key_value(const QString & key,const QString & value) {
    if (!m_key_values.isEmpty() && m_key_values.last().key() == key) {
        m_key_values.last().addValue(value);
        return;
    }

    if (!key.isEmpty()) {
        int index = index_of_key(key);
        if (index >= 0) {
            m_key_values[index].addValue(value);
            return;
        }
    }

    KeyValue key_value(key,m_key_values.count());
    if (!key.isEmpty()) {
        m_sorted_key_values.append(key_value);
        m_sort_is_needed = true;
    }
    key_value.addValue(value);
    m_key_values.append(key_value);
}

void ConfReader::set_new_key_values(const QString & key,const QStringList & values) {
    if (!m_key_values.isEmpty() && m_key_values.last().key() == key) {
        m_key_values.last().addValues(values);
        return;
    }

    if (!key.isEmpty()) {
        int index = index_of_key(key);
        if (index >= 0) {
            m_key_values[index].addValues(values);
            return;
        }
    }

    KeyValue key_value(key,m_key_values.count());
    if (!key.isEmpty()) {
        m_sorted_key_values.append(key_value);
        m_sort_is_needed = true;
    }
    key_value.setValues(values);
    m_key_values.append(key_value);
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
    m_sync_is_needed = true;
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
        if (!key_value.isOrphan()) continue;

        line.clear();
        for (QString & value: key_value.values()) {
            line += (key_value.item() + " = " + value).toLocal8Bit()+"\n";
        }

        if (file.write(line) != line.length()) {
            m_error = "ConfReader::sync: " + file.errorString();
            return false;
        }
    }

    section = "";
    for (KeyValue & key_value: m_key_values) {
        if (key_value.isUnusable()) continue;
        if (key_value.isOrphan()) continue;

        if (key_value.isComment()) {
            line.clear();
            for (QString & value: key_value.values()) {
                line += value.toLocal8Bit()+"\n";
            }
        }
        else if (section == key_value.section()) {
            line.clear();
            line += write_value(key_value.item(),key_value.values());
        }
        else {
            line = ("["+key_value.section()+"]").toLocal8Bit()+"\n";
            line += write_value(key_value.item(),key_value.values());
            section = key_value.section();
        }
        if (file.write(line) != line.length()) {
            m_error = "ConfReader::sync: " + file.errorString();
            return false;
        }
    }

    m_sync_is_needed = false;
    return true;
}

QByteArray ConfReader::write_value(const QString & item,const QStringList &values) const {
    QByteArray line;
    for (const QString & value: values) {
        line += (value.isNull()?item:(item + " = " + value)).toLocal8Bit()+"\n";
    }
    return line;
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

bool ConfReader::read() {
    m_sync_is_needed = false;
    m_sort_is_needed = false;
    m_error.clear();
    m_key_values.clear();
    m_sorted_key_values.clear();
    m_error.clear();
    m_sections.clear();

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
    QString section2 = "";
    QString item = "";
    QString value;
    while (read_conf_line(&file,line,begin,end)) {
        if (line.trimmed().isEmpty() || line.at(begin) == QLatin1Char('#')) {
            set_new_key_value("",line);
        }
        else if (line.at(begin) == QLatin1Char('[') && line.at(end) == QLatin1Char(']')) {
            section2 = line.mid(begin+1,(end - begin) - 1);
            if (!section2.isNull() && !section2.isEmpty()) {
                m_sections.append(section2);
                section = section2;
            }
        }
        else if ((equal_sign = line.indexOf(QLatin1Char('='),begin)) <= begin) {
            set_new_key_value(section+"/"+line.trimmed(),QString());
        }
        else {
            item = line.left(equal_sign).trimmed();
            value = line.mid(equal_sign+1).trimmed();
            set_new_key_value(section+"/"+item,(value.isNull())?"":value);
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
        ret.append(key_value.section()+"/"+key_value.item());
    }
    return ret;
}

QStringList ConfReader::items(const QString & section) const {
    QStringList ret;
    for (int & index: index_of_section(section)) {
        ret.append(m_key_values[index].item());
    }
    return ret;
}

const QStringList ConfReader::CSVProcessor::parse(const QString & line) {
    CSVProcessor::state state = CSVProcessor::UnquotedField;
    QStringList fields;
    ulong i = 0;
    for (QChar c : line) {
        if (fields.isEmpty()) fields.append("");
        switch (state) {
            case CSVProcessor::UnquotedField:
                if (c == QLatin1Char(',')) { fields.append(""); i++; }
                else if (c == QLatin1Char('"')) state = CSVProcessor::QuotedField;
                else fields[i].append(c);
                break;
            case CSVProcessor::QuotedField:
                if (c == QLatin1Char('"')) state = CSVProcessor::QuotedQuote;
                else fields[i].append(c);
                break;
            case CSVProcessor::QuotedQuote:
                if (c == QLatin1Char(',')) { fields.append(""); i++; state = CSVProcessor::UnquotedField; }
                else if (c == QLatin1Char('"')) { fields[i].append('"'); state = CSVProcessor::QuotedField; }
                else state = CSVProcessor::UnquotedField;
                break;
        }
    }
    return fields;
}

const QString ConfReader::CSVProcessor::save(const QStringList & fields) {
    QString ret;
    bool quoted_quote;
    int index;
    QString field;

    for (int i=0;i<fields.count();i++) {
        field = fields.at(i);
        quoted_quote = false;
        index = 0;

        while ((index = field.indexOf(QLatin1Char('"'),index)) >= 0) {
            quoted_quote = true;
            field.insert(index+1,QLatin1Char('"'));
            index+=2;
        }
        if (field.contains(QLatin1Char(','))) quoted_quote = true;

        if (quoted_quote) field = QLatin1Char('"') + field + QLatin1Char('"');
        if ((i + 1) < fields.count()) field += QLatin1Char(',');

        ret += field;
    }
    return ret;
}
