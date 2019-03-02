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


#ifndef MOBJECT_H_
#define MOBJECT_H_

#include "aliases.h"
#include "MObjectType.h"
#include <QSet>
#include <QList>
#include <QMap>
#include <QMultiMap>



class XmiWriter;
class Model;

class MObject
{
    // Properties are Friend classes thus they can modify the _propertyValueMap using the corresponding adapted method to their type
    // (setPropertyValueFromQVariant, setPropertyValueFromElement, ...
    // This is useful when using a property editor so the property can directly update the value of an MObject
    friend class Property;
    template<typename TypeAttribute> friend class AttributeProperty;
    template<typename TypeAttribute> friend class AttributeListProperty;
    friend class EnumProperty;
    friend class LinkToOneProperty;
    template <template <typename...> class Container, typename... Args> friend class GenericLinkToManyProperty;

    friend class Model; // to be able to change the state of the MObject


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
    bool   _isReadOnly;
    bool   _isNameReadOnly;


protected:
    QMap<Property*, QVariant> _propertyValueMap;

public:
    static MObjectType*    TYPE;
    static StringProperty* PROPERTY_NAME;
//    static StringProperty* PROPERTY_DESCRIPTION;

    static QMap<QString, Property*> * initPropertyMap();
    static void addPropertyToMap(QMap<QString, Property*>* propertyMap, Property* property);

    static MObject *createModelObject(){return nullptr;}


public:
    virtual ~MObject();

    MObject(const MObject& other) = delete;
    MObject(MObject&& other) = delete;

    MObject & operator=(const MObject& other) = delete;
    MObject & operator=(MObject&& other) = delete;

    MObject *clone(MObject *ecoreContainer, uint modelId, bool sameId);

    MObject *shallowCopy();
    void copyPropertiesFromSourceElementWithCloneElements(MObject *srcElem, Model *clonedModel);


    inline ElemId getId() const;
    inline void setId(const ElemId &id);

    inline bool isInModel() const;

    inline bool isReadOnly() const;
    inline void setReadOnly(bool readOnly);

    inline bool isNameReadOnly() const;
    inline void setNameReadOnly(bool readOnly);

    inline bool isA(MObjectType *type) const;

    virtual void initDefaultProperties() {}

    // #####################
    // #### MObjectType ####

    virtual MObjectType* getModelObjectType() const = 0;

    inline int getModelObjectTypeId() const;
    inline const QString getModelObjectTypeName()  const;
    inline const QString getModelObjectTypeLabel() const;

    MObjectType* getLinkedModelObjectType(LinkProperty* linkProperty) const;



    // ####################
    // #### PROPERTIES ####


    inline QVariant toVariant();

//    QString getDescription();
//    void setDescription(const QString &value);

    QString getName() const;
    void setName(const QString &name);

    virtual QString getDefaultName();

    void validateLinkProperties(QStringList &ecoreErrors);
    virtual bool validateBusinessRules(QStringList &businessErrors, MObject *owner = nullptr);


    inline QList<Property*> getPropertyList() const ;
    QSet<LinkProperty*> getLinkProperties();
    QMap<QString, LinkProperty *> getContainmentProperties() const;
    QMap<QString, Property*> getNonContainmentProperties() const;



    void hideFromLinkedModelObjects();
    void makeVisibleForLinkedModelObjects();

    inline QVariant getPropertyVariant(Property *property) const;
    virtual inline MObject *getEcoreContainer();


    inline bool operator<(MObject & elt);
    inline static bool elementNameInsensitiveLessThan(MObject *elt1, MObject *elt2);
    inline static bool elementIdLessThan(MObject *elt1, MObject *elt2);

    inline static MObjectList convertMObjectSetToSortedList(const MObjectSet &mObjectSet);
    inline static MObjectList convertMObjectSetToSortedListAlpha(const MObjectSet &mObjectSet);

    void serialize(XmiWriter *xmiWriter, const QString &tagName, const QString &xmiType = "");


    void exportWithLinksAsNewModelSharingSameModelObjects(Model *subModel, const QSet<MObjectType*> &rootTypesToNotTake = QSet<MObjectType*>());//!< use to do some xml exports of the mObject with all the links needed


