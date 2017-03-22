/********************************************************************************
** Created by: Alex Levkovich (alevkovich@tut.by)
** License:    GPL
********************************************************************************/

#include "pacmanrepositoryreader.h"

PacmanRepositoryReader::PacmanRepositoryReader(QObject *parent) : PacmanProcessReader(parent) {
    m_package = new PacmanEntry();
}

PacmanRepositoryReader::~PacmanRepositoryReader() {
    delete m_package;
}

QString PacmanRepositoryReader::command() const {
    return QString("( %1 -Si;%1 -Qi ) | %2").arg(PACMAN_BIN).arg(CAT_BIN);
}

bool PacmanRepositoryReader::output(const QString & out) {
    if (m_package->parseLine(out.toLocal8Bit()) == PacmanEntry::EMPTY) {
        emit read_package(m_package);
        m_package = new PacmanEntry();
    }
    return true;
}
