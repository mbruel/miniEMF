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

#ifndef PROPERTY_H
#define PROPERTY_H
#include <type_traits>
#include <QStringList>
#include <QSet>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QDateTime>

#include <Utils/XmiWriter.h>

#include "MObject.h"
#include "Model.h"

#ifdef __USE_HMI__
#include <QLineEdit>
#include <QComboBox>
#endif

class Property
{
public:
    virtual ~Property() = default;

    const QString getName()  const { return _name; }
    QString getLabel() const;
    const QString getUnit()  const { return _unit; }
    bool isSerializable()    const { return _serializable; }

    void setUnit(const char *unit) { _unit = unit; }

    virtual bool isAttributeProperty() const { return false; }
    virtual bool isALinkProperty()     const { return false; }
    virtual bool isEcoreContainment()  const { return false; }
    virtual bool isEcoreContainer()    const { return false; }

    virtual QVariant createNewInitValue() = 0;

    virtual void serializeAsXmiAttribute(XmiWriter *xmiWriter, MObject *mObject) = 0;
    virtual void deserializeFromXmiAttribute(MObject *const mObject, const QString &xmiValue) = 0;

    virtual void updateValue(MObject *const mObject, QVariant value);
    virtual QVariant convertIntoUpdatableValue(QVariant value);

#ifdef __USE_HMI__
    virtual QWidget *getEditor(MObject *const mObj, Model *model = nullptr, QWidget *parent = nullptr) = 0;
    virtual QVariant getEditorUpdatedVariant(MObject *const mObj, QWidget *editor) = 0;
    virtual QValidator *getEditorValidator(QObject *parent) {Q_UNUSED(parent);return nullptr;}
#endif

    static MObjectList getMapValuesInInsertionOrder(const MObjectMap &map);

    static const int    INT_INFINITE_POS;
    static const int    INT_INFINITE_NEG;
    static const float  FLT_INFINITE_POS;
    static const float  FLT_INFINITE_NEG;
    static const double DBL_INFINITE_POS;
    static const double DBL_INFINITE_NEG;

protected:
    Property(const QString &name, const char *label, bool isSerializable = true);

protected:
    const QString _name;
    const char   *_label;
    const char   *_unit;
    const bool    _serializable;
};


template<typename TypeAttribute> class AttributeProperty : public Property{
public:
    AttributeProperty(const QString &name, const char *label, const TypeAttribute &defaultValue = TypeAttribute());
    virtual ~AttributeProperty() = default;

    bool isAttributeProperty() const override { return true; }

    void setDefaultValue(const TypeAttribute &defaultValue)  { _defaultValue = defaultValue; }
    bool getDefaultValue() const                        { return _defaultValue; }

    TypeAttribute getValue(const MObject *mObject);
    void setValue(MObject *const mObject, const TypeAttribute &value);

    // Mandatory function to be instanciable
    QVariant createNewInitValue() override { return _defaultValue; }

    virtual void serializeAsXmiAttribute(XmiWriter *xmiWriter, MObject *mObject) override;
    virtual void deserializeFromXmiAttribute(MObject *const mObject, const QString &xmiValue) override;

    QVariant convertIntoUpdatableValue(QVariant value) override {return Property::convertIntoUpdatableValue(value);}

#ifdef __USE_HMI__
    virtual QWidget *getEditor(MObject *const mObj, Model *model = nullptr, QWidget *parent = nullptr) override;
    virtual QVariant getEditorUpdatedVariant(MObject *const mObj, QWidget *editor) override;
    virtual QValidator *getEditorValidator(QObject *parent) override;
#endif

private:
    TypeAttribute _defaultValue;
};

class uIntProperty : public IntProperty{
public:
    uIntProperty(const QString &name, const char *label, const int &defaultValue = 0);
    ~uIntProperty();
#ifdef __USE_HMI__
    QValidator *getEditorValidator(QObject *parent) override;
#endif
};

class uDoubleProperty : public DoubleProperty{
public:
    uDoubleProperty(const QString &name, const char *label, const double &defaultValue = 0);
    ~uDoubleProperty();
#ifdef __USE_HMI__
    QValidator *getEditorValidator(QObject *parent) override;
#endif
};

class PerCentProperty: public DoubleProperty {
public:
    PerCentProperty(const QString &name, const char *label, const double &defaultValue = 0);
    ~PerCentProperty();
#ifdef __USE_HMI__
    QValidator *getEditorValidator(QObject *parent) override;
#endif
};

class EnumProperty : public AttributeProperty<int>
{
public:
    EnumProperty(const QString &name, const char *label, int defaultValue = 0);
    ~EnumProperty() = default;

    QString getValueAsString(MObject *const mObject);
    void setEnumValues(const QMap<int,QString> &enumValues);

    QMap<int, QString> getEnumValues();
    QString getEnumValueByKey(int key) const;
    int getEnumKeyByValue(QString value) const;
    QStringList getAllValues();

    // Mandatory function to be instanciable
    void serializeAsXmiAttribute(XmiWriter *xmiWriter, MObject *mObject) override;
    void deserializeFromXmiAttribute(MObject *const mObject, const QString &xmiValue) override;

#ifdef __USE_HMI__
    QWidget *getEditor(MObject *const mObj, Model *model = nullptr, QWidget *parent = nullptr) override;
    QVariant getEditorUpdatedVariant(MObject *const mObj, QWidget *editor) override;
    int getEditorValue(QWidget *editor);
    void setEditorValue(QWidget *editor, int value);
#endif

private:
    QMap<int, QString> _enumValues;
};



