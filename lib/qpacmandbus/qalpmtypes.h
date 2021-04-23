/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef QALPMTYPES_H
#define QALPMTYPES_H

typedef QMap<QString,QString> StringStringMap;
struct ThreadRun {
    enum RC {
        OK = 0,
        BAD = 1,
        TERMINATED = 2,
        FORBIDDEN = 3,
        ROOTPW = 4
    };
};
Q_DECLARE_METATYPE(ThreadRun::RC)

#endif // QALPMTYPES_H
