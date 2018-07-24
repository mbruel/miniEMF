#ifndef ELEMENTTYPEFACTORYMINIEMF_H
#define ELEMENTTYPEFACTORYMINIEMF_H

#include "Model/ElementTypeFactory.h"
#include "Utils/Singleton.h"

class ElementType;


class ElementTypeFactoryMiniEMF : public ElementTypeFactory, public Singleton<ElementTypeFactoryMiniEMF>
{
    friend class Singleton<ElementTypeFactoryMiniEMF>;

public:
    QList<ElementType*> getRootElementTypes() override;
    void initStatics() override;

protected:
    void defineDerivedElementTypesFromEcore() override;

private:
    ElementTypeFactoryMiniEMF() = default;
    ~ElementTypeFactoryMiniEMF() = default;
};

#endif // ELEMENTTYPEFACTORYMINIEMF_H
