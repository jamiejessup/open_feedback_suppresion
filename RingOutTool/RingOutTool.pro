#-------------------------------------------------
#
# Project created by QtCreator 2013-07-22T19:06:09
#
#-------------------------------------------------

QT       += core gui

TARGET = ofs_ring_out_tool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    biquad_filter.c \
    jack.cpp \
    qcustomplot.cpp \
    hannwindow.cpp \

HEADERS  += mainwindow.h \
    biquad_filter.h \
    jack.h \
    qcustomplot.h \
    rngen.h \
    prbsgen.h \
    hannwindow.h \

FORMS    += mainwindow.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += jack

unix:!macx:!symbian: LIBS += -lfftw3
