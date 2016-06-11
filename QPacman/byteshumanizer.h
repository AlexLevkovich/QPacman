/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#ifndef BYTESHUMANIZER_H
#define BYTESHUMANIZER_H

#include <QString>

class BytesHumanizer {
public:
    BytesHumanizer(const QString & value);
    BytesHumanizer(double value);
    bool was_error() { return m_was_error; }
    double value() { return m_value; }
    QString toString(int precision = 2) const;

private:
    static const QString labels[9];
    double m_value;
    bool m_was_error;
};

#endif // BYTESHUMANIZER_H