    virtual QVariant getPropertyMapKey(Property *mapProperty); //!< for non use of that function, we return the name by default

#ifdef __USE_HMI__
    virtual QIcon getIcon();
#endif

protected:
    MObject(QMap<QString, Property*>* classPropertyMap);
    void _addBusinessError(QStringList &businessErrors, MObject *visibleObject, const QString &errorMsg);


private:

    void _initPropertyValueMap(QMap<QString, Property *> *classPropertyMap);


    // Those methods are shared with the Property classes
    template<typename TypeAttribute> TypeAttribute getPropertyValue(AttributeProperty<TypeAttribute> *property) const;
    template<typename TypeAttribute> QList<TypeAttribute> getListPropertyValue(AttributeListProperty<TypeAttribute> *property);
    template<typename ReturnTypeLinkProperty> ReturnTypeLinkProperty *getLinkPropertyValue(Property *property);

    void setPropertyValueFromQVariant(Property *property, const QVariant &value);
    void setPropertyValueFromElement(LinkProperty *property, MObject *value);
    template<typename ReturnTypeLinkProperty> void setLinkToManyPropertyValue(Property *property, ReturnTypeLinkProperty *value);


    // Those 2 functions will be specialized for each type but the template will be used by GenericLinkToManyProperty<Container, Args...>::addLink
    template <template <typename...> class Container, typename... Args> void addALinkToMany(Property *property, MObject *value);
    template <template <typename...> class Container, typename... Args> void removeALinkFromMany(Property *property, MObject *value);
};


Q_DECLARE_METATYPE( MObjectType* )
Q_DECLARE_METATYPE( MObject* )
Q_DECLARE_METATYPE( MObjectSet* )
Q_DECLARE_METATYPE( MObjectList* )
Q_DECLARE_METATYPE( MObjectMap* )
Q_DECLARE_METATYPE( MObjectMultiMap* )


struct ModelObjectUpdate{
    MObject *existingElement; // can be null
    double   oldVal;
    double   newVal;

    explicit ModelObjectUpdate(MObject *elem_, double oldVal_, double newVal_)
        : existingElement(elem_), oldVal(oldVal_), newVal(newVal_){}
    ~ModelObjectUpdate() = default;
};


////////////////////////////////
/// inline functions definition
////////////////////////////////
int MObject::getModelObjectTypeId() const { return getModelObjectType()->getId(); }
const QString MObject::getModelObjectTypeName() const { return getModelObjectType()->getName(); }
const QString MObject::getModelObjectTypeLabel() const { return getModelObjectType()->getLabel(); }

QVariant MObject::toVariant() { return QVariant::fromValue(static_cast<void*>(this)); }


ElemId MObject::getId() const { return _id; }
void MObject::setId(const ElemId &id) { _id = id; }

inline bool MObject::isInModel() const {return _state == STATE::ADDED_IN_MODEL;}

bool MObject::isReadOnly() const { return _isReadOnly; }
void MObject::setReadOnly(bool readOnly) { _isReadOnly = readOnly; }

bool MObject::isNameReadOnly() const { return _isReadOnly || _isNameReadOnly; }
void MObject::setNameReadOnly(bool readOnly) { _isNameReadOnly = readOnly; }

bool MObject::isA(MObjectType *type) const { return getModelObjectType()->isA(type); }

QList<Property *> MObject::getPropertyList() const { return _propertyValueMap.keys(); }

QVariant MObject::getPropertyVariant(Property *property) const { return _propertyValueMap.value(property, QVariant()); }
MObject *MObject::getEcoreContainer() { return nullptr; }

bool MObject::operator<(MObject& elt){ return getId() < elt.getId(); }
bool MObject::elementNameInsensitiveLessThan(MObject *elt1, MObject *elt2) {return  elt1->getName().toLower() < elt2->getName().toLower();}
bool MObject::elementIdLessThan(MObject *elt1, MObject *elt2) { return elt1->getId() < elt2->getId();}

MObjectList MObject::convertMObjectSetToSortedList(const MObjectSet &mObjectSet)
{
    MObjectList list = mObjectSet.toList();
    std::sort(list.begin(), list.end(), &MObject::elementIdLessThan);
    return list;
}

