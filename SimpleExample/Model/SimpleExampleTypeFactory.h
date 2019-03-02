#ifndef MObjectTypeFACTORYMINIEMF_H
#define MObjectTypeFACTORYMINIEMF_H

#include "Model/MObjectTypeFactory.h"
#include "Utils/Singleton.h"

class MObjectType;


class SimpleExampleTypeFactory : public MObjectTypeFactory, public Singleton<SimpleExampleTypeFactory>
{
    friend class Singleton<SimpleExampleTypeFactory>;

public:
    QList<MObjectType*> getRootModelObjectTypes() override;
    void initStatics() override;

protected:
    void defineDerivedModelObjectTypesFromEcore() override;

public:
    SimpleExampleTypeFactory()  = default;
    ~SimpleExampleTypeFactory() = default;
};

#endif // MObjectTypeFACTORYMINIEMF_H
