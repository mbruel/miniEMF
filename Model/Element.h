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


#ifndef ELEMENT_H_
#define ELEMENT_H_

// Activate this if you wish to use generated Property Editors with their Validator
// (needs to add gui or widgets in the .pro)
//#define __USE_HMI__ 1

// You can desactivate the hidding on deletion in case of cascade deletion
#define __HIDE_ELEMENT_ON_DESTRUCTION__ 1

#include "ElementType.h"

#include <QSet>
#include <QList>
#include <QMap>
#include <QMultiMap>
#include <QVariant>


class Property;
class EnumProperty;
template<typename TypeAttribute> class AttributeProperty;
using BoolProperty     = AttributeProperty<bool>;
using IntProperty      = AttributeProperty<int>;
using FloatProperty    = AttributeProperty<float>;
using DoubleProperty   = AttributeProperty<double>;
using StringProperty   = AttributeProperty<QString>;
using DateTimeProperty = AttributeProperty<QDateTime>;

class uIntProperty;
class uDoubleProperty;

class LinkToOneProperty;
using Link01Property = LinkToOneProperty;
class Link11Property;

class LinkProperty;
template <template <typename...> class Container, typename... Args> class GenericLinkToManyProperty;
using LinkToManyProperty        = GenericLinkToManyProperty<QSet>;
using Link0NProperty            = LinkToManyProperty;
using OrderedLinkToManyProperty = GenericLinkToManyProperty<QList>;
using OrderedLink0NProperty     = GenericLinkToManyProperty<QList>;
using OrderedLink1NProperty     = GenericLinkToManyProperty<QList>;
using MapLinkProperty           = GenericLinkToManyProperty<QMap, QVariant>;
using MultiMapLinkProperty      = GenericLinkToManyProperty<QMultiMap, QVariant>;

using ElemId       = QString;
using ElemSet      = QSet<Element*>;
using ElemList     = QList<Element*>;
using ElemMap      = QMap<QVariant, Element*>;
using ElemMultiMap = QMultiMap<QVariant, Element*>;


class XmiWriter;
class Model;


class Element
{
    // Properties are Friend classes thus they can modify the _propertyValueMap using the corresponding adapted method to their type
    // (setPropertyValueFromQVariant, setPropertyValueFromElement, ...
    // This is useful when using a property editor so the property can directly update the value of an Element
    friend class Property;
    template<typename TypeAttribute> friend class AttributeProperty;
    friend class EnumProperty;
    friend class LinkToOneProperty;
    friend class Link11Property;
    template <template <typename...> class Container, typename... Args> friend class GenericLinkToManyProperty;

    friend class Model; // to be able to change the state of the Element


    enum class STATE
    {
        CREATED,
        ADDED_IN_MODEL,
        REMOVED_FROM_MODEL,
        CLONE
    };

private:
    ElemId _id;
    STATE  _state;

protected:
    QMap<Property*, QVariant> _propertyValueMap;

public:
    static ElementType*    TYPE;
    static StringProperty* PROPERTY_NAME;
    static StringProperty* PROPERTY_DESCRIPTION;

    static QMap<QString, Property*> * initPropertyMap();
    static void addPropertyToMap(QMap<QString, Property*>* propertyMap, Property* property);

    static Element *createElement(){return nullptr;}


public:
    virtual ~Element();

    Element(const Element& other) = delete;
    Element(Element&& other) = delete;

    Element & operator=(const Element& other) = delete;
    Element & operator=(Element&& other) = delete;

    Element *clone();

    Element *shallowCopy();
    void copyPropertiesFromSourceElementWithCloneElements(Element *srcElem, Model *clonedModel);


    inline ElemId getId() const;
    inline void setId(const ElemId &id);


    // #####################
    // #### ElementType ####

    virtual ElementType* getElementType() const = 0;

    inline int getElementTypeId() const;
    inline const QString getElementTypeName()  const;
    inline const QString getElementTypeLabel() const;

    ElementType* getLinkedElementType(LinkProperty* linkProperty) const;



    // ####################
    // #### PROPERTIES ####

    QString getDescription();
    void setDescription(const QString &value);

    QString getName();
    void setName(const QString &name);

    virtual QString getDefaultName();



    inline QList<Property*> getPropertyList() const ;
    QSet<LinkProperty*> getLinkProperties();

    void hideFromLinkedElements();
    void makeVisibleForLinkedElements();

    inline QVariant getPropertyVariant(Property *property) const;
    virtual inline Element *getEcoreContainer();


    inline bool operator<(Element & elt);
    inline static bool elementNameInsensitiveLessThan(Element *elt1, Element *elt2);
    inline static bool elementIdLessThan(Element *elt1, Element *elt2);

    void serialize(XmiWriter *xmiWriter, const QString &tagName, const QString &xmiType = "");


    void exportWithLinksAsNewModelSharingSameElements(Model *refModel, Model *subModel);//!< use to do some xml exports of the element with all the links needed

//    Property *getPropertyByName(QString propertyName);

//    Element *getEcoreContainer();
//    virtual void getAllDependantElements(QList<Element*>& allDependantElements, QSet<Element *> &inspectedElements);
//    void getEcoreConsistencyDependantElements(QList<Element *> &allDependantElements, QSet<Element*>& dependantElements);
//    virtual void getBusinessRuleDependantElements(QSet<Element*>& dependantElements);


protected:
    Element(QMap<QString, Property*>* classPropertyMap);
    bool isInModel() const {return _state == STATE::ADDED_IN_MODEL;}

private:

    void _initPropertyValueMap(QMap<QString, Property *> *classPropertyMap);

    virtual QVariant getPropertyMapKey(Property *mapProperty); //!< for non use of that function, we return the name by default


