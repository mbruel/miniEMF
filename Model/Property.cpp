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

#include "Property.h"
#include "Element.h"



#include <QCoreApplication>
Property::Property(const QString &name, const char *label, bool isSerializable):
    _name(name), _label(label), _unit(), _serializable(isSerializable)
{}

QString Property::getLabel() const { return QCoreApplication::translate("Property", _label);} //QObject::tr(_label); }

void Property::updateValue(Element *const element, QVariant value)
{
    element->setPropertyValueFromQVariant(this, value);
}

QVariant Property::convertIntoUpdatableValue(QVariant value) { return value; }

ElemList Property::getMapValuesInInsertionOrder(const ElemMap &map)
{
    ElemList values;
    for (const QVariant & key : map.uniqueKeys())
    {
        ElemList elems = map.values(key);
        auto it = elems.cend(), itStart = elems.cbegin();
        do
        { // we do it backwards cause insertMulti insert duplicate in front and values send back in the same order.
            --it;
            values.append(*it);
        } while (it != itStart);
    }
    return values;
}






// #######################
// #### ENUM PROPERTY ####

EnumProperty::EnumProperty(const QString &name, const char *label, int defaultValue) :
    AttributeProperty<int>(name, label, defaultValue), _enumValues(){}


QString EnumProperty::getValueAsString(Element *element)
{
    return QCoreApplication::translate("Constant", _enumValues.value(getValue(element)).toStdString().c_str());
}

void EnumProperty::setEnumValues(const QMap<int, QString> &enumValues)
{
    _enumValues = enumValues;
}

QMap<int,QString> EnumProperty::getEnumValues()
{
    return _enumValues;
}

QStringList EnumProperty::getAllValues()
{
    return _enumValues.values();
}

void EnumProperty::serializeAsXmiAttribute(XmiWriter *xmiWriter, Element *element)
{
    xmiWriter->addAttribute(_name, getEnumValueByKey(getValue(element)));
}

void EnumProperty::deserializeFromXmiAttribute(Element *element, const QString &xmiValue)
{
    setValue(element, getEnumKeyByValue(xmiValue));
}

QString EnumProperty::getEnumValueByKey(int key) const
{
    return QCoreApplication::translate("Constant", _enumValues.value(key).toStdString().c_str());
}

int EnumProperty::getEnumKeyByValue(QString value) const
{
    return _enumValues.key(value);
}

#ifdef __USE_HMI__
QWidget *EnumProperty::getEditor(Element * const elem, QWidget *parent)
{
    QComboBox *combobox = new QComboBox(parent);
    ushort index = 0;
    int currentValue = getValue(elem), currentValueIndex = 0;
    for (auto it = _enumValues.cbegin() ; it != _enumValues.cend() ; ++it)
    {
        combobox->addItem(it.value(), it.key());
        if (it.key() == currentValue)
            currentValueIndex = index;
        ++index;
    }
    combobox->setCurrentIndex(currentValueIndex);
    return combobox;
}

QVariant EnumProperty::getEditorUpdatedVariant(Element * const elem, QWidget *editor)
{
    QComboBox *combobox = static_cast<QComboBox*>(editor);
    int newVal = combobox->currentData().toInt(), oldVal = getValue(elem);
    if (newVal != oldVal)
        return combobox->currentData();
    else
        return QVariant();
}
#endif



// #######################
// #### LINK PROPERTY ####

LinkProperty::LinkProperty(ElementType *const eltType,
                           ElementType *const linkedEltType,
                           const QString &name,
                           const char *label,
                           bool isMandatory,
                           bool isSerializable) :
    Property(name, label, isSerializable),
    _elementType(eltType),
    _linkedElementType(linkedEltType),
    _isEcoreContainment(false),
    _reverseLinkProperty(nullptr),
    _isMandatory(isMandatory)
{}


bool LinkProperty::isALinkToManyProperty() const
{
    return !isALinkToOneProperty();
}

bool LinkProperty::isEcoreContainer() const
{
    LinkProperty *reverseLinkProperty = getReverseLinkProperty();
    if (reverseLinkProperty)
        return reverseLinkProperty->isEcoreContainment();

    return false;
}

void LinkProperty::serializeAsXmiAttribute(XmiWriter *xmiWriter, Element *element)
{
    xmiWriter->addAttribute(_name, getLinkedElements(element, true));
}


void LinkProperty::setReverseLinkProperty(LinkProperty *reverseLinkProperty)
{
    _reverseLinkProperty = reverseLinkProperty;
//    this->elementType = reverseLinkProperty->getLinkedElementType();
}


void LinkProperty::deserializeFromXmiAttribute(Element *element, const QString &xmiValue)
{
    Q_UNUSED(element);
    Q_UNUSED(xmiValue);
    // No action : the deserialization is managed directly by ElementDao::deserializeElement(QDomNode node)
}

void LinkProperty::validateElement(Element *element, QStringList &ecoreErrors)
{
    if (_isMandatory && getLinkedElements(element).isEmpty())
        ecoreErrors << QString("Error on '%1': the property '%2' should not be null...").arg(
                                element->getName()).arg(getLabel());
}


// ############################
// #### LinkToOne PROPERTY ####

