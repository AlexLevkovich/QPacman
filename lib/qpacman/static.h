#ifndef STATIC_H
#define STATIC_H

#include <QString>
#include <QSize>
#include <QSettings>

const QString fixButtonText(const QString & label);
const QSize quadroSize(int dimension);
void setupTranslations(const QString & mainName,const QString & installDir,const QString & mainLocalDir,const QString & alpmLocalDir,const QString & libLocalDir);

template<class ForwardIt, class T, class Compare> static ForwardIt binary_search_ex(ForwardIt first, ForwardIt last, const T& value, Compare comp) {
    ForwardIt it = std::lower_bound(first, last, value, comp);
    if ((it == last) || comp(value,*it)) it = last;
    return it;
}

template<class ForwardIt, class T> static ForwardIt binary_search_ex(ForwardIt first, ForwardIt last, const T& value) {
    ForwardIt it = std::lower_bound(first, last, value);
    if ((it == last) || (value < *it)) it = last;
    return it;
}

template<typename T> static T iniValue(const QString & key) {
    QSettings settings;
    return settings.value("settings/"+key).value<T>();
}
template<typename T> static T iniValue(const QString & key,const T & defValue) {
    QSettings settings;
    return settings.value("settings/"+key,(QVariant)defValue).value<T>();
}
void setIniValue(const QString & key,const QVariant & value);

#endif // STATIC_H
