#-------------------------------------------------
#
# Project created by QtCreator 2016-07-19T14:47:30
#
#-------------------------------------------------

QT       += opengl

TARGET = QMeshLib
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    QMeshEdge.cpp \
    QMeshFace.cpp \
    QMeshNode.cpp \
    QMeshPatch.cpp \
    QMeshCluster.cpp \
    PolygenMesh.cpp

HEADERS += \
    QMeshEdge.h \
    QMeshFace.h \
    QMeshNode.h \
    QMeshPatch.h \
    PolygenMesh.h \
    QMeshCluster.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
