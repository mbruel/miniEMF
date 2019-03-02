# Shall we use hmi?
CONFIG -= use_hmi

QT += core
use_hmi {
    QT += gui
}
else {
    QT -= gui
}

CONFIG += c++14 console
CONFIG -= app_bundle

#CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



# MiniEMF++ macro to remove (TO NOT TOUCH)
DEFINES -= __HIDE_ELEMENT_ON_DESTRUCTION__
DEFINES += __CASCADE_DELETION__



include(miniEMF.pri)
include(SimpleExample/model.pri)

SOURCES += \
        SimpleExample/main.cpp



TRANSLATIONS = SimpleExample/languages/example_en.ts SimpleExample/languages/example_fr.ts
