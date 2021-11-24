#ifndef CONFREADER_H
#define CONFREADER_H

#include <QString>
#include <QList>

class QFile;

class ConfReader {
public:
    ConfReader(const QString & filename);
    ~ConfReader();

    QString value(const QString &key, const QString & defaultValue) const;
    QString value(const QString &key, const char * defaultValue) const;
    int value(const QString &key, int defaultValue) const;
    bool value(const QString &key, bool defaultValue) const;
    bool setValue(const QString &key, const QString &value);
    bool setValue(const QString &key, const char * value);
    bool setValue(const QString &key, int value);
    bool setValue(const QString &key, bool value);
    void remove(const QString &key);
    void clear();
    bool sync();
    QString lastError() const;
    QStringList sections() const;
    QStringList allKeys() const;
    QStringList items(const QString & section) const;
    QString fileName() const;

private:
    void sort();
    bool parse();
    int index_of_key(const QString &key) const;
    int index_of_key(const QString &key,int & sorted_index) const;
    QList<int> index_of_section(const QString &section) const;
    QList<int> index_of_section(const QString &section,QList<int> & sorted_indexes) const;
    void set_new_key_value(const QString & key,const QString & value);
    bool read_conf_line(QFile * file,QString & line,int & begin,int & end);

    static const QString section_from_key(const QString &key);
    static const QString item_from_key(const QString &key);

    class KeyValue {
    public:
        KeyValue(const QString &key) {
            section = section_from_key(key);
            item = item_from_key(key);
            index = -1;
        }

        QString section;
        QString item;
        QString value;
        int index;

        bool isUnusable() const { return index == -1; }
        void setUnusable() { index = -1; }
        bool isComment() { return (section.isEmpty() && item.isEmpty()); }
        bool isOrphan() { return (section.isEmpty() && !item.isEmpty()); }
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
