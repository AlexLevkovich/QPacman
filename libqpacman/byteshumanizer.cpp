#include "byteshumanizer.h"
#include <QStringList>
#include <QObject>

const QString BytesHumanizer::labels[9] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"};
const QString BytesHumanizer::labels_tr[9] = {QObject::tr("B"), QObject::tr("KiB"), QObject::tr("MiB"), QObject::tr("GiB"), QObject::tr("TiB"), QObject::tr("PiB"), QObject::tr("EiB"), QObject::tr("ZiB"), QObject::tr("YiB")};

BytesHumanizer::BytesHumanizer(const QString & val) {
    m_was_error = false;

    QString value = val.simplified();
    int index = -1;
    for (int i=8;i>=0;i--) {
        if (value.endsWith(" "+labels[i])) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        for (int i=8;i>=0;i--) {
            if (value.endsWith(labels[i])) {
                index = i;
                value = value.insert(value.length()-labels[i].length(),' ');
                break;
            }
        }
    }
    if (index == -1) index = 0;

    bool ok = false;
    m_value = value.split(" ",QString::SkipEmptyParts).at(0).toDouble(&ok);
    if (!ok) {
        m_value = 0.0;
        m_was_error = true;
        return;
    }

    if (index == 0) return;

    for (int i=0;i<index;i++) {
        m_value *= 1024.0;
    }
}

BytesHumanizer::BytesHumanizer(qreal value) {
    m_value = value;
    m_was_error = false;
}

QString BytesHumanizer::toString(int precision) const {
    qreal value = m_value;

    int index = 0;
    for(int i=0;i<8;i++) {
        if(value <= 2048.0 && value >= -2048.0) {
            break;
        }
        value /= 1024.0;
        index++;
    }

    return QString("%1 %2").arg(value,0,'f',precision).arg(labels_tr[index]);
}