template<typename TypeAttribute> class AttributeListProperty : public Property{
public:
    AttributeListProperty(const QString &name, const char *label);
    virtual ~AttributeListProperty() = default;

    bool isAttributeProperty() const override { return true; }

    QList<TypeAttribute> getValue(MObject *const mObject);
    void setValue(MObject *const mObject, const QList<TypeAttribute> &value);

    // Mandatory function to be instanciable
    QVariant createNewInitValue() override { return QVariant::fromValue(QList<TypeAttribute>()); }

    virtual void serializeAsXmiAttribute(XmiWriter *xmiWriter, MObject *mObject) override;
    virtual void deserializeFromXmiAttribute(MObject *const mObject, const QString &xmiValue) override;

#ifdef __USE_HMI__
    QWidget *getEditor(MObject *const mObj, Model *model = nullptr, QWidget *parent = nullptr) override {Q_UNUSED(mObj);return new QWidget(parent);}
    QVariant getEditorUpdatedVariant(MObject *const mObj, QWidget *editor) override {Q_UNUSED(mObj);Q_UNUSED(editor); return QVariant();}
#endif
};


// pure virtual class
class LinkProperty : public Property
{
public:
    bool isALinkProperty()              const override { return true; }
    bool isMandatory()                  const          { return _isMandatory; }
    virtual bool isOrdered()            const = 0;
    virtual bool isMapProperty()        const = 0;
    virtual bool isALinkToOneProperty() const = 0;
    bool isALinkToManyProperty() const ;

    virtual ~LinkProperty() = default;

    virtual void updateValue(MObject *const mObject, QVariant value) override = 0;

    // Handy setter from a LinkToMany properties
    virtual void updateValue(MObject *const mObject, const MObjectList &values) = 0;

    virtual void addLink(MObject *const mObject, MObject *const mObjectToAdd) = 0;
    virtual void removeLink(MObject *const mObject, MObject *const mObjectToRemove) = 0;
    virtual MObjectList getLinkedModelObjects(MObject *const mObject, bool ordered = false) = 0;
    virtual void setValues(MObject *mObject, const MObjectList &values) = 0;

    virtual MObjectMap *getLinkedModelObjectsMap(MObject *const mObject) {Q_UNUSED(mObject); return nullptr;}
    virtual void setValuesFromMap(MObject *mObject, MObjectMap *values) {Q_UNUSED(mObject);Q_UNUSED(values);}

    bool isEcoreContainment() const override {return _isEcoreContainment;}
    void setEcoreContainment(bool isEcoreContainment) {_isEcoreContainment = isEcoreContainment;}

    bool isEcoreContainer() const override;

    LinkProperty* getReverseLinkProperty() const { return _reverseLinkProperty; }
    void setReverseLinkProperty(LinkProperty *const reverseLinkProperty);
    void updateReverseProperty(MObject* mObject, MObject* value);

    MObjectType* getModelObjectType() const { return _mObjectType; }
    MObjectType* getLinkedModelObjectType() const { return _linkedModelObjectType; }

    virtual void serializeAsXmiAttribute(XmiWriter *xmiWriter, MObject *mObject) override;
    void deserializeFromXmiAttribute(MObject *mObject, const QString &xmiValue) override;

    virtual void setValueFromXMIStringIdList(MObject *mObject, const QString &ids, Model *model) = 0;

     void validateModelObject(MObject *mObject, QStringList &ecoreErrors);

#ifdef __USE_HMI__
    virtual QWidget *getEditor(MObject *const mObj, Model *model = nullptr, QWidget *parent = nullptr) override {Q_UNUSED(mObj);return new QWidget(parent);}
    virtual QVariant getEditorUpdatedVariant(MObject *const mObj, QWidget *editor) override {Q_UNUSED(mObj);Q_UNUSED(editor); return QVariant();}
#endif

protected:
    LinkProperty(MObjectType *const eltType, MObjectType *const linkedEltType, const QString &name, const char *label, bool isMandatory, bool isSerializable = true);

protected:
    MObjectType  *_mObjectType;       // MObjectType to which this LinkProperty belongs
    MObjectType  *_linkedModelObjectType; // MObjectType linked through this LinkProperty
    bool         _isEcoreContainment;
    LinkProperty *_reverseLinkProperty;
    const bool    _isMandatory;
};


class LinkToOneProperty : public LinkProperty
{
public:
    virtual bool isOrdered()            const override { return false; }
    virtual bool isMapProperty()        const override { return false; }
    virtual bool isALinkToOneProperty() const override { return true; }

    LinkToOneProperty(MObjectType *const eltType, MObjectType *const linkedEltType, const QString &name, const char *label, bool isMandatory, bool isSerializable = true):
        LinkProperty(eltType, linkedEltType, name, label, isMandatory, isSerializable),
        _defaultLinkedObject(nullptr)
    {}
    virtual ~LinkToOneProperty() = default;

    QVariant createNewInitValue() override;

    void updateValue(MObject *const mObject, QVariant value) override;
    void updateValue(MObject *const mObject, const MObjectList &values) override;

    void addLink(MObject *const mObject, MObject *const mObjectToAdd) override;
    void removeLink(MObject *const mObject, MObject *const mObjectToRemove) override;
    MObjectList getLinkedModelObjects(MObject *const mObject, bool ordered = false) override;

    MObject* getValue(MObject *const mObject);
    void setValue(MObject *const mObject, MObject *const value);
    void setValues(MObject *mObject, const MObjectList &values) override;

