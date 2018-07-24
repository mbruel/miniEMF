#ifndef PROPERTYFACTORYMINIEMF_H
#define PROPERTYFACTORYMINIEMF_H

#include "Model/PropertyFactory.h"
#include "Utils/Singleton.h"


class PropertyFactoryMiniEMF : public PropertyFactory, public Singleton<PropertyFactoryMiniEMF>
{
    friend class Singleton<PropertyFactoryMiniEMF>;

public:
    void initStatics() override; // Ecore specific

protected:
    // Ecore specific methods (generated)
    void linkAllReverseProperties() override;
    void defineEnumPropertyValues() override;
    void defineEcoreContainmentProperties() override;
    void defineElementTypeContainerProperties() override;

private:
    PropertyFactoryMiniEMF() = default;
    ~PropertyFactoryMiniEMF() = default;
};


#endif // PROPERTYFACTORYMINIEMF_H
