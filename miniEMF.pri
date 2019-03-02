QT += xml
CONFIG += c++11

use_hmi {
    QT += widgets
    DEFINES += __USE_HMI__
}

SOURCES += \
    $$PWD/Model/MObject.cpp \
    $$PWD/Model/MObjectTypeFactory.cpp \
    $$PWD/Model/MObjectType.cpp \
    $$PWD/Model/Model.cpp \
    $$PWD/Model/PropertyFactory.cpp \
    $$PWD/Model/Property.cpp \
\
    $$PWD/Service/XMIService.cpp \
\
    $$PWD/Utils/XmiWriter.cpp


HEADERS += \
    $$PWD/Model/aliases.h \
    $$PWD/Model/MObject.h \
    $$PWD/Model/MObjectTypeFactory.h \
    $$PWD/Model/MObjectType.h \
    $$PWD/Model/Model.h \
    $$PWD/Model/PropertyFactory.h \
    $$PWD/Model/Property.h \
\
    $$PWD/Service/XMIService.h \
\
    $$PWD/Utils/PureStaticClass.h \
    $$PWD/Utils/Singleton.h \
    $$PWD/Utils/XmiWriter.h