    void setValueFromXMIStringIdList(MObject *mObject, const QString &ids, Model *model) override;

#ifdef __USE_HMI__
    virtual QWidget *getEditor(MObject *const mObj, Model *model = nullptr, QWidget *parent = nullptr) override;
    virtual QVariant getEditorUpdatedVariant(MObject *const mObj, QWidget *editor) override;
    MObject *getEditorValue(QWidget *editor);
    void setEditorValue(QWidget *editor, MObject *value);
    QWidget *getEditorUsingAlsoDerivedLinkedObjects(MObject *const mObj, Model *model, QWidget *parent = nullptr);
#endif

    inline void setDefaultLinkedObject(MObject *mObject);

protected:
    MObject *_defaultLinkedObject;
};

void LinkToOneProperty::setDefaultLinkedObject(MObject *mObject) {_defaultLinkedObject = mObject;}



template <template <typename...> class Container, typename... Args> class GenericLinkToManyProperty : public LinkProperty {
public:
    virtual bool isOrdered()            const override { return true; }
    virtual bool isMapProperty()        const override { return false; }
    virtual bool isALinkToOneProperty() const override { return false; }

    GenericLinkToManyProperty(MObjectType *const eltType, MObjectType *const linkedEltType, const QString &name, const char *label, bool isMandatory, bool isSerializable = true):
        LinkProperty(eltType, linkedEltType, name, label, isMandatory, isSerializable) {}
    virtual ~GenericLinkToManyProperty() = default;

    QVariant createNewInitValue() override;

    virtual void addLink(MObject *const mObject, MObject *const mObjectToAdd) override;
    virtual void removeLink(MObject *const mObject, MObject *const mObjectToRemove) override;
    virtual void updateValue(MObject *const mObject, QVariant value) override;
    virtual MObjectList getLinkedModelObjects(MObject *const mObject, bool ordered = true) override;

    Container<Args..., MObject*> *getValues(MObject *const mObject) ;
    void setValues(MObject *mObject, Container<Args..., MObject*> *values);
    void setValues(MObject *mObject, const MObjectList &values) override;

    // Handy setter from a QSet
    void updateValue(MObject *const mObject, const MObjectList &values) override;

    virtual MObjectMap *getLinkedModelObjectsMap(MObject *const mObject) override {Q_UNUSED(mObject);return nullptr;}
    virtual void setValuesFromMap(MObject *mObject, MObjectMap *values) override {Q_UNUSED(mObject);Q_UNUSED(values);}

    virtual void serializeAsXmiAttribute(XmiWriter *xmiWriter, MObject *mObject) override;
    virtual void setValueFromXMIStringIdList(MObject *mObject, const QString &ids, Model *model) override;
};

template <> inline bool MapLinkProperty::isMapProperty()      const {return true;}
template <> inline bool MultiMapLinkProperty::isMapProperty() const {return true;}
template <> inline bool LinkToManyProperty::isOrdered()       const {return false;}

/*
class LinkToOneProperty : public GenericLinkToManyProperty<QSet>
{
public:
    bool isALinkToOneProperty() const override { return true; }

    LinkToOneProperty(MObjectType *const eltType, MObjectType *const linkedEltType, const QString &name, const char *label, bool isSerializable = true):
        GenericLinkToManyProperty<QSet>(eltType, linkedEltType, name, label, isSerializable){}

    virtual ~LinkToOneProperty() = default;

    MObject* getValue(MObject *const mObject);
    void setValue(MObject *const mObject, MObject *const value);
    void addLink(MObject *const mObject, MObject *const mObjectToAdd) override;
    void removeLink(MObject *const mObject, MObject *const mObjectToRemove) override;
    void updateValue(MObject *const mObject, QVariant value) override;

    void setValueFromXMIStringIdList(MObject *mObject, const QString &ids, Model *model) override;

#ifdef __USE_HMI__
    virtual QWidget *getEditor(MObject *const mObj, Model *model = nullptr, QWidget *parent = nullptr) override;
    virtual QVariant getEditorUpdatedVariant(MObject *const mObj, QWidget *editor) override;
#endif
};

class Link11Property : public LinkToOneProperty
{
public:
    bool isMandatory() const override { return true; }

    Link11Property(MObjectType *const eltType, MObjectType *const linkedEltType, const QString &name, const char *label, bool isSerializable = true):
        LinkToOneProperty(eltType, linkedEltType, name, label, isSerializable) {}

    ~Link11Property() = default;
};
*/

//////////////////////////////////////////////////////
// Template definitions for GenericLinkToManyProperty
//////////////////////////////////////////////////////

template <template <typename...> class Container, typename... Args>
    QVariant GenericLinkToManyProperty<Container, Args...>::createNewInitValue()
{
    return QVariant::fromValue(static_cast<void*>(new Container<Args..., MObject*>()));
}
template <template <typename...> class Container, typename... Args>
    Container<Args..., MObject*> * GenericLinkToManyProperty<Container, Args...>::getValues(MObject *const mObject)
{
    return mObject->getLinkPropertyValue<Container<Args..., MObject*>>(this);
}
template <template <typename...> class Container, typename... Args>
    void GenericLinkToManyProperty<Container, Args...>::setValues(MObject *mObject, Container<Args..., MObject*> *values)
{
    mObject->setLinkToManyPropertyValue<Container<Args..., MObject*>>(this, values);
}
template <template <typename...> class Container, typename... Args>
    void GenericLinkToManyProperty<Container, Args...>::addLink(MObject *const mObject, MObject *const mObjectToAdd)
{
    mObject->addALinkToMany<Container, Args...>(this, mObjectToAdd);
}
template <template <typename...> class Container, typename... Args>
    void GenericLinkToManyProperty<Container, Args...>::removeLink(MObject *const mObject, MObject *const mObjectToRemove)
{
    mObject->removeALinkFromMany<Container, Args...>(this, mObjectToRemove);
}
template <template <typename...> class Container, typename... Args>
    void GenericLinkToManyProperty<Container, Args...>::serializeAsXmiAttribute(XmiWriter *xmiWriter, MObject *mObject)
{
    xmiWriter->addAttribute(_name, getLinkedModelObjects(mObject, true));
}

