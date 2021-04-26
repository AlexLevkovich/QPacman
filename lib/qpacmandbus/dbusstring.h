#ifndef DBUSSTRING_H
#define DBUSSTRING_H

#include <QString>
#include <QDBusArgument>

class String {
public:
    String();
    String(const QString & str);

    operator QString() const;
private:
    QString str;
    bool is_null;

    friend QDBusArgument & operator<<(QDBusArgument &argument,const String & str);
    friend const QDBusArgument & operator>>(const QDBusArgument &argument,String & str);
};
Q_DECLARE_METATYPE(String)

#endif // DBUSSTRING_H
