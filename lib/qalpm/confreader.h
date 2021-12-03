/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/

#ifndef CONFREADER_H
#define CONFREADER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QVariant>

class QFile;

class ConfReader {
public:
    ConfReader(const QString & filename);
    ConfReader();
    ~ConfReader();

private:
    class CSVProcessor {
    public:
        static const QStringList parse(const QString & line);
        static const QString save(const QStringList & fields);
    private:
        enum state {
            UnquotedField,
            QuotedField,
            QuotedQuote
        };
    };

    template <typename T> QString template_type_to_string(const T & item) const {
        QVariant value(item);
        if ((QMetaType::Type)value.type() == QMetaType::QStringList) {
            return CSVProcessor::save(value.value<QStringList>());
        }
        else if ((QMetaType::Type)value.type() == QMetaType::Bool) {
            return QString("%1").arg(value.value<bool>()?1:0);
        }
        else if (value.canConvert<QString>()) {
            return value.toString();
        }
        return QString();
    }

    template <typename T> QStringList template_list_to_stringlist(const QList<T> & list) const {
        QStringList ret;
        for (const T & item: list) {
            ret.append(template_type_to_string(item));
        }
        return ret;
    }

    template <typename T> T string_to_template_type(const QString & str) const {
        if ((QMetaType::Type)QVariant(T()).type() == QMetaType::QStringList) {
            return QVariant(CSVProcessor::parse(str)).value<T>();
        }
        else if ((QMetaType::Type)QVariant(T()).type() == QMetaType::Bool) {
            return QVariant(str.toInt()).value<T>();
        }
        return QVariant(str).value<T>();
    }

    template <typename T> QList<T> stringlist_to_template_list(const QStringList & list) const {
        QList<T> ret;
        for (const QString & item: list) {
            ret.append(string_to_template_type<T>(item));
        }
        return ret;
    }

public:
    template <typename T> QList<T> values(const QString &key) const {
        int index = index_of_key(key);
        if (index == -1) return QList<T>();

        QStringList values = m_key_values[index].values();
        if (values.isEmpty()) return QList<T>();

        return stringlist_to_template_list<T>(values);
    }

    template <typename T> QList<T> values(const QString &key, const T & defaultValue) const {
        int index = index_of_key(key);
        if (index == -1) return (QList<T>() << defaultValue);

        QStringList values = m_key_values[index].values();
        if (values.isEmpty()) return (QList<T>() << defaultValue);

        return stringlist_to_template_list<T>(values);
    }

    template <typename T> T value(const QString &key, const T & defaultValue) const {
        return values(key,defaultValue).at(0);
    }

    template <typename T> bool addValue(const QString &key, const T &value) {
        m_error.clear();
        QString section = section_from_key(key);
        if (section.isEmpty() && item_from_key(key).isEmpty()) {
            m_error = "ConfReader::setValue: " + QObject::tr("wrong input key!");
            return false;
        }

        QString key2 = key;
        if (section.isEmpty() && !key2.startsWith("/")) key2 = "/" + key2;

        int sorted_index;
        int index = index_of_key(key2,sorted_index);
        if (index == -1) {
            set_new_key_value(key2,template_type_to_string(value));
            m_sync_is_needed = true;
            m_sections.append(section);
            m_sections.removeDuplicates();
            return true;
        }

        m_key_values[index].addValue(template_type_to_string(value));
        m_sync_is_needed = true;
        return true;
    }

    template <typename T> bool addValues(const QString &key, const QList<T> &values) {
        m_error.clear();
        QString section = section_from_key(key);
        if (section.isEmpty() && item_from_key(key).isEmpty()) {
            m_error = "ConfReader::setValue: " + QObject::tr("wrong input key!");
            return false;
        }

        QString key2 = key;
        if (section.isEmpty() && !key2.startsWith("/")) key2 = "/" + key2;

        int sorted_index;
        int index = index_of_key(key2,sorted_index);
        if (index == -1) {
            set_new_key_values(key2,template_list_to_stringlist(values));
            m_sync_is_needed = true;
            m_sections.append(section);
            m_sections.removeDuplicates();
            return true;
        }

        m_key_values[index].addValues(template_list_to_stringlist(values));
        m_sync_is_needed = true;
        return true;
    }

    template <typename T> bool setValues(const QString &key, const QList<T> &values) {
        m_error.clear();
        QString section = section_from_key(key);
        if (section.isEmpty() && item_from_key(key).isEmpty()) {
            m_error = "ConfReader::setValue: " + QObject::tr("wrong input key!");
            return false;
        }

        QString key2 = key;
        if (section.isEmpty() && !key2.startsWith("/")) key2 = "/" + key2;

        int sorted_index;
        int index = index_of_key(key2,sorted_index);
        if (index == -1) {
            set_new_key_values(key2,template_list_to_stringlist(values));
            m_sync_is_needed = true;
            m_sections.append(section);
            m_sections.removeDuplicates();
            return true;
        }

        m_key_values[index].setValues(template_list_to_stringlist(values));
        m_sync_is_needed = true;
        return true;
    }

    void setFileName(const QString & fileName);
    void remove(const QString &key);
    void clear();
    bool read();
    bool sync();
    QString lastError() const;
    QStringList sections() const;
    bool keyExists(const QString &key) const;
    QStringList allKeys() const;
    QStringList items(const QString & section) const;
    QString fileName() const;

private:
    void sort();
    int index_of_key(const QString &key) const;
    int index_of_key(const QString &key,int & sorted_index) const;
    QList<int> index_of_section(const QString &section) const;
    QList<int> index_of_section(const QString &section,QList<int> & sorted_indexes) const;
    void set_new_key_value(const QString & key,const QString & value);
    void set_new_key_values(const QString & key,const QStringList & values);
    bool read_conf_line(QFile * file,QString & line,int & begin,int & end);
    QByteArray write_value(const QString & item,const QStringList &values) const;

    static const QString section_from_key(const QString &key);
    static const QString item_from_key(const QString &key);

    class KeyValue {
    public:
        KeyValue(const QString &key,int index = -1) {
            m_section = section_from_key(key);
            m_item = item_from_key(key);
            m_index = index;
        }

        QString key() const {
            if (m_section.isEmpty() && m_item.isEmpty()) return QString("");
            return m_section + "/" + m_item;
        }
        QString section() const { return m_section; }
        QString item() const { return m_item; }
        QStringList values() const { return m_values; }
        int index() const { return m_index; }

        void setValues(const QStringList & values) { m_values = values; }
        void addValue(const QString & value) { m_values.append(value); }
        void addValues(const QStringList & values) { m_values.append(values); }

        bool isUnusable() const { return m_index == -1; }
        void setUnusable() { m_index = -1; }
        bool isComment() { return (m_section.isEmpty() && m_item.isEmpty()); }
        bool isOrphan() { return (m_section.isEmpty() && !m_item.isEmpty()); }

    private:
        QString m_section;
        QString m_item;
        QStringList m_values;
        int m_index;
    };

    static bool only_key_cmp(const KeyValue & item1, const KeyValue & item2);
    static bool only_section_cmp(const KeyValue & item1, const KeyValue & item2);

    QList<KeyValue> m_key_values;
    QList<KeyValue> m_sorted_key_values;
    QString m_file_name;
    bool m_sync_is_needed;
    bool m_sort_is_needed;
    QString m_error;
    QStringList m_sections;
};

#endif // CONFREADER_H
