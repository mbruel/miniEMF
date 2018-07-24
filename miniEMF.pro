QT -= gui
QT += xml

CONFIG += c++11 console
CONFIG -= app_bundle

CONFIG += c++14
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

SOURCES += \
        main.cpp \
    Model/Element.cpp \
    Model/ElementType.cpp \
    Model/Model.cpp \
    Model/Property.cpp \
    Model/example/Person.cpp \
    Model/example/Meeting.cpp \
    Service/XMIService.cpp \
    Utils/XmiWriter.cpp \
    Model/example/PropertyFactoryMiniEMF.cpp \
    Model/example/ElementTypeFactoryMiniEMF.cpp \
    Model/ElementTypeFactory.cpp \
    Model/PropertyFactory.cpp \
    Model/example/Constant.cpp

HEADERS += \
    Model/Element.h \
    Model/ElementType.h \
    Model/Model.h \
    Model/Property.h \
    Model/example/Person.h \
    Utils/PureStaticClass.h \
    Model/example/Meeting.h \
    Service/XMIService.h \
    Utils/Singleton.h \
    Utils/XmiWriter.h \
    Model/example/ElementTypeFactoryMiniEMF.h \
    Model/example/PropertyFactoryMiniEMF.h \
    Model/ElementTypeFactory.h \
    Model/PropertyFactory.h \
    Model/example/allElementsInclude.h \
    Model/example/Constant.h


TRANSLATIONS = languages/example_en.ts languages/example_fr.ts