MObjectList MObject::convertMObjectSetToSortedListAlpha(const MObjectSet &mObjectSet)
{
    MObjectList list = mObjectSet.toList();
    std::sort(list.begin(), list.end(), &MObject::elementNameInsensitiveLessThan);
    return list;
}


////////////////////////////////////////////
/// Template functions definition (generic)
////////////////////////////////////////////

template<typename ReturnTypeLinkProperty>
void MObject::setLinkToManyPropertyValue(Property *property, ReturnTypeLinkProperty *value)
{
    ReturnTypeLinkProperty *propertyValues = getLinkPropertyValue<ReturnTypeLinkProperty>(property);
    propertyValues->swap(*value);
}

template<typename ReturnTypeLinkProperty>
ReturnTypeLinkProperty *MObject::getLinkPropertyValue(Property *property)
{
    QVariant &variant = _propertyValueMap[property];
    return static_cast<ReturnTypeLinkProperty*>(variant.value<void*>());
}


template<typename TypeAttribute>
TypeAttribute MObject::getPropertyValue(AttributeProperty<TypeAttribute> *property) const
{
    const QVariant &variant = _propertyValueMap[property];
    return variant.value<TypeAttribute>();
}

template<typename TypeAttribute>
QList<TypeAttribute> MObject::getListPropertyValue(AttributeListProperty<TypeAttribute> *property)
{
    QVariant &variant = _propertyValueMap[property];
    return variant.value<QList<TypeAttribute> >();
}



/////////////////////////////////////////////////
/// Template functions specializations (per type)
/////////////////////////////////////////////////

// Template specializations of addALinkToMany for QSet, QList, QMap and QMultiMap
template <> inline void MObject::addALinkToMany<QSet>(Property *property, MObject *value)
{
    if (value)
    {
        MObjectSet *propertyValues = getLinkPropertyValue<MObjectSet>(property);
        propertyValues->insert(value);
    }
}
template <> inline void MObject::addALinkToMany<QList>(Property *property, MObject *value)
{
    if (value)
    {
        MObjectList *propertyValues = getLinkPropertyValue<MObjectList>(property);
//        if (!propertyValues->contains(value))
            propertyValues->append(value);
    }
}
template <> inline void MObject::addALinkToMany<QMap, QVariant>(Property *property, MObject *value)
{
    if (value)
    {
        MObjectMap *propertyValues = getLinkPropertyValue<MObjectMap>(property);
        QVariant key = value->getPropertyMapKey(property);
        propertyValues->insert(key, value);
    }
}
template <> inline void MObject::addALinkToMany<QMultiMap, QVariant>(Property *property, MObject *value)
{
    if (value)
    {
        MObjectMultiMap *propertyValues = getLinkPropertyValue<MObjectMultiMap>(property);
        QVariant key = value->getPropertyMapKey(property);
        propertyValues->insert(key, value);
    }
}

// Template specializations of removeALinkFromMany2 for QSet, QList, QMap and QMultiMap
template <> inline void MObject::removeALinkFromMany<QSet>(Property *property, MObject *value)
{
    if (value)
    {
        MObjectSet *propertyValues = getLinkPropertyValue<MObjectSet>(property);
        propertyValues->remove(value);
    }
}
template <> inline void MObject::removeALinkFromMany<QList>(Property *property, MObject *value)
{
    if (value)
    {
        MObjectList *propertyValues = getLinkPropertyValue<MObjectList>(property);
        propertyValues->removeOne(value);
    }
}
template <> inline void MObject::removeALinkFromMany<QMap, QVariant>(Property *property, MObject *value)
{
    if (value)
    {
        MObjectMap *propertyValues = getLinkPropertyValue<MObjectMap>(property);
        QVariant key = value->getPropertyMapKey(property);
        propertyValues->remove(key);
    }
}
template <> inline void MObject::removeALinkFromMany<QMultiMap, QVariant>(Property *property, MObject *value)
{
    if (value)
    {
        MObjectMultiMap *propertyValues = getLinkPropertyValue<MObjectMultiMap>(property);
        QVariant key = value->getPropertyMapKey(property);
        propertyValues->remove(key, value); // remove only the couple (key, value)
    }
}

#endif /* MOBJECT_H_ */
