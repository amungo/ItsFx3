#-------------------------------------------------
#
# Project created by QtCreator 2015-10-28T11:02:57
#
#-------------------------------------------------

QT       += core gui widgets printsupport multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ItsFx3
TEMPLATE = app

unix:!macx: DEFINES += NO_CY_API
#DEFINES += NO_GPS

CONFIG += c++11
CONFIG += static
CONFIG += console

unix: QMAKE_CXXFLAGS += -std=c++11


SOURCES += main.cpp \
    hwmanager.cpp \
    itsmain.cpp \
    gui/qcustomplot.cpp \
    util/AttMap.cpp \
    rawsignalform.cpp \
    datahandlers/streamdumper.cpp \
    datastreams/streamrouter.cpp \
    util/Chan2bitParser.cpp \
    hwfx3/HexParser.cpp \
    hwfx3/fx3deverr.cpp \
    hwfx3/FX3Dev.cpp \
    hwfx3/fx3fwparser.cpp \
    dialogconfig.cpp \
    hwfx3/fx3devcyapi.cpp \
    gcacorr/dsp_utils.cpp \
    gcacorr/fftwrapper.cpp \
    gcacorr/gpsvis.cpp \
    gcacorr/matrixstatistic.cpp \
    gcacorr/rawsignal.cpp \
    gpscorrform.cpp \
    datahandlers/streamleapdumper.cpp \
    datahandlers/streamgapchecker.cpp \
    leap/leapconverter.cpp \
    hwfx3/FileSimDev.cpp \
    SpectrumForm.cpp \
    util/TimeComputator.cpp \
    hwfx3/ad9361/ad9361_tuner.cpp \
    hwfx3/ad9361/types/filters.cpp \
    gcacorr/lazy_matrix.cpp \
    phaseform.cpp \
    phasewidget.cpp \
    spectrumwidget.cpp \
    coverwidget.cpp \
    gcacorr/convresult.cpp \
    gcacorr/etalometrbase.cpp \
    gcacorr/etalometrgeo.cpp \
    convolutionwidget.cpp \
    videowidgetsurface.cpp \
    videowidget.cpp \
    gcacorr/point3d_t.cpp \
    datastreams/singleevent.cpp \
    gcacorr/etalometrfile.cpp

HEADERS  += itsmain.h \
            hwfx3/FX3Dev.h \
            hwmanager.h \
    gui/qcustomplot.h \
    util/AttMap.h \
    rawsignalform.h \
    datahandlers/streamdumper.h \
    datastreams/streamdatahandler.h \
    datastreams/streamrouter.h \
    util/Chan2bitParser.h \
    util/convoluter.h \
    util/mathTypes.h \
    util/cas_codes.h \
    hwfx3/HexParser.h \
    hwfx3/fx3devifce.h \
    hwfx3/fx3deverr.h \
    hwfx3/fx3devdebuginfo.h \
    hwfx3/fx3fwparser.h \
    hwfx3/fx3devdrvtype.h \
    dialogconfig.h \
    hwfx3/fx3config.h \
    hwfx3/fx3devcyapi.h \
    hwfx3/cy_inc/CyAPI.h \
    hwfx3/cy_inc/cyioctl.h \
    hwfx3/cy_inc/CyUSB30_def.h \
    hwfx3/cy_inc/usb100.h \
    hwfx3/cy_inc/usb200.h \
    hwfx3/cy_inc/UsbdStatus.h \
    hwfx3/cy_inc/VersionNo.h \
    hwfx3/libusb.h \
    fftw_inc/fftw3.h \
    gcacorr/cas_codes.h \
    gcacorr/dsp_utils.h \
    gcacorr/fftwrapper.h \
    gcacorr/fir_filter.h \
    gcacorr/gpsvis.h \
    gcacorr/mathTypes.h \
    gcacorr/matrixstatistic.h \
    gcacorr/rawsignal.h \
    gcacorr/stattype.h \
    gpscorrform.h \
    datahandlers/streamleapdumper.h \
    datahandlers/streamgapchecker.h \
    leap/leapconverter.h \
    hwfx3/FileSimDev.h \
    SpectrumForm.h \
    gcacorr/averagervector.h \
    util/TimeComputator.h \
    hwfx3/ad9361/ad9361_defines.h \
    hwfx3/ad9361/ad9361_tuner.h \
    hwfx3/devioifce.h \
    hwfx3/ad9361/types/filters.h \
    hwfx3/ad9361/ad9361_client.h \
    hwfx3/ad9361/ad9361_filter_taps.h \
    hwfx3/ad9361/ad9361_gain_tables.h \
    hwfx3/ad9361/ad9361_synth_lut.h \
    datahandlers/filedumpcallbackifce.h \
    gcacorr/lazy_matrix.h \
    phaseform.h \
    phasewidget.h \
    spectrumwidget.h \
    coverwidget.h \
    gcacorr/convresult.h \
    gcacorr/etalometrbase.h \
    gcacorr/etalometrgeo.h \
    convolutionwidget.h \
    videowidgetsurface.h \
    videowidget.h \
    gcacorr/point3d_t.h \
    datastreams/singleevent.h \
    gcacorr/etalometrfile.h \
    hwfx3/fx3commands.h

FORMS    += itsmain.ui \
    rawsignalform.ui \
    dialogconfig.ui \
    gpscorrform.ui \
    SpectrumForm.ui \
    phaseform.ui

INCLUDEPATH += $$PWD/fftw_inc

win32: LIBS += -luser32
win32: LIBS += -lsetupapi
win32: QMAKE_LFLAGS += /NODEFAULTLIB:LIBCMT

win32:!win32-g++: LIBS += -L$$PWD/libs/libusb/MS32/static/ -llibusb-1.0
else:win32-g++:   LIBS += -L$$PWD/libs/libusb/MinGW32/static/ -lusb-1.0

win32:!win32-g++: INCLUDEPATH += $$PWD/libs/libusb/MS32/static
else:win32-g++:   INCLUDEPATH += $$PWD/libs/libusb/MinGW32/static

win32:!win32-g++: DEPENDPATH += $$PWD/libs/libusb/MS32/static
else:win32-g++:   DEPENDPATH += $$PWD/libs/libusb/MinGW32/static

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/libs/libusb/MS32/static/libusb-1.0.lib
else:win32-g++:   PRE_TARGETDEPS += $$PWD/libs/libusb/MinGW32/static/libusb-1.0.a



win32: LIBS += -L$$PWD/libs/cyapi/x86/ -lCyAPI

win32: INCLUDEPATH += $$PWD/libs/cyapi/x86
win32: DEPENDPATH += $$PWD/libs/cyapi/x86

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/libs/cyapi/x86/CyAPI.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/libs/cyapi/x86/libCyAPI.a



win32: LIBS += -L$$PWD/libs/libfftw/ -llibfftw3f-3

win32: INCLUDEPATH += $$PWD/libs/libfftw
win32: DEPENDPATH += $$PWD/libs/libfftw

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/libs/libfftw/libfftw3f-3.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/libs/libfftw/liblibfftw3f-3.a

unix:!macx: LIBS += -lusb-1.0

unix:!macx: LIBS += -lfftw3f
