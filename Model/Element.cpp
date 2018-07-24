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

#include <QDebug>
#include <QString>

#include "Element.h"
#include "Property.h"
#include "Model.h"
#include "Utils/XmiWriter.h"


ElementType    *Element::TYPE        = nullptr;

StringProperty *Element::PROPERTY_NAME        = nullptr;
StringProperty *Element::PROPERTY_DESCRIPTION = nullptr;

QMap<QString, Property *> *Element::initPropertyMap()
{
    QMap<QString, Property*>* propertyMap = new QMap<QString, Property*>();
    addPropertyToMap(propertyMap, Element::PROPERTY_NAME);
    addPropertyToMap(propertyMap, Element::PROPERTY_DESCRIPTION);
    return propertyMap;
}

void Element::addPropertyToMap(QMap<QString, Property *> *propertyMap, Property *property)
{
    (*propertyMap)[property->getName()] = property;
}


QString Element::getDescription() { return PROPERTY_DESCRIPTION->getValue(this); }
void Element::setDescription(const QString &value) { PROPERTY_DESCRIPTION->setValue(this, value);}

QString Element::getName() { return PROPERTY_NAME->getValue(this);}
void Element::setName(const QString &name){ PROPERTY_NAME->setValue(this, name);}
QString Element::getDefaultName()
{
    QString defaultName(getElementTypeName());
    defaultName += "_";
    defaultName += getId();

    return defaultName;
}

//Property *Element::getProperty(QString propertyName)
//{
//    return (*propertyMap)[propertyName];
//}



void Element::_initPropertyValueMap(QMap<QString, Property*>* classPropertyMap)
{
    auto itEnd = classPropertyMap->cend();
    for (auto it = classPropertyMap->cbegin(); it != itEnd; ++it) {
        Property *property = it.value();
        _propertyValueMap[property] = property->createNewInitValue();
    }
}




void Element::setPropertyValueFromQVariant(Property *property, const QVariant &value)
{
    _propertyValueMap.insert(property, value);
}

void Element::setPropertyValueFromElement(LinkProperty* property, Element* value)
{
    setPropertyValueFromQVariant(property, QVariant::fromValue(value));
}


QVariant Element::getPropertyMapKey(Property *mapProperty)
{
    Q_UNUSED(mapProperty);
    return getName();
}


QSet<LinkProperty*> Element::getLinkProperties()
{
    QSet<LinkProperty*> linkProperties;
    for (Property * property : _propertyValueMap.keys()){
        if (property->isALinkProperty())
            linkProperties.insert(static_cast<LinkProperty*>(property));
    }
    return linkProperties;
}

void Element::hideFromLinkedElements()
{
    for (LinkProperty *linkProperty : getLinkProperties())
    {
        if (!linkProperty->isEcoreContainment())
        {
            LinkProperty *reverseProperty = linkProperty->getReverseLinkProperty();
            if (reverseProperty)
            {
                for (Element *linkedElem : linkProperty->getLinkedElements(this))
                    reverseProperty->removeLink(linkedElem, this);
            }
        }
    }
}

void Element::makeVisibleForLinkedElements()
{
    for (LinkProperty *linkProperty : getLinkProperties())
    {
        if (!linkProperty->isEcoreContainment())
        {
            LinkProperty *reverseProperty = linkProperty->getReverseLinkProperty();
            if (reverseProperty)
            {
                for (Element *linkedElem : linkProperty->getLinkedElements(this))
                    reverseProperty->addLink(linkedElem, this);
            }
        }
    }
}




// ##### END MANAGEMENT OF PROPERTIES ####
// #######################################


Element::Element(QMap<QString, Property *> *classPropertyMap):
    _id(), _state(STATE::CREATED), _propertyValueMap()
{
    _initPropertyValueMap(classPropertyMap);
}

Element::~Element()
{
// Hide from LinkedElement should be done in the Children destructor
// as they may reimplement getPropertyMapKey
    //    hideFromLinkedElements();
}

Element *Element::clone()
{
    ElementType *type = getElementType();
    Element *newElem = type->createElement();

    // Set up the id
    type->initElementWithDefaultValues(newElem);
//qDebug() << "[MB_TRACE][Element::clone] >>>>>>>>>> " << getName();
    // Copy the properties (deep copy)
    for (auto it = _propertyValueMap.cbegin() ; it != _propertyValueMap.cend() ; ++it)
    {
        Property *property = it.key();
        if (property->isAttributeProperty())
            newElem->setPropertyValueFromQVariant(property, it.value());
        else
        {
            LinkProperty *linkProperty = static_cast<LinkProperty*>(property);
            if (linkProperty->isALinkToOneProperty())
            {
                if (linkProperty->isEcoreContainment())
                {
                    Element *linkedElement = it.value().value<Element*>();
                    if (linkedElement)
                        linkedElement = linkedElement->clone();
                    newElem->setPropertyValueFromQVariant(property, QVariant::fromValue(linkedElement));
                }
                else
                    newElem->setPropertyValueFromQVariant(property, it.value());
            }
            else
            {
                if (linkProperty->isEcoreContainment())
                {
                    ElemList clonedLinkedElements;
                    for (Element *linkedElem : linkProperty->getLinkedElements(this))
                        clonedLinkedElements.append(linkedElem->clone());

                    linkProperty->setValues(newElem, clonedLinkedElements);
                }
                else
                    linkProperty->setValues(newElem, linkProperty->getLinkedElements(this));
            }
        }
    }
//qDebug() << "[MB_TRACE][Element::clone] <<<<<<<<<< " << getName();
    return newElem;
}

