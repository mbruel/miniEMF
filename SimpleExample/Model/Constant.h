#ifndef CONSTANT_H
#define CONSTANT_H

#include <QString>
#include "Utils/PureStaticClass.h"


class Constant : public PureStaticClass
{
public:
    // /miniExample/SEX
    static const char *C_Male;
    static const char *C_Female;

    QString tr(const QString &txt) const;
};

class Enum : public PureStaticClass
{
public:
    enum class SEX {
        Male = 0,
        Female = 1,
    };

    enum class SEX2 {
        Male = 10,
        Female = 11,
    };
};


#endif // CONSTANT_H
