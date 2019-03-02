//========================================================================
//
// Copyright (C) 2018 Matthieu Bruel <Matthieu.Bruel@gmail.com>
//
// This file is a part of miniEMF++ : https://github.com/mbruel/miniEMF
//
// miniEMF++ is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 3.0 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301,
// USA.
//
//========================================================================

#ifndef PROPERTYFACTORY_H
#define PROPERTYFACTORY_H

#include "Model/Property.h"
#include "Model/MObject.h"
#include <QMap>

class PropertyFactory
{
public:
    virtual void initStatics() = 0; // Ecore specific
    void initProperties(); // Ecore independant

    virtual ~PropertyFactory() = default;

protected:
    PropertyFactory() = default;

    // Fixed methods (independant of the Ecore Model in use)
    void initModelObjectProperties();
    void linkReverseProperties(LinkProperty *const linkProperty, LinkProperty *const reverseLinkProperty);

    template< typename TypeAttribute, template <typename> class AttributeProperty>
        AttributeProperty<TypeAttribute> *_create(QMap<QString, Property*> *propertyMap, const QString &name, const char *label, const TypeAttribute & defaultValue = TypeAttribute());
    template< typename PropertyType>     PropertyType     *_create(QMap<QString, Property*> *propertyMap, const QString &name, const char *label);
    template< typename LinkPropertyType> LinkPropertyType *_create(QMap<QString, Property*> *propertyMap, MObjectType *const eltType, MObjectType *const linkedEltType, const QString &name, const char *label, bool isMandatory, bool isSerializable = true);


    // Ecore specific methods (generated)
    virtual void linkAllReverseProperties() = 0;
    virtual void defineEnumPropertyValues() = 0;
    virtual void defineEcoreContainmentProperties() = 0;
    virtual void defineModelObjectTypeContainerProperties() = 0;
};


template< typename TypeAttribute, template< typename > class AttributeProperty>
AttributeProperty<TypeAttribute> *PropertyFactory::_create(QMap<QString, Property *> *propertyMap, const QString &name, const char *label, const TypeAttribute &defaultValue)
{
    AttributeProperty<TypeAttribute> *property = new AttributeProperty<TypeAttribute>(name, label, defaultValue);
    MObject::addPropertyToMap(propertyMap, property);
    return property;
}

template< typename PropertyType> PropertyType *PropertyFactory::_create(QMap<QString, Property *> *propertyMap, const QString &name, const char *label)
{
    PropertyType* property = new PropertyType(name, label);
    MObject::addPropertyToMap(propertyMap, property);
    return property;
}

template< typename LinkPropertyType> LinkPropertyType *PropertyFactory::_create(QMap<QString, Property *> *propertyMap, MObjectType * const eltType, MObjectType * const linkedEltType, const QString &name, const char *label, bool isMandatory, bool isSerializable)
{
    LinkPropertyType *property = new LinkPropertyType(eltType, linkedEltType, name, label, isMandatory, isSerializable);
    MObject::addPropertyToMap(propertyMap, property);
    return property;
}

#endif // PROPERTYFACTORY_H