//////////////////////////////////////////////////////////
// Template specializations for GenericLinkToManyProperty
//////////////////////////////////////////////////////////

// Template specializations of getLinkedModelObjects for QSet, QList, QMap and QMultiMap
template <> inline MObjectList LinkToManyProperty::getLinkedModelObjects(MObject *const mObject, bool ordered)
{
    if (ordered)
    {
        MObjectList list = getValues(mObject)->toList();
        std::sort(list.begin(), list.end(), &MObject::elementIdLessThan);
        return list;
    }
    else
        return getValues(mObject)->toList();
}
template <> inline MObjectList OrderedLinkToManyProperty::getLinkedModelObjects(MObject *const mObject, bool ordered)
{
    Q_UNUSED(ordered)
    return *(getValues(mObject));
}
template <> inline MObjectList MapLinkProperty::getLinkedModelObjects(MObject *const mObject, bool ordered)
{
    Q_UNUSED(ordered)
    return getValues(mObject)->values();
}
template <> inline MObjectList MultiMapLinkProperty::getLinkedModelObjects(MObject *const mObject, bool ordered)
{
    Q_UNUSED(ordered)
    return getValues(mObject)->values();
}

template <> inline MObjectMap *MapLinkProperty::getLinkedModelObjectsMap(MObject *const mObject)
{
    return getValues(mObject);
}
template <> inline  void MapLinkProperty::setValuesFromMap(MObject *mObject, MObjectMap *values)
{
    setValues(mObject, values);
}
template <> inline MObjectMap *MultiMapLinkProperty::getLinkedModelObjectsMap(MObject *const mObject)
{
    return getValues(mObject);
}
template <> inline  void MultiMapLinkProperty::setValuesFromMap(MObject *mObject, MObjectMap *values)
{
    MObjectMultiMap multiMap = *values;
    setValues(mObject, &multiMap);
}

// Template specializations of setValue with MObjectList for QSet, QList, QMap and QMultiMap
template <> inline void LinkToManyProperty::setValues(MObject *mObject, const MObjectList &values)
{
    MObjectSet mObjSet = values.toSet();
    setValues(mObject, &mObjSet);
}
template <> inline void OrderedLinkToManyProperty::setValues(MObject *mObject, const MObjectList &values)
{
    MObjectList mObjList = values;
    setValues(mObject, &mObjList);
}
template <> inline void MapLinkProperty::setValues(MObject *mObject, const MObjectList &values)
{
    MObjectMap map;
    for (MObject *val : values)
        map.insert(val->getPropertyMapKey(this), val);
    setValues(mObject, &map);
}
template <> inline void MultiMapLinkProperty::setValues(MObject *mObject, const MObjectList &values)
{
    MObjectMultiMap map;
    for (MObject *val : values)
        map.insert(val->getPropertyMapKey(this), val);
    setValues(mObject, &map);
}


// Template specializations of updateValue for QSet, QList, QMap and QMultiMap
template <> inline void LinkToManyProperty::updateValue(MObject *const mObject, QVariant value)
{
    if (!value.canConvert<void* >())
        return;

    MObjectSet *newValueSet = static_cast<MObjectSet*>(value.value<void*>()),
            *oldValueSet = mObject->getLinkPropertyValue<MObjectSet>(this);
    if (*newValueSet == *oldValueSet)
        return;

    if (_reverseLinkProperty)
    {
        for (MObject *linkedModelObject : *newValueSet)
        {
            if (!oldValueSet->contains(linkedModelObject))
                _reverseLinkProperty->addLink(linkedModelObject, mObject);
        }

        for (MObject *linkedModelObject : *oldValueSet)
        {
            if(!newValueSet->contains(linkedModelObject))
                _reverseLinkProperty->removeLink(linkedModelObject, mObject);
        }
    }

    // As we do a swap for efficiency we need to do this at the end
    setValues(mObject, newValueSet);
}
template <> inline void OrderedLinkToManyProperty::updateValue(MObject *const mObject, QVariant value)
{
    if (!value.canConvert<void* >())
        return;

    MObjectList *newValueList = static_cast<MObjectList*>(value.value<void*>()),
            *oldValueList = mObject->getLinkPropertyValue<MObjectList>(this);
    if (*newValueList == *oldValueList)
        return;

    if (_reverseLinkProperty)
    {
        for (MObject *linkedModelObject : *newValueList)
        {
            if (!oldValueList->contains(linkedModelObject))
                _reverseLinkProperty->addLink(linkedModelObject, mObject);
        }

        for (MObject *linkedModelObject : *oldValueList)
        {
            if(!newValueList->contains(linkedModelObject))
                _reverseLinkProperty->removeLink(linkedModelObject, mObject);
        }
    }

    setValues(mObject, newValueList);
}
template <> inline void MapLinkProperty::updateValue(MObject *const mObject, QVariant value)
{
    if (!value.canConvert<void*>())
        return;

    MObjectMap *newValueMap = static_cast<MObjectMap*>(value.value<void*>()),
            *oldValueMap = mObject->getLinkPropertyValue<MObjectMap>(this);
    if (*newValueMap == *oldValueMap)
        return;

    if (_reverseLinkProperty)
    {
        for (auto it = newValueMap->cbegin() , itEnd = newValueMap->cend(); it != itEnd; ++it)
        {
            if (!oldValueMap->contains(it.key()))
                _reverseLinkProperty->addLink(it.value(), mObject);
        }

        for (auto it = oldValueMap->cbegin() , itEnd = oldValueMap->cend(); it != itEnd; ++it)
        {
            if(!newValueMap->contains(it.key()))
                _reverseLinkProperty->removeLink(it.value(), mObject);
        }
    }

    setValues(mObject, newValueMap);
}
template <> inline void MultiMapLinkProperty::updateValue(MObject *const mObject, QVariant value)
{
    if (!value.canConvert<void*>())
        return;

    MObjectMultiMap *newValueMap = static_cast<MObjectMultiMap*>(value.value<void*>()),
            *oldValueMap = mObject->getLinkPropertyValue<MObjectMultiMap>(this);
    if (*newValueMap == *oldValueMap)
        return;

    if (_reverseLinkProperty)
    {
        for (auto it = newValueMap->cbegin() , itEnd = newValueMap->cend(); it != itEnd; ++it)
        {
            if (!oldValueMap->contains(it.key(), it.value()))
                _reverseLinkProperty->addLink(it.value(), mObject);
        }

        for (auto it = oldValueMap->cbegin() , itEnd = oldValueMap->cend(); it != itEnd; ++it)
        {
            if(!newValueMap->contains(it.key(), it.value()))
                _reverseLinkProperty->removeLink(it.value(), mObject);
        }
    }
    setValues(mObject, newValueMap);
}



