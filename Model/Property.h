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

#include "Element.h"
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

    virtual bool isAttributeProperty() const { return false; }
    virtual bool isALinkProperty()     const { return false; }
    virtual bool isEcoreContainment()  const { return false; }
    virtual bool isEcoreContainer()    const { return false; }

    virtual QVariant createNewInitValue() = 0;

    virtual void serializeAsXmiAttribute(XmiWriter *xmiWriter, Element *element) = 0;
    virtual void deserializeFromXmiAttribute(Element *const element, const QString &xmiValue) = 0;

    virtual void updateValue(Element *const element, QVariant value);
    virtual QVariant convertIntoUpdatableValue(QVariant value);

#ifdef __USE_HMI__
    virtual QWidget *getEditor(Element *const elem, QWidget *parent = nullptr) = 0;
    virtual QVariant getEditorUpdatedVariant(Element *const elem, QWidget *editor) = 0;
    virtual QValidator *getEditorValidator(QObject *parent) {Q_UNUSED(parent);return nullptr;}
#endif

    static ElemList getMapValuesInInsertionOrder(const ElemMap &map);

protected:
    Property(const QString &name, const char *label, bool isSerializable = true);

protected:
    const QString _name;
    const char   *_label;
    const QString _unit;
    const bool    _serializable;
};

template<typename TypeAttribute> class AttributeProperty : public Property{
public:
    AttributeProperty(const QString &name, const char *label, const TypeAttribute &defaultValue = TypeAttribute());
    virtual ~AttributeProperty() = default;

    bool isAttributeProperty() const override { return true; }

    void setDefaultValue(const TypeAttribute &defaultValue)  { _defaultValue = defaultValue; }
    bool getDefaultValue() const                        { return _defaultValue; }

    TypeAttribute getValue(Element *const element);
    void setValue(Element *const element, const TypeAttribute &value);

    // Mandatory function to be instanciable
    QVariant createNewInitValue() override { return _defaultValue; }

    virtual void serializeAsXmiAttribute(XmiWriter *xmiWriter, Element *element) override;
    virtual void deserializeFromXmiAttribute(Element *const element, const QString &xmiValue) override;

    QVariant convertIntoUpdatableValue(QVariant value) override {return Property::convertIntoUpdatableValue(value);}

#ifdef __USE_HMI__
    virtual QWidget *getEditor(Element *const elem, QWidget *parent = nullptr) override;
    virtual QVariant getEditorUpdatedVariant(Element *const elem, QWidget *editor) override;
    virtual QValidator *getEditorValidator(QObject *parent) override;
#endif

private:
    TypeAttribute _defaultValue;
};

class uIntProperty : public IntProperty{
public:
    uIntProperty(const QString &name, const char *label, const int &defaultValue = 0) : IntProperty(name, label, defaultValue){}
    ~uIntProperty() = default;
#ifdef __USE_HMI__
    QValidator *getEditorValidator(QObject *parent) override;
#endif
};

class uDoubleProperty : public DoubleProperty{
public:
    uDoubleProperty(const QString &name, const char *label, const double &defaultValue = 0) : DoubleProperty(name, label, defaultValue){}
    ~uDoubleProperty() = default;
#ifdef __USE_HMI__
    QValidator *getEditorValidator(QObject *parent) override;
#endif
};


class EnumProperty : public AttributeProperty<int>
{
public:
    EnumProperty(const QString &name, const char *label, int defaultValue = 0);
    ~EnumProperty() = default;

    QString getValueAsString(Element *const element);
    void setEnumValues(const QMap<int,QString> &enumValues);

    QMap<int, QString> getEnumValues();
    QString getEnumValueByKey(int key) const;
    int getEnumKeyByValue(QString value) const;
    QStringList getAllValues();

    // Mandatory function to be instanciable
    void serializeAsXmiAttribute(XmiWriter *xmiWriter, Element *element) override;
    void deserializeFromXmiAttribute(Element *const element, const QString &xmiValue) override;

#ifdef __USE_HMI__
    virtual QWidget *getEditor(Element *const elem, QWidget *parent = nullptr) override;
    virtual QVariant getEditorUpdatedVariant(Element *const elem, QWidget *editor) override;
#endif

private:
    QMap<int, QString> _enumValues;
};