QVariant LinkToOneProperty::createNewInitValue()
{
    Element *dummy = nullptr;
    return QVariant::fromValue(dummy);
}


void LinkToOneProperty::setValues(Element *element, const ElemList &values)
{
    Element *value = nullptr;
    if (!values.isEmpty())
        value = *(values.begin());
    setValue(element, value);
}

void LinkToOneProperty::setValueFromXMIStringIdList(Element *element, const QString &ids, Model *model)
{
    Element *linkedElt = model->getElementById(_linkedElementType, ids.trimmed());
    if (linkedElt)
        updateValue(element, QVariant::fromValue(linkedElt));
}

Element* LinkToOneProperty::getValue(Element* element)
{
    if(element)
        return element->getLinkPropertyValue<Element>(this);

    return nullptr;
}

void LinkToOneProperty::setValue(Element* element, Element* value)
{
    element->setPropertyValueFromElement(this, value);
}

void LinkToOneProperty::updateValue(Element* element, QVariant value)
{
    if (element && value.canConvert<Element*>())
    {
        Element *newLink = static_cast<Element*>(value.value<Element*>());
        setValue(element, newLink);

        // update reverse property
        Element* oldLink = getValue(element);
        if (_reverseLinkProperty) // Example of linkProperty for which reverseLinkProperty is null : ConstraintSet -> Element)
        {
            if (oldLink)
                _reverseLinkProperty->removeLink(oldLink, element);
            if (newLink)
                _reverseLinkProperty->addLink(newLink, element);
        }
    }
}

void LinkToOneProperty::updateValue(Element * const element, ElemList &values)
{
    Element *value = nullptr;
    if (values.size())
        value = *(values.begin());
    updateValue(element, QVariant::fromValue(value));
}

void LinkToOneProperty::addLink(Element* element, Element* elementToAdd)
{
    setValue(element, elementToAdd);
}

void LinkToOneProperty::removeLink(Element* element, Element* elementToRemove)
{
    Q_UNUSED(elementToRemove);
    setValue(element, nullptr);
}


QList<Element*> LinkToOneProperty::getLinkedElements(Element* element, bool ordered)
{
    Q_UNUSED(ordered)
    QList<Element*> linkedElements;
    Element *linkedElement = getValue(element);
    if (linkedElement)
        linkedElements.append(linkedElement);

    return linkedElements;
}

/*
Element *LinkToOneProperty::getValue(Element * const element)
{
    ElemSet *values = getValues(element);
    if (values->isEmpty())
        return nullptr;
    else
        return *(values->begin());
}

void LinkToOneProperty::setValue(Element * const element, Element * const value)
{
    ElemSet values;
    if (value)
        values.insert(value);
    setValues(element, &values);
}

void LinkToOneProperty::addLink(Element* element, Element* elementToAdd)
{
    setValue(element, elementToAdd);
}

void LinkToOneProperty::removeLink(Element* element, Element* elementToRemove)
{
    Q_UNUSED(elementToRemove);
    setValue(element, nullptr);
}

void LinkToOneProperty::updateValue(Element* element, QVariant value)
{
    if (element && value.canConvert<Element*>())
    {
        Element *newLink = static_cast<Element*>(value.value<Element*>());

        // update reverse property
        Element* oldLink = getValue(element);
        if (_reverseLinkProperty) // Example of linkProperty for which reverseLinkProperty is null : ConstraintSet -> Element)
        {
            if (oldLink)
                _reverseLinkProperty->removeLink(oldLink, element);
            if (newLink)
                _reverseLinkProperty->addLink(newLink, element);
        }

        setValue(element, newLink);
    }
}

void LinkToOneProperty::setValueFromXMIStringIdList(Element *element, const QString &ids, Model *model)
{
    Element *linkedElt = model->getElementById(_linkedElementType, ids.trimmed());
    if (linkedElt)
        updateValue(element, QVariant::fromValue(linkedElt));
}
*/

#ifdef __USE_HMI__
#include "Cosi7Application.h"
QWidget *LinkToOneProperty::getEditor(Element * const elem, QWidget *parent)
{
    QComboBox *combobox = new QComboBox(parent);
    ushort index = 0;
    Element *currentValue = getValue(elem);
    Model *model = Cosi7Application::instance()->getModel();
    if (model)
    {
        if (!isMandatory() || !currentValue)
        {
            Element *dummyElem = nullptr;
            combobox->addItem("", QVariant::fromValue(dummyElem));
            ++index;
        }
        int currentValueIndex = 0;
        for (Element *e : model->getElementsOrderedByNames(_linkedElementType))
        {
            combobox->addItem(e->getName(), QVariant::fromValue(e));
            if (e == currentValue)
                currentValueIndex = index;
            ++index;
        }
        combobox->setCurrentIndex(currentValueIndex);
    }
    return combobox;
}

QVariant LinkToOneProperty::getEditorUpdatedVariant(Element * const elem, QWidget *editor)
{
    QComboBox *combobox = static_cast<QComboBox*>(editor);
    Element *newVal = combobox->currentData().value<Element*>(), *oldVal = getValue(elem);
    if (newVal != oldVal)
        return combobox->currentData();
    else
        return QVariant();
}
#endif