template <> inline void LinkToManyProperty::updateValue(MObject *const mObject, const MObjectList &values)
{
    MObjectSet set(values.toSet());
    updateValue(mObject, QVariant::fromValue(static_cast<void*>(&set)));
}
template <> inline void OrderedLinkToManyProperty::updateValue(MObject *const mObject, const MObjectList &values)
{
    MObjectList listCopy(values);
    updateValue(mObject, QVariant::fromValue(static_cast<void*>(&listCopy)));
}
template <> inline void MapLinkProperty::updateValue(MObject *const mObject, const MObjectList &values)
{
    MObjectMap map;
    for (MObject *val : values)
        map[val->getPropertyMapKey(this)] = val;
    updateValue(mObject, QVariant::fromValue(static_cast<void*>(&map)));
}

template <> inline void MultiMapLinkProperty::updateValue(MObject *const mObject, const MObjectList &values)
{
    MObjectMultiMap map;
    for (MObject *val : values)
        map.insert(val->getPropertyMapKey(this), val);
    updateValue(mObject, QVariant::fromValue(static_cast<void*>(&map)));
}


template <> inline void LinkToManyProperty::setValueFromXMIStringIdList(MObject *mObject, const QString &ids, Model *model)
{
    MObjectSet mObjectsToLink;
    MObjectType *linkedEltType = mObject->getLinkedModelObjectType(this);
    for (const QString &id_ : ids.split(" "))
    {
        QString id = id_.trimmed();
        if (!id.isEmpty())
        {
            MObject *linkedModelObject = model->getModelObjectById(linkedEltType, id);
            if (linkedModelObject)
                mObjectsToLink.insert(linkedModelObject);
        }
    }
    if (!mObjectsToLink.isEmpty())
        updateValue(mObject, QVariant::fromValue(static_cast<void*>(&mObjectsToLink)));
}
template <> inline void OrderedLinkToManyProperty::setValueFromXMIStringIdList(MObject *mObject, const QString &ids, Model *model)
{
    MObjectList mObjectsToLink;
    MObjectType *linkedEltType = mObject->getLinkedModelObjectType(this);
    for (const QString &id_ : ids.split(" "))
    {
        QString id = id_.trimmed();
        if (!id.isEmpty())
        {
            MObject *linkedModelObject = model->getModelObjectById(linkedEltType, id);
            if (linkedModelObject)
                mObjectsToLink.append(linkedModelObject);
        }
    }
    if (!mObjectsToLink.isEmpty())
        updateValue(mObject, QVariant::fromValue(static_cast<void*>(&mObjectsToLink)));
}
template <> inline void MapLinkProperty::setValueFromXMIStringIdList(MObject *mObject, const QString &ids, Model *model)
{
    MObjectMap mObjectsToLink;
    MObjectType *linkedEltType = mObject->getLinkedModelObjectType(this);
    for (const QString &id_ : ids.split(" "))
    {
        QString id = id_.trimmed();
        if (!id.isEmpty())
        {
            MObject *linkedModelObject = model->getModelObjectById(linkedEltType, id);
            if (linkedModelObject)
                mObjectsToLink[linkedModelObject->getPropertyMapKey(this)] = linkedModelObject;
        }
    }
    if (!mObjectsToLink.isEmpty())
        updateValue(mObject, QVariant::fromValue(static_cast<void*>(&mObjectsToLink)));
}
template <> inline void MultiMapLinkProperty::setValueFromXMIStringIdList(MObject *mObject, const QString &ids, Model *model)
{
    MObjectMultiMap mObjectsToLink;
    MObjectType *linkedEltType = mObject->getLinkedModelObjectType(this);
    for (const QString &id_ : ids.split(" "))
    {
        QString id = id_.trimmed();
        if (!id.isEmpty())
        {
            MObject *linkedModelObject = model->getModelObjectById(linkedEltType, id);
            if (linkedModelObject)
                mObjectsToLink.insert(linkedModelObject->getPropertyMapKey(this), linkedModelObject);
        }
    }
    if (!mObjectsToLink.isEmpty())
        updateValue(mObject, QVariant::fromValue(static_cast<void*>(&mObjectsToLink)));
}

