#include "dbusstring.h"

String::String() {
    is_null = true;
}
String::String(const QString & str) {
    is_null = str.isNull();
    this->str = str;
}

String::operator QString() const {
    return is_null?QString():str;
}

QDBusArgument & operator<<(QDBusArgument &argument,const String & str)  {
    argument.beginStructure();
    argument << str.str;
    argument << str.is_null;
    argument.endStructure();
    return argument;
}

const QDBusArgument & operator>>(const QDBusArgument &argument,String & str) {
    argument.beginStructure();
    argument >> str.str;
    argument >> str.is_null;
    argument.endStructure();
    return argument;
}
