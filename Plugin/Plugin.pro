TEMPLATE = lib

QMAKE_CC= gcc -std=c99

SOURCES += \
    biquad_filter.c \
    feedback_suppressor.c \
    feedback_suppressor_ui.c

OTHER_FILES += \
    feedback_suppressor.lv2/feedback_suppressor.ttl \
    feedback_suppressor.lv2/manifest.ttl

HEADERS += \
    biquad_filter.h \
    hann_window.h \
    uris.h

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += fftw3 gtk+-2.0