template <> inline void MapLinkProperty::serializeAsXmiAttribute(XmiWriter *xmiWriter, MObject *mObject)
{
    MObjectMap *map = getValues(mObject);
    xmiWriter->addAttribute(_name, getMapValuesInInsertionOrder(*map));
}
template <> inline void MultiMapLinkProperty::serializeAsXmiAttribute(XmiWriter *xmiWriter, MObject *mObject)
{
    MObjectMap *map = getValues(mObject);
    xmiWriter->addAttribute(_name, getMapValuesInInsertionOrder(*map));
}

////////////////////////////////////////////////////////////////
// Template definitions for AttributeProperty (nothing to touch)
////////////////////////////////////////////////////////////////

template<typename TypeAttribute>
AttributeProperty<TypeAttribute>::AttributeProperty(const QString &name, const char *label, const TypeAttribute &defaultValue):
    Property(name, label), _defaultValue(defaultValue){}

template<typename TypeAttribute>
TypeAttribute AttributeProperty<TypeAttribute>::getValue(const MObject *mObject)
{
    return mObject->getPropertyValue<TypeAttribute>(this);
}

template<typename TypeAttribute>
void AttributeProperty<TypeAttribute>::setValue(MObject * const mObject, const TypeAttribute &value)
{
    mObject->setPropertyValueFromQVariant(this, QVariant::fromValue(value));
}

template<typename TypeAttribute>
void AttributeProperty<TypeAttribute>::serializeAsXmiAttribute(XmiWriter *xmiWriter, MObject *mObject)
{
    TypeAttribute value = getValue(mObject);
    if (value != _defaultValue)
        xmiWriter->addAttribute(_name, value);
}

#ifdef __USE_HMI__
template<typename TypeAttribute>
QWidget *AttributeProperty<TypeAttribute>::getEditor(MObject * const mObj, Model *model, QWidget *parent)
{
    Q_UNUSED(model)
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setText(QString("%1").arg(getValue(mObj)));

    QValidator *validator = getEditorValidator(lineEdit);
    lineEdit->setValidator(validator);

    if (mObj && mObj->isReadOnly())
        lineEdit->setReadOnly(true);
    return lineEdit;
}
#endif

template<typename TypeAttribute>
AttributeListProperty<TypeAttribute>::AttributeListProperty(const QString &name, const char *label)
    : Property(name, label) {}

template<typename TypeAttribute>
QList<TypeAttribute> AttributeListProperty<TypeAttribute>::getValue(MObject * const mObject)
{
    return mObject->getListPropertyValue<TypeAttribute>(this);
}

template<typename TypeAttribute>
void AttributeListProperty<TypeAttribute>::setValue(MObject * const mObject, const QList<TypeAttribute> &value)
{
    mObject->setPropertyValueFromQVariant(this, QVariant::fromValue(value));
}

template<typename TypeAttribute>
void AttributeListProperty<TypeAttribute>::serializeAsXmiAttribute(XmiWriter *xmiWriter, MObject *mObject)
{
    QList<TypeAttribute> values = getValue(mObject);
    if (values.size())
    {
        QString str;
        ushort idx = 0;
        for (auto val : values)
        {
            if (idx++ != 0)
                str += " ";
            str += QString::number(val);
        }
        xmiWriter->addAttribute(_name, str);
    }
}


//////////////////////////////////////////////////////////////////////////////
// Template specializations for AttributeProperty (add one for each new type)
//////////////////////////////////////////////////////////////////////////////

template<> inline void BoolProperty::deserializeFromXmiAttribute(MObject *mObject, const QString &xmiValue)
{
    if (xmiValue.isEmpty())
        setValue(mObject, _defaultValue);
    else
    {
        bool value = (xmiValue == "true");
        setValue(mObject, value);
    }
}

template<> inline void DoubleProperty::deserializeFromXmiAttribute(MObject *mObject, const QString &xmiValue)
{
    if (xmiValue.isEmpty())
        setValue(mObject, _defaultValue);
    else if (xmiValue == "-∞")
        setValue(mObject, DBL_INFINITE_NEG);
    else if (xmiValue == "+∞")
        setValue(mObject, DBL_INFINITE_POS);
    else
        setValue(mObject, xmiValue.toDouble());
}

template<> inline void FloatProperty::deserializeFromXmiAttribute(MObject *mObject, const QString &xmiValue)
{
    if (xmiValue.isEmpty())
        setValue(mObject, _defaultValue);
    else if (xmiValue == "-∞")
        setValue(mObject, FLT_INFINITE_NEG);
    else if (xmiValue == "+∞")
        setValue(mObject, FLT_INFINITE_POS);
    else
        setValue(mObject, xmiValue.toFloat());
}


template<> inline void IntProperty::deserializeFromXmiAttribute(MObject *const mObject, const QString &xmiValue)
{
    if (xmiValue.isEmpty())
        setValue(mObject, _defaultValue);
    else if (xmiValue == "-∞")
        setValue(mObject, INT_INFINITE_NEG);
    else if (xmiValue == "+∞")
        setValue(mObject, INT_INFINITE_POS);
    else
        setValue(mObject, xmiValue.toInt());
}

template<> inline QVariant StringProperty::convertIntoUpdatableValue(QVariant value)
{
    return QVariant(value.toString().replace(";","-"));
}