// pure virtual class
class LinkProperty : public Property
{
public:
    bool isALinkProperty()              const override { return true; }
    virtual bool isOrdered()            const = 0;
    virtual bool isMapProperty()        const = 0;
    virtual bool isMandatory()          const = 0;
    virtual bool isALinkToOneProperty() const = 0;
    bool isALinkToManyProperty() const ;

    virtual ~LinkProperty() = default;

    virtual void updateValue(Element *const element, QVariant value) = 0;

    // Handy setter from a LinkToMany properties
    virtual void updateValue(Element *const element, ElemList &values) = 0;

    virtual void addLink(Element *const element, Element *const elementToAdd) = 0;
    virtual void removeLink(Element *const element, Element *const elementToRemove) = 0;
    virtual ElemList getLinkedElements(Element *const element, bool ordered = false) = 0;
    virtual void setValues(Element *element, const ElemList &values) = 0;

    virtual ElemMap *getLinkedElementsMap(Element *const element) {Q_UNUSED(element); return nullptr;}
    virtual void setValuesFromMap(Element *element, ElemMap *values) {Q_UNUSED(element);Q_UNUSED(values);}

    bool isEcoreContainment() const {return _isEcoreContainment;}
    void setEcoreContainment(bool isEcoreContainment) {_isEcoreContainment = isEcoreContainment;}

    bool isEcoreContainer() const;

    LinkProperty* getReverseLinkProperty() const { return _reverseLinkProperty; }
    void setReverseLinkProperty(LinkProperty *const reverseLinkProperty);
    void updateReverseProperty(Element* element, Element* value);

    ElementType* getElementType() const { return _elementType; }
    ElementType* getLinkedElementType() const { return _linkedElementType; }

    virtual void serializeAsXmiAttribute(XmiWriter *xmiWriter, Element *element) override;
    void deserializeFromXmiAttribute(Element *element, const QString &xmiValue) override;

    virtual void setValueFromXMIStringIdList(Element *element, const QString &ids, Model *model) = 0;

#ifdef __USE_HMI__
    virtual QWidget *getEditor(Element *const elem, QWidget *parent = nullptr) override {Q_UNUSED(elem);return new QWidget(parent);}
    virtual QVariant getEditorUpdatedVariant(Element *const elem, QWidget *editor) override {Q_UNUSED(elem);Q_UNUSED(editor); return QVariant();}
#endif

protected:
    LinkProperty(ElementType *const eltType, ElementType *const linkedEltType, const QString &name, const char *label, bool isSerializable = true);

protected:
    ElementType  *_elementType;       // ElementType to which this LinkProperty belongs
    ElementType  *_linkedElementType; // ElementType linked through this LinkProperty
    bool         _isEcoreContainment;
    LinkProperty *_reverseLinkProperty;
};


class LinkToOneProperty : public LinkProperty
{
public:
    virtual bool isOrdered()            const override { return false; }
    virtual bool isMapProperty()        const override { return false; }
    virtual bool isMandatory()          const override { return false; }
    virtual bool isALinkToOneProperty() const override { return true; }

    LinkToOneProperty(ElementType *const eltType, ElementType *const linkedEltType, const QString &name, const char *label, bool isSerializable = true):
        LinkProperty(eltType, linkedEltType, name, label, isSerializable){}
    virtual ~LinkToOneProperty() = default;

    QVariant createNewInitValue() override;

    void updateValue(Element *const element, QVariant value) override;
    void updateValue(Element *const element, ElemList &values) override;

    void addLink(Element *const element, Element *const elementToAdd) override;
    void removeLink(Element *const element, Element *const elementToRemove) override;
    ElemList getLinkedElements(Element *const element, bool ordered) override;

    Element* getValue(Element *const element);
    void setValue(Element *const element, Element *const value);
    void setValues(Element *element, const ElemList &values) override;

    void setValueFromXMIStringIdList(Element *element, const QString &ids, Model *model) override;

#ifdef __USE_HMI__
    virtual QWidget *getEditor(Element *const elem, QWidget *parent = nullptr) override;
    virtual QVariant getEditorUpdatedVariant(Element *const elem, QWidget *editor) override;
#endif
};

class Link11Property : public LinkToOneProperty
{
public:
    bool isMandatory() const override { return true; }

    Link11Property(ElementType *const eltType, ElementType *const linkedEltType, const QString &name, const char *label, bool isSerializable = true):
        LinkToOneProperty(eltType, linkedEltType, name, label, isSerializable) {}

    ~Link11Property() = default;
};



