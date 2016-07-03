/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanrepositoryreader.h"

PacmanRepositoryReader::PacmanRepositoryReader(QObject *parent) : PacmanProcessReader(parent) {
}

QString PacmanRepositoryReader::command() const {
    return QString("%1/bash -c \"%1/pacman -Si;%1/pacman -Qi | %1/cat\"").arg(TOOLS_BIN);
}

bool PacmanRepositoryReader::output(const QString & out) {
    if (m_package.parseLine(out.toLocal8Bit()) == PacmanEntry::EMPTY) {
        emit read_package(m_package);
        m_package = PacmanEntry();
    }
    return true;
}