template<> inline void StringProperty::deserializeFromXmiAttribute(MObject *mObject, const QString &xmiValue)
{
    setValue(mObject, xmiValue);
}

template<> inline void AttributeProperty<QDateTime>::deserializeFromXmiAttribute(MObject *mObject, const QString &xmiValue)
{
    setValue(mObject, QDateTime::fromString(xmiValue, "yyyy/MM/dd hh:mm:ss"));
}


template<> inline void FloatListProperty::deserializeFromXmiAttribute(MObject *mObject, const QString &xmiValue)
{
    QList<float> values;
    if (!xmiValue.isEmpty())
    {
        for (QString val : xmiValue.split(" "))
            values.append(val.toFloat());
    }

    setValue(mObject, values);
}

template<> inline void IntListProperty::deserializeFromXmiAttribute(MObject *mObject, const QString &xmiValue)
{
    QList<int> values;
    if (!xmiValue.isEmpty())
    {
        for (QString val : xmiValue.split(" "))
            values.append(val.toInt());
    }

    setValue(mObject, values);
}

template<> inline void DoubleListProperty::deserializeFromXmiAttribute(MObject *mObject, const QString &xmiValue)
{
    QList<double> values;
    if (!xmiValue.isEmpty())
    {
        for (QString val : xmiValue.split(" "))
            values.append(val.toDouble());
    }

    setValue(mObject, values);
}

#ifdef __USE_HMI__
template<> inline QVariant BoolProperty::getEditorUpdatedVariant(MObject * const mObj, QWidget *editor)
{
    QComboBox *cb = static_cast<QComboBox*>(editor);
    bool value = true;
    if (cb->currentText() == "false")
        value = false;

    if (getValue(mObj) != value)
        return value;
    else
        return QVariant();
}

template<> inline QVariant DoubleProperty::getEditorUpdatedVariant(MObject * const mObj, QWidget *editor)
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    bool inputOk = true;
    QString str(lineEdit->text());
    double newVal;
    if (str == "-∞")
        newVal = DBL_INFINITE_NEG;
    else if (str == "+∞")
        newVal = DBL_INFINITE_POS;
    else
        newVal = str.toDouble(&inputOk);
    if (inputOk && (newVal != getValue(mObj)))
        return newVal;
    else
        return QVariant();
}
template<> inline QVariant FloatProperty::getEditorUpdatedVariant(MObject * const mObj, QWidget *editor)
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    bool inputOk = true;
    QString str(lineEdit->text());
    float newVal;
    if (str == "-∞")
        newVal = FLT_INFINITE_NEG;
    else if (str == "+∞")
        newVal = FLT_INFINITE_POS;
    else
        newVal = str.toFloat(&inputOk);
    if (inputOk && (newVal != getValue(mObj)))
        return newVal;
    else
        return QVariant();
}
template<> inline QVariant IntProperty::getEditorUpdatedVariant(MObject * const mObj, QWidget *editor)
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    bool inputOk = true;
    QString str(lineEdit->text());
    int newVal;
    if (str == "-∞")
        newVal = INT_INFINITE_NEG;
    else if (str == "+∞")
        newVal = INT_INFINITE_POS;
    else
        newVal = str.toInt(&inputOk);
//    int newVal = lineEdit->text().toInt(&inputOk);
    if (inputOk && (newVal != getValue(mObj)))
        return newVal;
    else
        return QVariant();
}
template<> inline QVariant StringProperty::getEditorUpdatedVariant(MObject * const mObj, QWidget *editor)
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    if (lineEdit->text() != getValue(mObj))
        return lineEdit->text();
    else
        return QVariant();
}
template<> inline QVariant DateTimeProperty::getEditorUpdatedVariant(MObject * const mObj, QWidget *editor)
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    QDateTime newDate = QDateTime::fromString(lineEdit->text(), "yyyy/MM/dd hh:mm:ss");
    if (newDate.isValid() && newDate != getValue(mObj))
        return QVariant::fromValue(newDate);
    else
        return QVariant();
}

class DoubleValidatorWithInfinity : public QDoubleValidator
{
public :
    DoubleValidatorWithInfinity(QObject *parent) : QDoubleValidator(parent){}
    virtual ~DoubleValidatorWithInfinity() = default;

    virtual QValidator::State validate(QString & input, int &pos) const override
    {
        if (input == "-∞" || input == "+∞")
            return QValidator::Acceptable;
        else if (input == "--" || input == "++")
            return QValidator::Intermediate;
        else
            return QDoubleValidator::validate(input, pos);
    }


    void fixup(QString &input) const override
    {
        if (input == "--")
            input = "-∞";
        else if (input == "++")
            input = "+∞";
        else
            QDoubleValidator::fixup(input);
    }
};
class IntValidatorWithInfinity : public QIntValidator
{
public :
    IntValidatorWithInfinity(QObject *parent) : QIntValidator(parent){}
    virtual ~IntValidatorWithInfinity() = default;

    virtual QValidator::State validate(QString & input, int &pos) const override
    {
        if (input == "-∞" || input == "+∞")
            return QValidator::Acceptable;
        else if (input == "--" || input == "++")
            return QValidator::Intermediate;
        else
            return QIntValidator::validate(input, pos);
    }
    void fixup(QString &input) const override
    {
        if (input == "--")
            input = "-∞";
        else if (input == "++")
            input = "+∞";
        else
            QIntValidator::fixup(input);
    }
};

class uIntValidatorWithInfinity : public IntValidatorWithInfinity
{
public:
    uIntValidatorWithInfinity(QObject *parent): IntValidatorWithInfinity(parent){}
    ~uIntValidatorWithInfinity() = default;