template <template <typename...> class Container, typename... Args> class GenericLinkToManyProperty : public LinkProperty {
public:
    virtual bool isOrdered()            const override { return true; }
    virtual bool isMapProperty()        const override { return false; }
    virtual bool isMandatory()          const override { return false; }
    virtual bool isALinkToOneProperty() const override { return false; }

    GenericLinkToManyProperty(ElementType *const eltType, ElementType *const linkedEltType, const QString &name, const char *label, bool isSerializable = true):
        LinkProperty(eltType, linkedEltType, name, label, isSerializable) {}
    virtual ~GenericLinkToManyProperty() = default;

    QVariant createNewInitValue() override;

    virtual void addLink(Element *const element, Element *const elementToAdd) override;
    virtual void removeLink(Element *const element, Element *const elementToRemove) override;
    virtual void updateValue(Element *const element, QVariant value) override;
    virtual ElemList getLinkedElements(Element *const element, bool ordered) override;

    Container<Args..., Element*> *getValues(Element *const element) ;
    void setValues(Element *element, Container<Args..., Element*> *values);
    void setValues(Element *element, const ElemList &values) override;

    // Handy setter from a QSet
    void updateValue(Element *const element, ElemList &values) override;

    virtual ElemMap *getLinkedElementsMap(Element *const element) override {Q_UNUSED(element);return nullptr;}
    virtual void setValuesFromMap(Element *element, ElemMap *values) override {Q_UNUSED(element);Q_UNUSED(values);}

    virtual void serializeAsXmiAttribute(XmiWriter *xmiWriter, Element *element) override;
    virtual void setValueFromXMIStringIdList(Element *element, const QString &ids, Model *model) override;
};

template <> inline bool MapLinkProperty::isMapProperty()      const {return true;}
template <> inline bool MultiMapLinkProperty::isMapProperty() const {return true;}
template <> inline bool LinkToManyProperty::isOrdered()       const {return false;}