Element *Element::shallowCopy()
{
    // create clone using the appropriate Type constructor
    Element *newElem = getElementType()->createElement();
    newElem->_id     = _id;
    newElem->_state  = STATE::CLONE;

    // we don't copy the properties as QVariant is a QSharedData
    return newElem;
}

void Element::copyPropertiesFromSourceElementWithCloneElements(Element *srcElem, Model *clonedModel)
{
    for (auto itProp = srcElem->_propertyValueMap.cbegin() ; itProp != srcElem->_propertyValueMap.cend() ; ++itProp)
    {
        Property *property = itProp.key();
        QVariant valueCopy = itProp.value();
        if (property->isAttributeProperty())
            _propertyValueMap[property] = valueCopy;
        else
        {
            LinkProperty *linkProperty      = static_cast<LinkProperty*>(property);
            ElementType  *linkedElementType = linkProperty->getLinkedElementType();
            if (linkProperty->isMapProperty())
            {
                ElemMap clonedLinkedElemMap, *srcLinkedElemMap = linkProperty->getLinkedElementsMap(srcElem);
                for (const QVariant & key : srcLinkedElemMap->uniqueKeys())
                {
                    QVariant keyCopy     = key;
                    ElemList linkedElems = srcLinkedElemMap->values(key);
                    auto it = linkedElems.cend(), itStart = linkedElems.cbegin();
                    do
                    { // we do it backwards cause insertMulti insert duplicate in front and values send back in the same order.
                        --it;
                        Element *srcLinkedElem = *it;
                        clonedLinkedElemMap.insertMulti(keyCopy, clonedModel->getElementById(linkedElementType, srcLinkedElem->getId()));
                    } while (it != itStart);
                }
//                for (auto it = srcLinkedElemMap->cbegin() ; it != srcLinkedElemMap->cend() ; ++it)
//                {
//                    QVariant keyCopy       = it.key();
//                    Element *srcLinkedElem = it.value();
//                    clonedLinkedElemMap.insertMulti(keyCopy, clonedModel->getElementById(linkedElementType, srcLinkedElem->getId()));
//                }
                linkProperty->setValuesFromMap(this, &clonedLinkedElemMap);
            }
            else
            {
                ElemList clonedLinkedElements;
                for (Element *srcLinkedElem : linkProperty->getLinkedElements(srcElem))
                    clonedLinkedElements.append(clonedModel->getElementById(linkedElementType, srcLinkedElem->getId()));
                linkProperty->setValues(this, clonedLinkedElements);
            }
        }
    }
}

ElementType* Element::getLinkedElementType(LinkProperty *linkProperty) const
{
    return linkProperty->getLinkedElementType();
}


void Element::serialize(XmiWriter *xmiWriter, const QString &tagName, const QString &xmiType)
{
    if (_state == STATE::REMOVED_FROM_MODEL)
        return;

    // Start
    xmiWriter->writeStartElement(tagName);
    if (!xmiType.isEmpty())
        xmiWriter->addAttribute("xsi:type", "PamDataModel:" + xmiType);
    xmiWriter->addAttribute("xmi:id", this->getId());

    // Non Containment / Container properties
    auto itStart = _propertyValueMap.cbegin(), itEnd = _propertyValueMap.cend();
    for (auto it = itStart ; it != itEnd; ++it)
    {
        Property *property = it.key();
        if (property->isSerializable() && !property->isEcoreContainer() && !property->isEcoreContainment())
            property->serializeAsXmiAttribute(xmiWriter, this);
    }

    // Now the Children (Containment)
    for (auto it = itStart ; it != itEnd; ++it)
    {
        Property *property = it.key();
        if ( property->isSerializable() && property->isEcoreContainment() )
        {
            LinkProperty *linkProperty = static_cast<LinkProperty*>(property);
            bool specifyXmiType  = linkProperty->getLinkedElementType()->isDerived();
            QString childTagName = linkProperty->getName();
            for (Element *childElem : linkProperty->getLinkedElements(this))
            {
                if (specifyXmiType)
                    childElem->serialize(xmiWriter, childTagName, childElem->getElementTypeName());
                else
                    childElem->serialize(xmiWriter, childTagName);
            }
        }
    }

    // End
    xmiWriter->writeEndElement();
}

void Element::exportWithLinksAsNewModelSharingSameElements(Model *refModel, Model *subModel)
{
    if (!subModel->contains(this))
    {
        // add this to the subModel
        subModel->add(getElementType(), this);

        // add recursively all its linked elements
        for (Property *property : _propertyValueMap.keys())
        {
            if (property->isALinkProperty())
            {
                for (Element *linkedElement : static_cast<LinkProperty*>(property)->getLinkedElements(this) )
                    linkedElement->exportWithLinksAsNewModelSharingSameElements(refModel, subModel);
            }
        }
    }
}
