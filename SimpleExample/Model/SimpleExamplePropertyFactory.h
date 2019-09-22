#ifndef PROPERTYFACTORYMINIEMF_H
#define PROPERTYFACTORYMINIEMF_H

#include "Model/PropertyFactory.h"
#include "Utils/Singleton.h"


class SimpleExamplePropertyFactory : public PropertyFactory, public Singleton<SimpleExamplePropertyFactory>
{
    friend class Singleton<SimpleExamplePropertyFactory>; // to access constructor

public:
    void initStatics() override; // Ecore specific

protected:
    // Ecore specific methods (generated)
    void linkAllReverseProperties() override;
    void defineEnumPropertyValues() override;
    void defineEcoreContainmentProperties() override;
    void defineModelObjectTypeContainerProperties() override;
    void defineMapPropertiesKey() override;

private:
    SimpleExamplePropertyFactory() = default;
    ~SimpleExamplePropertyFactory() = default;
};


#endif // PROPERTYFACTORYMINIEMF_H
