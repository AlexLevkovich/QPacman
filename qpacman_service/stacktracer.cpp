/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by) 2021
** License:    GPL
********************************************************************************/
//Thanks to https://stackoverflow.com/questions/77005/how-to-automatically-generate-a-stacktrace-when-my-program-crashes

#include "stacktracer.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <unistd.h>

typedef struct bfd bfd;
extern "C" char *bfd_demangle (bfd *, const char *, int);

typedef struct _sig_ucontext {
    unsigned long     uc_flags;
    struct ucontext   *uc_link;
    stack_t           uc_stack;
    struct sigcontext uc_mcontext;
    sigset_t          uc_sigmask;
} sig_ucontext_t;

QByteArray StackTracer::m_output;

StackTracer::StackTracer(const QString & output) {
    struct sigaction sigact;
    sigact.sa_sigaction = (void (*)(int,siginfo_t *,void *))crit_err_hdlr;
    sigact.sa_flags = SA_RESTART | SA_SIGINFO;
    if (sigaction(SIGSEGV, &sigact, (struct sigaction *)nullptr) != 0) {
        fprintf(stderr, "error setting SIGSEGV signal handler\n");
        ::exit(EXIT_FAILURE);
    }

    m_output = output.toLocal8Bit();
}

void StackTracer::crit_err_hdlr(int sig_num, void * _info, void * ucontext) {
    void *             array[50];
    void *             caller_address;
    char **            messages;
    int                size, i;
    sig_ucontext_t *   uc;
    char *             dem_str;
    siginfo_t *        info;

    uc = (sig_ucontext_t *)ucontext;
    info = (siginfo_t *)_info;

    /* Get the address at the time the signal was raised */
    #if defined(__i386__) // gcc specific
    caller_address = (void *) uc->uc_mcontext.eip; // EIP: x86 specific
    #elif defined(__x86_64__) // gcc specific
    caller_address = (void *) uc->uc_mcontext.rip; // RIP: x86_64 specific
    #else
    #error Unsupported architecture. // TODO: Add support for other arch.
    #endif

    fprintf(stderr, "\n");
    FILE * backtraceFile;

    backtraceFile = fopen(m_output.constData(),"w");

    if (sig_num == SIGSEGV)
        fprintf(backtraceFile, "signal %d (%s), address is %p from %p\n",sig_num, strsignal(sig_num), info->si_addr,(void *)caller_address);
    else
        fprintf(backtraceFile, "signal %d (%s)\n",sig_num, strsignal(sig_num));

    size = backtrace(array, 50);
    /* overwrite sigaction with caller's address */
    array[1] = caller_address;
    messages = backtrace_symbols(array, size);
    /* skip first stack frame (points here) */
    for (i = 1; i < size && messages != nullptr; ++i) {
        dem_str = bfd_demangle(nullptr,messages[i],3);
        fprintf(backtraceFile,"%s\n",dem_str);
        free(dem_str);
    }

    fclose(backtraceFile);
    free(messages);

    exit(EXIT_FAILURE);
}