    // Those methods are shared with the Property classes
    template<typename TypeAttribute> TypeAttribute getPropertyValue(AttributeProperty<TypeAttribute> *property);
    template<typename ReturnTypeLinkProperty> ReturnTypeLinkProperty *getLinkPropertyValue(Property *property);

    void setPropertyValueFromQVariant(Property *property, const QVariant &value);
    void setPropertyValueFromElement(LinkProperty *property, Element *value);
    template<typename ReturnTypeLinkProperty> void setLinkToManyPropertyValue(Property *property, ReturnTypeLinkProperty *value);


    // Those 2 functions will be specialized for each type but the template will be used by GenericLinkToManyProperty<Container, Args...>::addLink
    template <template <typename...> class Container, typename... Args> void addALinkToMany(Property *property, Element *value);
    template <template <typename...> class Container, typename... Args> void removeALinkFromMany(Property *property, Element *value);
};

Q_DECLARE_METATYPE( ElementType* )
Q_DECLARE_METATYPE( Element* )
Q_DECLARE_METATYPE( ElemSet* )
Q_DECLARE_METATYPE( ElemList* )
Q_DECLARE_METATYPE( ElemMap* )
Q_DECLARE_METATYPE( ElemMultiMap* )


////////////////////////////////
/// inline functions definition
////////////////////////////////
int Element::getElementTypeId() const { return getElementType()->getId(); }
const QString Element::getElementTypeName() const { return getElementType()->getName(); }
const QString Element::getElementTypeLabel() const { return getElementType()->getLabel(); }

ElemId Element::getId() const { return _id; }
void Element::setId(const ElemId &id) { _id = id; }

QList<Property *> Element::getPropertyList() const { return _propertyValueMap.keys(); }

QVariant Element::getPropertyVariant(Property *property) const { return _propertyValueMap.value(property, QVariant()); }
Element *Element::getEcoreContainer() { return nullptr; }

bool Element::operator<(Element& elt){ return getId() < elt.getId(); }
bool Element::elementNameInsensitiveLessThan(Element *elt1, Element *elt2) {return  elt1->getName().toLower() < elt2->getName().toLower();}
bool Element::elementIdLessThan(Element *elt1, Element *elt2) { return elt1->getId() < elt2->getId();}


////////////////////////////////////////////
/// Template functions definition (generic)
////////////////////////////////////////////

template<typename ReturnTypeLinkProperty>
void Element::setLinkToManyPropertyValue(Property *property, ReturnTypeLinkProperty *value)
{
    ReturnTypeLinkProperty *propertyValues = getLinkPropertyValue<ReturnTypeLinkProperty>(property);
    propertyValues->swap(*value);
}

template<typename ReturnTypeLinkProperty>
ReturnTypeLinkProperty *Element::getLinkPropertyValue(Property *property)
{
    QVariant &variant = _propertyValueMap[property];
    return variant.value<ReturnTypeLinkProperty*>();
}


template<typename TypeAttribute>
TypeAttribute Element::getPropertyValue(AttributeProperty<TypeAttribute> *property)
{
    QVariant &variant = _propertyValueMap[property];
    return variant.value<TypeAttribute>();
}



/////////////////////////////////////////////////
/// Template functions specializations (per type)
/////////////////////////////////////////////////

// Template specializations of addALinkToMany for QSet, QList, QMap and QMultiMap
template <> inline void Element::addALinkToMany<QSet>(Property *property, Element *value)
{
    if (value)
    {
        ElemSet *propertyValues = getLinkPropertyValue<ElemSet>(property);
        propertyValues->insert(value);
    }
}
template <> inline void Element::addALinkToMany<QList>(Property *property, Element *value)
{
    if (value)
    {
        ElemList *propertyValues = getLinkPropertyValue<ElemList>(property);
//        if (!propertyValues->contains(value))
            propertyValues->append(value);
    }
}
template <> inline void Element::addALinkToMany<QMap, QVariant>(Property *property, Element *value)
{
    if (value)
    {
        ElemMap *propertyValues = getLinkPropertyValue<ElemMap>(property);
        QVariant key = value->getPropertyMapKey(property);
        propertyValues->insert(key, value);
    }
}
template <> inline void Element::addALinkToMany<QMultiMap, QVariant>(Property *property, Element *value)
{
    if (value)
    {
        ElemMultiMap *propertyValues = getLinkPropertyValue<ElemMultiMap>(property);
        QVariant key = value->getPropertyMapKey(property);
        propertyValues->insert(key, value);
    }
}

// Template specializations of removeALinkFromMany2 for QSet, QList, QMap and QMultiMap
template <> inline void Element::removeALinkFromMany<QSet>(Property *property, Element *value)
{
    if (value)
    {
        ElemSet *propertyValues = getLinkPropertyValue<ElemSet>(property);
        propertyValues->remove(value);
    }
}
template <> inline void Element::removeALinkFromMany<QList>(Property *property, Element *value)
{
    if (value)
    {
        ElemList *propertyValues = getLinkPropertyValue<ElemList>(property);
        propertyValues->removeOne(value);
    }
}
template <> inline void Element::removeALinkFromMany<QMap, QVariant>(Property *property, Element *value)
{
    if (value)
    {
        ElemMap *propertyValues = getLinkPropertyValue<ElemMap>(property);
        QVariant key = value->getPropertyMapKey(property);
        propertyValues->remove(key);
    }
}
template <> inline void Element::removeALinkFromMany<QMultiMap, QVariant>(Property *property, Element *value)
{
    if (value)
    {
        ElemMultiMap *propertyValues = getLinkPropertyValue<ElemMultiMap>(property);
        QVariant key = value->getPropertyMapKey(property);
        propertyValues->remove(key, value); // remove only the couple (key, value)
    }
}



#endif /* ELEMENT_H_ */