    QValidator::State validate(QString & input, int &pos) const override
    {
        if (input == "-")
            return QValidator::Invalid;
        else
            return IntValidatorWithInfinity::validate(input, pos);
    }
};

class uDoubleValidatorWithInfinity : public DoubleValidatorWithInfinity
{
public:
    uDoubleValidatorWithInfinity(QObject *parent): DoubleValidatorWithInfinity(parent){}
    ~uDoubleValidatorWithInfinity() = default;

    QValidator::State validate(QString & input, int &pos) const override
    {
        if (input == "-")
            return QValidator::Invalid;
        else
            return DoubleValidatorWithInfinity::validate(input, pos);
    }
};

class PerCentValidator : public QDoubleValidator
{
public:
    PerCentValidator(QObject *parent): QDoubleValidator(0.00000, 100.00000, 5, parent)
    {setNotation(QDoubleValidator::StandardNotation);}
    ~PerCentValidator() = default;
};

inline QValidator *PerCentProperty::getEditorValidator(QObject *parent)
{
    return new PerCentValidator(parent);
}

inline QValidator *uIntProperty::getEditorValidator(QObject *parent)
{
    return new uIntValidatorWithInfinity(parent);
}

inline QValidator *uDoubleProperty::getEditorValidator(QObject *parent)
{
    return new uDoubleValidatorWithInfinity(parent);
}

template<> inline QValidator *IntProperty::getEditorValidator(QObject *parent)
{
    return new IntValidatorWithInfinity(parent);
}
template<> inline QValidator *FloatProperty::getEditorValidator(QObject *parent)
{
    return new DoubleValidatorWithInfinity(parent);
}
template<> inline QValidator *DoubleProperty::getEditorValidator(QObject *parent)
{
    return new DoubleValidatorWithInfinity(parent);
}
template<> inline QValidator *StringProperty::getEditorValidator(QObject *parent)
{
    Q_UNUSED(parent)
    return nullptr;
}
template<> inline QValidator *BoolProperty::getEditorValidator(QObject *parent)
{
    Q_UNUSED(parent)
    return nullptr;
}
template<> inline QValidator *DateTimeProperty::getEditorValidator(QObject *parent)
{
    Q_UNUSED(parent)
    return nullptr;
}


template <> inline QWidget *BoolProperty::getEditor(MObject * const mObj, Model *model, QWidget *parent)
{
    Q_UNUSED(model)
    QComboBox *cb = new QComboBox(parent);
    cb->addItem("true");
    cb->addItem("false");

    bool value = false;
    if (mObj)
        value = getValue(mObj);
    else
        value = _defaultValue;

    if (value)
        cb->setCurrentIndex(0);
    else
        cb->setCurrentIndex(1);

    if (mObj && mObj->isReadOnly())
        cb->setEnabled(false);

    return cb;
}

template <> inline QWidget *DoubleProperty::getEditor(MObject * const mObj, Model *model, QWidget *parent)
{
    Q_UNUSED(model)
    QLineEdit *lineEdit = new QLineEdit(parent);
    if (mObj)
    {
        double val = getValue(mObj);
        QString valStr;
        if (val == DBL_INFINITE_NEG)
            valStr = "-∞";
        else if (val == DBL_INFINITE_POS)
            valStr = "+∞";
        else
            valStr = QString::number(val);

        lineEdit->setText(valStr);

        if (mObj->isReadOnly())
            lineEdit->setReadOnly(true);
    }

    QValidator *validator = getEditorValidator(lineEdit);
    lineEdit->setValidator(validator);
    return lineEdit;
}
template <> inline QWidget *IntProperty::getEditor(MObject * const mObj, Model *model, QWidget *parent)
{
    Q_UNUSED(model)
    QLineEdit *lineEdit = new QLineEdit(parent);
    if (mObj)
    {
        int val = getValue(mObj);
        QString valStr;
        if (val == INT_INFINITE_NEG)
            valStr = "-∞";
        else if (val == INT_INFINITE_POS)
            valStr = "+∞";
        else
            valStr = QString::number(val);

        if (mObj->isReadOnly())
            lineEdit->setReadOnly(true);

        lineEdit->setText(valStr);
    }

    QValidator *validator = getEditorValidator(lineEdit);
    lineEdit->setValidator(validator);
    return lineEdit;
}
template <> inline QWidget *FloatProperty::getEditor(MObject * const mObj, Model *model, QWidget *parent)
{
    Q_UNUSED(model)
    QLineEdit *lineEdit = new QLineEdit(parent);
    if (mObj)
    {
        float val = getValue(mObj);
        QString valStr;
        if (val == FLT_INFINITE_NEG)
            valStr = "-∞";
        else if (val == FLT_INFINITE_POS)
            valStr = "+∞";
        else
            valStr = QString::number(val);

        if (mObj->isReadOnly())
            lineEdit->setReadOnly(true);

        lineEdit->setText(valStr);
    }

    QValidator *validator = getEditorValidator(lineEdit);
    lineEdit->setValidator(validator);
    return lineEdit;
}
template<> inline QWidget *DateTimeProperty::getEditor(MObject * const mObj, Model *model, QWidget *parent)
{
    Q_UNUSED(model)
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setText(getValue(mObj).toString("yyyy/MM/dd hh:mm:ss"));
    if (mObj && mObj->isReadOnly())
        lineEdit->setReadOnly(true);
    return lineEdit;
}
#endif

Q_DECLARE_METATYPE(LinkProperty*)
Q_DECLARE_METATYPE(Property*)



#endif // PROPERTY_H