/*
class LinkToOneProperty : public GenericLinkToManyProperty<QSet>
{
public:
    bool isALinkToOneProperty() const override { return true; }

    LinkToOneProperty(ElementType *const eltType, ElementType *const linkedEltType, const QString &name, const char *label, bool isSerializable = true):
        GenericLinkToManyProperty<QSet>(eltType, linkedEltType, name, label, isSerializable){}

    virtual ~LinkToOneProperty() = default;

    Element* getValue(Element *const element);
    void setValue(Element *const element, Element *const value);
    void addLink(Element *const element, Element *const elementToAdd) override;
    void removeLink(Element *const element, Element *const elementToRemove) override;
    void updateValue(Element *const element, QVariant value) override;

    void setValueFromXMIStringIdList(Element *element, const QString &ids, Model *model) override;

#ifdef __USE_HMI__
    virtual QWidget *getEditor(Element *const elem, QWidget *parent = nullptr) override;
    virtual QVariant getEditorUpdatedVariant(Element *const elem, QWidget *editor) override;
#endif
};

class Link11Property : public LinkToOneProperty
{
public:
    bool isMandatory() const override { return true; }

    Link11Property(ElementType *const eltType, ElementType *const linkedEltType, const QString &name, const char *label, bool isSerializable = true):
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
    return QVariant::fromValue(new Container<Args..., Element*>());
}
template <template <typename...> class Container, typename... Args>
    Container<Args..., Element*> * GenericLinkToManyProperty<Container, Args...>::getValues(Element *const element)
{
    return element->getLinkPropertyValue<Container<Args..., Element*>>(this);
}
template <template <typename...> class Container, typename... Args>
    void GenericLinkToManyProperty<Container, Args...>::setValues(Element *element, Container<Args..., Element*> *values)
{
    element->setLinkToManyPropertyValue<Container<Args..., Element*>>(this, values);
}
template <template <typename...> class Container, typename... Args>
    void GenericLinkToManyProperty<Container, Args...>::addLink(Element *const element, Element *const elementToAdd)
{
    element->addALinkToMany<Container, Args...>(this, elementToAdd);
}
template <template <typename...> class Container, typename... Args>
    void GenericLinkToManyProperty<Container, Args...>::removeLink(Element *const element, Element *const elementToRemove)
{
    element->removeALinkFromMany<Container, Args...>(this, elementToRemove);
}
template <template <typename...> class Container, typename... Args>
    void GenericLinkToManyProperty<Container, Args...>::serializeAsXmiAttribute(XmiWriter *xmiWriter, Element *element)
{
    xmiWriter->addAttribute(_name, getLinkedElements(element, true));
}

//////////////////////////////////////////////////////////
// Template specializations for GenericLinkToManyProperty
//////////////////////////////////////////////////////////

// Template specializations of getLinkedElements for QSet, QList, QMap and QMultiMap
template <> inline ElemList LinkToManyProperty::getLinkedElements(Element *const element, bool ordered)
{
    if (ordered)
    {
        ElemList list = getValues(element)->toList();
        std::sort(list.begin(), list.end(), &Element::elementIdLessThan);
        return list;
    }
    else
        return getValues(element)->toList();
}
template <> inline ElemList OrderedLinkToManyProperty::getLinkedElements(Element *const element, bool ordered)
{
    Q_UNUSED(ordered)
    return *(getValues(element));
}
template <> inline ElemList MapLinkProperty::getLinkedElements(Element *const element, bool ordered)
{
    Q_UNUSED(ordered)
    return getValues(element)->values();
}
template <> inline ElemList MultiMapLinkProperty::getLinkedElements(Element *const element, bool ordered)
{
    Q_UNUSED(ordered)
    return getValues(element)->values();
}

template <> inline ElemMap *MapLinkProperty::getLinkedElementsMap(Element *const element)
{
    return getValues(element);
}
template <> inline  void MapLinkProperty::setValuesFromMap(Element *element, ElemMap *values)
{
    setValues(element, values);
}
template <> inline ElemMap *MultiMapLinkProperty::getLinkedElementsMap(Element *const element)
{
    return getValues(element);
}
template <> inline  void MultiMapLinkProperty::setValuesFromMap(Element *element, ElemMap *values)
{
    ElemMultiMap multiMap = *values;
    setValues(element, &multiMap);
}

// Template specializations of setValue with ElemList for QSet, QList, QMap and QMultiMap
template <> inline void LinkToManyProperty::setValues(Element *element, const ElemList &values)
{
    ElemSet elemSet = values.toSet();
    setValues(element, &elemSet);
}
template <> inline void OrderedLinkToManyProperty::setValues(Element *element, const ElemList &values)
{
    ElemList elemList = values;
    setValues(element, &elemList);
}
template <> inline void MapLinkProperty::setValues(Element *element, const ElemList &values)
{
    ElemMap map;
    for (Element *val : values)
        map.insert(val->getPropertyMapKey(this), val);
    setValues(element, &map);
}
template <> inline void MultiMapLinkProperty::setValues(Element *element, const ElemList &values)
{
    ElemMultiMap map;
    for (Element *val : values)
        map.insert(val->getPropertyMapKey(this), val);
    setValues(element, &map);
}


// Template specializations of updateValue for QSet, QList, QMap and QMultiMap
template <> inline void LinkToManyProperty::updateValue(Element *const element, QVariant value)
{
    if (!value.canConvert<ElemSet* >())
        return;

    ElemSet *newValueSet = value.value<ElemSet*>(),
            *oldValueSet = element->getLinkPropertyValue<ElemSet>(this);
    if (*newValueSet == *oldValueSet)
        return;

    if (_reverseLinkProperty)
    {
        for (Element *linkedElement : *newValueSet)
        {
            if (!oldValueSet->contains(linkedElement))
                _reverseLinkProperty->addLink(linkedElement, element);
        }

        for (Element *linkedElement : *oldValueSet)
        {
            if(!newValueSet->contains(linkedElement))
                _reverseLinkProperty->removeLink(linkedElement, element);
        }
    }

    // As we do a swap for efficiency we need to do this at the end
    setValues(element, newValueSet);
}
template <> inline void OrderedLinkToManyProperty::updateValue(Element *const element, QVariant value)
{
    if (!value.canConvert<ElemList* >())
        return;

    ElemList *newValueList = value.value<ElemList*>(),
            *oldValueList = element->getLinkPropertyValue<ElemList>(this);
    if (*newValueList == *oldValueList)
        return;

    if (_reverseLinkProperty)
    {
        for (Element *linkedElement : *newValueList)
        {
            if (!oldValueList->contains(linkedElement))
                _reverseLinkProperty->addLink(linkedElement, element);
        }

        for (Element *linkedElement : *oldValueList)
        {
            if(!newValueList->contains(linkedElement))
                _reverseLinkProperty->removeLink(linkedElement, element);
        }
    }

    setValues(element, newValueList);
}
template <> inline void MapLinkProperty::updateValue(Element *const element, QVariant value)
{
    if (!value.canConvert<ElemMap*>())
        return;

    ElemMap *newValueMap = value.value<ElemMap*>(),
            *oldValueMap = element->getLinkPropertyValue<ElemMap>(this);
    if (*newValueMap == *oldValueMap)
        return;

    if (_reverseLinkProperty)
    {
        for (auto it = newValueMap->cbegin() ; it != newValueMap->cend(); ++it)
        {
            if (!oldValueMap->contains(it.key()))
                _reverseLinkProperty->addLink(it.value(), element);
        }

        for (auto it = oldValueMap->cbegin() ; it != oldValueMap->cend(); ++it)
        {
            if(!newValueMap->contains(it.key()))
                _reverseLinkProperty->removeLink(it.value(), element);
        }
    }

    setValues(element, newValueMap);
}
template <> inline void MultiMapLinkProperty::updateValue(Element *const element, QVariant value)
{
    if (!value.canConvert<ElemMultiMap*>())
        return;

    ElemMultiMap *newValueMap = value.value<ElemMultiMap*>(),
            *oldValueMap = element->getLinkPropertyValue<ElemMultiMap>(this);
    if (*newValueMap == *oldValueMap)
        return;

    if (_reverseLinkProperty)
    {
        for (auto it = newValueMap->cbegin() ; it != newValueMap->cend(); ++it)
        {
            if (!oldValueMap->contains(it.key(), it.value()))
                _reverseLinkProperty->addLink(it.value(), element);
        }

        for (auto it = oldValueMap->cbegin() ; it != oldValueMap->cend(); ++it)
        {
            if(!newValueMap->contains(it.key(), it.value()))
                _reverseLinkProperty->removeLink(it.value(), element);
        }
    }
    setValues(element, newValueMap);
}



template <> inline void LinkToManyProperty::updateValue(Element *const element, ElemList &values)
{
    ElemSet set(values.toSet());
    updateValue(element, QVariant::fromValue(&set));

    values = set.toList();
}
template <> inline void OrderedLinkToManyProperty::updateValue(Element *const element, ElemList &values)
{
    updateValue(element, QVariant::fromValue(&values));
}
template <> inline void MapLinkProperty::updateValue(Element *const element, ElemList &values)
{
    ElemMap map;
    for (Element *val : values)
        map[val->getPropertyMapKey(this)] = val;
    updateValue(element, QVariant::fromValue(&map));

    values.clear();
    for (auto it = map.cbegin() ; it != map.cend() ; ++it)
        values.append(it.value());
}

template <> inline void MultiMapLinkProperty::updateValue(Element *const element, ElemList &values)
{
    ElemMultiMap map;
    for (Element *val : values)
        map.insert(val->getPropertyMapKey(this), val);
    updateValue(element, QVariant::fromValue(&map));

    values.clear();
    for (auto it = map.cbegin() ; it != map.cend() ; ++it)
        values.append(it.value());
}


template <> inline void LinkToManyProperty::setValueFromXMIStringIdList(Element *element, const QString &ids, Model *model)
{
    ElemSet elementsToLink;
    ElementType *linkedEltType = element->getLinkedElementType(this);
    for (const QString &id_ : ids.split(" "))
    {
        QString id = id_.trimmed();
        if (!id.isEmpty())
        {
            Element *linkedElement = model->getElementById(linkedEltType, id);
            if (linkedElement)
                elementsToLink.insert(linkedElement);
        }
    }
    if (!elementsToLink.isEmpty())
        updateValue(element, QVariant::fromValue(&elementsToLink));
}
template <> inline void OrderedLinkToManyProperty::setValueFromXMIStringIdList(Element *element, const QString &ids, Model *model)
{
    ElemList elementsToLink;
    ElementType *linkedEltType = element->getLinkedElementType(this);
    for (const QString &id_ : ids.split(" "))
    {
        QString id = id_.trimmed();
        if (!id.isEmpty())
        {
            Element *linkedElement = model->getElementById(linkedEltType, id);
            if (linkedElement)
                elementsToLink.append(linkedElement);
        }
    }
    if (!elementsToLink.isEmpty())
        updateValue(element, QVariant::fromValue(&elementsToLink));
}
template <> inline void MapLinkProperty::setValueFromXMIStringIdList(Element *element, const QString &ids, Model *model)
{
    ElemMap elementsToLink;
    ElementType *linkedEltType = element->getLinkedElementType(this);
    for (const QString &id_ : ids.split(" "))
    {
        QString id = id_.trimmed();
        if (!id.isEmpty())
        {
            Element *linkedElement = model->getElementById(linkedEltType, id);
            if (linkedElement)
                elementsToLink[linkedElement->getPropertyMapKey(this)] = linkedElement;
        }
    }
    if (!elementsToLink.isEmpty())
        updateValue(element, QVariant::fromValue(&elementsToLink));
}
template <> inline void MultiMapLinkProperty::setValueFromXMIStringIdList(Element *element, const QString &ids, Model *model)
{
    ElemMultiMap elementsToLink;
    ElementType *linkedEltType = element->getLinkedElementType(this);
    for (const QString &id_ : ids.split(" "))
    {
        QString id = id_.trimmed();
        if (!id.isEmpty())
        {
            Element *linkedElement = model->getElementById(linkedEltType, id);
            if (linkedElement)
                elementsToLink.insert(linkedElement->getPropertyMapKey(this), linkedElement);
        }
    }
    if (!elementsToLink.isEmpty())
        updateValue(element, QVariant::fromValue(&elementsToLink));
}

template <> inline void MapLinkProperty::serializeAsXmiAttribute(XmiWriter *xmiWriter, Element *element)
{
    ElemMap *map = getValues(element);
    xmiWriter->addAttribute(_name, getMapValuesInInsertionOrder(*map));
}
template <> inline void MultiMapLinkProperty::serializeAsXmiAttribute(XmiWriter *xmiWriter, Element *element)
{
    ElemMap *map = getValues(element);
    xmiWriter->addAttribute(_name, getMapValuesInInsertionOrder(*map));
}

////////////////////////////////////////////////////////////////
// Template definitions for AttributeProperty (nothing to touch)
////////////////////////////////////////////////////////////////

template<typename TypeAttribute>
AttributeProperty<TypeAttribute>::AttributeProperty(const QString &name, const char *label, const TypeAttribute &defaultValue):
    Property(name, label), _defaultValue(defaultValue){}

template<typename TypeAttribute>
TypeAttribute AttributeProperty<TypeAttribute>::getValue(Element * const element)
{
    return element->getPropertyValue<TypeAttribute>(this);
}

template<typename TypeAttribute>
void AttributeProperty<TypeAttribute>::setValue(Element * const element, const TypeAttribute &value)
{
    element->setPropertyValueFromQVariant(this, QVariant::fromValue(value));
}

template<typename TypeAttribute>
void AttributeProperty<TypeAttribute>::serializeAsXmiAttribute(XmiWriter *xmiWriter, Element *element)
{
    TypeAttribute value = getValue(element);
    if (value != _defaultValue)
        xmiWriter->addAttribute(_name, value);
}

#ifdef __USE_HMI__
template<typename TypeAttribute>
QWidget *AttributeProperty<TypeAttribute>::getEditor(Element * const elem, QWidget *parent)
{
    QLineEdit *lineEdit = new QLineEdit(parent);
    lineEdit->setText(QString("%1").arg(getValue(elem)));

    QValidator *validator = getEditorValidator(lineEdit);
    lineEdit->setValidator(validator);
    return lineEdit;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// Template specializations for AttributeProperty (add one for each new type)
//////////////////////////////////////////////////////////////////////////////

template<> inline void BoolProperty::deserializeFromXmiAttribute(Element *element, const QString &xmiValue)
{
    if (xmiValue.isEmpty())
        setValue(element, _defaultValue);
    else
    {
        bool value = (xmiValue == "true");
        setValue(element, value);
    }
}

template<> inline void DoubleProperty::deserializeFromXmiAttribute(Element *element, const QString &xmiValue)
{
    if (xmiValue.isEmpty())
        setValue(element, _defaultValue);
    else
        setValue(element, xmiValue.toDouble());
}

template<> inline void FloatProperty::deserializeFromXmiAttribute(Element *element, const QString &xmiValue)
{
    if (xmiValue.isEmpty())
        setValue(element, _defaultValue);
    else
        setValue(element, xmiValue.toFloat());
}


template<> inline void IntProperty::deserializeFromXmiAttribute(Element *const element, const QString &xmiValue)
{
    if (xmiValue.isEmpty())
        setValue(element, _defaultValue);
    else
        setValue(element, xmiValue.toInt());
}

template<> inline QVariant StringProperty::convertIntoUpdatableValue(QVariant value)
{
    return QVariant(value.toString().replace(";","-"));
}


template<> inline void StringProperty::deserializeFromXmiAttribute(Element *element, const QString &xmiValue)
{
    setValue(element, xmiValue);
}

template<> inline void AttributeProperty<QDateTime>::deserializeFromXmiAttribute(Element *element, const QString &xmiValue)
{
    setValue(element, QDateTime::fromString(xmiValue, "yyyy/MM/dd hh:mm:ss"));
}


#ifdef __USE_HMI__
template<> inline QVariant BoolProperty::getEditorUpdatedVariant(Element * const elem, QWidget *editor)
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    bool inputOk = false;
    int newVal = lineEdit->text().toInt(&inputOk);
    if (inputOk && (newVal != getValue(elem)))
        return newVal;
    else
        return QVariant();
}
template<> inline QVariant DoubleProperty::getEditorUpdatedVariant(Element * const elem, QWidget *editor)
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    bool inputOk = true;
    QString str(lineEdit->text());
    double newVal;
    if (str == "-∞")
        newVal = -1;
    else if (str == "+∞")
        newVal = -2;
    else
        newVal = str.toDouble(&inputOk);
    if (inputOk && (newVal != getValue(elem)))
        return newVal;
    else
        return QVariant();
}
template<> inline QVariant FloatProperty::getEditorUpdatedVariant(Element * const elem, QWidget *editor)
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    bool inputOk = true;
    QString str(lineEdit->text());
    float newVal;
    if (str == "-∞")
        newVal = -1;
    else if (str == "+∞")
        newVal = -2;
    else
        newVal = str.toFloat(&inputOk);
    if (inputOk && (newVal != getValue(elem)))
        return newVal;
    else
        return QVariant();
}
template<> inline QVariant IntProperty::getEditorUpdatedVariant(Element * const elem, QWidget *editor)
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    bool inputOk = true;
    QString str(lineEdit->text());
    int newVal;
    if (str == "-∞")
        newVal = -1;
    else if (str == "+∞")
        newVal = -2;
    else
        newVal = str.toInt(&inputOk);
//    int newVal = lineEdit->text().toInt(&inputOk);
    if (inputOk && (newVal != getValue(elem)))
        return newVal;
    else
        return QVariant();
}
template<> inline QVariant StringProperty::getEditorUpdatedVariant(Element * const elem, QWidget *editor)
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    if (lineEdit->text() != getValue(elem))
        return lineEdit->text();
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


template <> inline QWidget *DoubleProperty::getEditor(Element * const elem, QWidget *parent)
{
    QLineEdit *lineEdit = new QLineEdit(parent);
    double val = getValue(elem);
    QString valStr;
    if (val == -1)
        valStr = "-∞";
    else if (val == -2)
        valStr = "+∞";
    else
        valStr = QString::number(val);

    lineEdit->setText(valStr);

    QValidator *validator = getEditorValidator(lineEdit);
    lineEdit->setValidator(validator);
    return lineEdit;
}
template <> inline QWidget *IntProperty::getEditor(Element * const elem, QWidget *parent)
{
    QLineEdit *lineEdit = new QLineEdit(parent);
    int val = getValue(elem);
    QString valStr;
    if (val == -1)
        valStr = "-∞";
    else if (val == -2)
        valStr = "+∞";
    else
        valStr = QString::number(val);

    lineEdit->setText(valStr);

    QValidator *validator = getEditorValidator(lineEdit);
    lineEdit->setValidator(validator);
    return lineEdit;
}
template <> inline QWidget *FloatProperty::getEditor(Element * const elem, QWidget *parent)
{
    QLineEdit *lineEdit = new QLineEdit(parent);
    float val = getValue(elem);
    QString valStr;
    if (val == -1)
        valStr = "-∞";
    else if (val == -2)
        valStr = "+∞";
    else
        valStr = QString::number(val);

    lineEdit->setText(valStr);

    QValidator *validator = getEditorValidator(lineEdit);
    lineEdit->setValidator(validator);
    return lineEdit;
}
#endif

Q_DECLARE_METATYPE(LinkProperty*)
Q_DECLARE_METATYPE(Property*)



#endif // PROPERTY_H
