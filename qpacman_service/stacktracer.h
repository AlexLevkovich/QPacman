/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
#ifndef STACKTRACER_H
#define STACKTRACER_H

#include <QString>

class StackTracer {
public:
    StackTracer(const QString & output);

private:
    static void crit_err_hdlr(int sig_num, void * info, void * ucontext);
    static QByteArray m_output;
};

#endif // STACKTRACER_H
