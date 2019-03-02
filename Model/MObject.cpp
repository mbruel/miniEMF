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

#include "MObject.h"
#include "Property.h"
#include "Model.h"
#include "Utils/XmiWriter.h"


MObjectType    *MObject::TYPE        = nullptr;

StringProperty *MObject::PROPERTY_NAME        = nullptr;
//StringProperty *MObject::PROPERTY_DESCRIPTION = nullptr;



QMap<QString, Property *> *MObject::initPropertyMap()
{
    QMap<QString, Property*>* propertyMap = new QMap<QString, Property*>();
    addPropertyToMap(propertyMap, MObject::PROPERTY_NAME);
//    addPropertyToMap(propertyMap, MObject::PROPERTY_DESCRIPTION);
    return propertyMap;
}

void MObject::addPropertyToMap(QMap<QString, Property *> *propertyMap, Property *property)
{
    (*propertyMap)[property->getName()] = property;
}


void MObject::_addBusinessError(QStringList &businessErrors, MObject *visibleObject, const QString &errorMsg)
{
    businessErrors << QString("<%1: %2>").arg(visibleObject->getName()).arg(errorMsg);
}



//QString MObject::getDescription() { return PROPERTY_DESCRIPTION->getValue(this); }
//void MObject::setDescription(const QString &value) { PROPERTY_DESCRIPTION->setValue(this, value);}

QString MObject::getName() const { return PROPERTY_NAME->getValue(this);}
void MObject::setName(const QString &name){ PROPERTY_NAME->setValue(this, name);}

QString MObject::getDefaultName()
{
    QString defaultName(getModelObjectTypeName());
    defaultName += "_";
    defaultName += getId();

    return defaultName;
}

void MObject::validateLinkProperties(QStringList &ecoreErrors)
{
    for (auto it = _propertyValueMap.cbegin(), itEnd = _propertyValueMap.cend(); it != itEnd; ++it)
    {
        Property *property = it.key();
        if (property->isALinkProperty())
            static_cast<LinkProperty*>(property)->validateModelObject(this, ecoreErrors);
    }
}

bool MObject::validateBusinessRules(QStringList &businessErrors, MObject *owner)
{
    Q_UNUSED(businessErrors);
    Q_UNUSED(owner);
    return true;
}

//Property *MObject::getProperty(QString propertyName)
//{
//    return (*propertyMap)[propertyName];
//}



void MObject::_initPropertyValueMap(QMap<QString, Property*>* classPropertyMap)
{
    for (auto it = classPropertyMap->cbegin(), itEnd = classPropertyMap->cend(); it != itEnd; ++it) {
        Property *property = it.value();
        _propertyValueMap[property] = property->createNewInitValue();
    }
}




void MObject::setPropertyValueFromQVariant(Property *property, const QVariant &value)
{
    _propertyValueMap.insert(property, value);
}

void MObject::setPropertyValueFromElement(LinkProperty* property, MObject* value)
{
    setPropertyValueFromQVariant(property, QVariant::fromValue(static_cast<void*>(value)));
}


QVariant MObject::getPropertyMapKey(Property *mapProperty)
{
    Q_UNUSED(mapProperty);
    return getName();
}


QSet<LinkProperty*> MObject::getLinkProperties()
{
    QSet<LinkProperty*> linkProperties;
    for (Property * property : _propertyValueMap.keys()){
        if (property->isALinkProperty())
            linkProperties.insert(static_cast<LinkProperty*>(property));
    }
    return linkProperties;
}

QMap<QString, LinkProperty *> MObject::getContainmentProperties() const
{
    QMap<QString, LinkProperty *> properties;
    for (auto it = _propertyValueMap.cbegin(), itEnd = _propertyValueMap.cend() ; it != itEnd ; ++it)
    {
        Property *property = it.key();
        if (property->isEcoreContainment())
            properties.insert(property->getName(), static_cast<LinkProperty*>(property));
    }
    return properties;
}

QMap<QString, Property *> MObject::getNonContainmentProperties() const
{
    QMap<QString, Property *> properties;
    for (auto it = _propertyValueMap.cbegin(), itEnd = _propertyValueMap.cend() ; it != itEnd ; ++it)
    {
        Property *property = it.key();
        if (!property->isEcoreContainment() && property->isSerializable())
            properties.insert(property->getName(), property);
    }
    return properties;
}

void MObject::hideFromLinkedModelObjects()
{
    for (LinkProperty *linkProperty : getLinkProperties())
    {
        if (!linkProperty->isEcoreContainment())
        {
            LinkProperty *reverseProperty = linkProperty->getReverseLinkProperty();
            if (reverseProperty)
            {
                for (MObject *linkedElem : linkProperty->getLinkedModelObjects(this))
                    reverseProperty->removeLink(linkedElem, this);
            }
        }
    }
}

void MObject::makeVisibleForLinkedModelObjects()
{
    for (LinkProperty *linkProperty : getLinkProperties())
    {
        if (!linkProperty->isEcoreContainment())
        {
            LinkProperty *reverseProperty = linkProperty->getReverseLinkProperty();
            if (reverseProperty)
            {
                for (MObject *linkedElem : linkProperty->getLinkedModelObjects(this))
                    reverseProperty->addLink(linkedElem, this);
            }
        }
    }
}




// ##### END MANAGEMENT OF PROPERTIES ####
// #######################################


MObject::MObject(QMap<QString, Property *> *classPropertyMap):
    _id(), _state(STATE::CREATED),
    _isReadOnly(false), _isNameReadOnly(false),
    _propertyValueMap()
{
    _initPropertyValueMap(classPropertyMap);
}

MObject::~MObject()
{
#ifdef __CASCADE_DELETION__
    for (auto it = _propertyValueMap.begin(), itEnd = _propertyValueMap.end(); it != itEnd; ++it)
    {
        Property *property = it.key();
        if (property->isALinkProperty() && property->isEcoreContainment())
        {
            LinkProperty *linkProperty = static_cast<LinkProperty*>(property);
            for (MObject *containedObject : linkProperty->getLinkedModelObjects(this))
                delete containedObject;
        }
    }
#endif

// Hide from LinkedModelObject should be done in the Children destructor
// as they may reimplement getPropertyMapKey
    //    hideFromLinkedModelObjects();
}

MObject *MObject::clone(MObject *ecoreContainer, uint modelId, bool sameId)
{
    MObjectType *type = getModelObjectType();
    MObject *newModelObject = type->createModelObject(modelId);

    // Set up the id
    if (sameId)
        newModelObject->setId(_id);
    else
        type->initModelObjectWithDefaultValues(newModelObject, modelId);
qDebug() << "[MB_TRACE][MObject::clone] >>>>>>>>>> " << getName();
    // Copy the properties (deep copy)
    for (auto it = _propertyValueMap.cbegin(), itEnd = _propertyValueMap.cend(); it != itEnd ; ++it)
    {
        Property *property = it.key();
qDebug() << "[MB_TRACE][MObject::clone] - property: " << property->getName();
        if (property->isAttributeProperty())
            newModelObject->setPropertyValueFromQVariant(property, it.value());
        else
        {
            LinkProperty *linkProperty = static_cast<LinkProperty*>(property);
            if (linkProperty->isALinkToOneProperty())
            {
                if (linkProperty->isEcoreContainment())
                {
                    MObject *linkedModelObject = static_cast<MObject*>(it.value().value<void*>());
                    if (linkedModelObject)
                    {
                        linkedModelObject = linkedModelObject->clone(nullptr, modelId, sameId);
                        linkProperty->updateValue(newModelObject, linkedModelObject->toVariant());
                    }
                }
                else if (linkProperty->isEcoreContainer() && ecoreContainer)
                {
                    if (ecoreContainer->getModelObjectType() == linkProperty->getLinkedModelObjectType())
                        static_cast<LinkToOneProperty*>(linkProperty)->updateValue(newModelObject, ecoreContainer->toVariant());
                }
                else
                    newModelObject->setPropertyValueFromQVariant(property, it.value());
            }
            else
            {
                if (linkProperty->isEcoreContainment())
                {
                    MObjectList clonedLinkedModelObjects;
                    for (MObject *linkedElem : linkProperty->getLinkedModelObjects(this))
                        clonedLinkedModelObjects.append(linkedElem->clone(nullptr, modelId, sameId));

                    linkProperty->updateValue(newModelObject, clonedLinkedModelObjects);
                }
                else
                    linkProperty->setValues(newModelObject, linkProperty->getLinkedModelObjects(this));
            }
        }
    }
qDebug() << "[MB_TRACE][MObject::clone] <<<<<<<<<< " << getName();
    return newModelObject;
}

MObject *MObject::shallowCopy()
{
    // create clone using the appropriate Type constructor
    MObject *newModelObject = getModelObjectType()->createModelObject(0);
    newModelObject->_id     = _id;
    newModelObject->_state  = STATE::CLONE;

    // we don't copy the properties as QVariant is a QSharedData
    return newModelObject;
}

void MObject::copyPropertiesFromSourceElementWithCloneElements(MObject *srcElem, Model *clonedModel)
{
    for (auto itProp = srcElem->_propertyValueMap.cbegin(), itPropEnd = srcElem->_propertyValueMap.cend(); itProp != itPropEnd ; ++itProp)
    {
        Property *property = itProp.key();
        QVariant valueCopy = itProp.value();

        if (property->isAttributeProperty())
            _propertyValueMap[property] = valueCopy;
        else
        {
            LinkProperty *linkProperty      = static_cast<LinkProperty*>(property);
            MObjectType  *linkedModelObjectType = linkProperty->getLinkedModelObjectType();
            if (linkProperty->isMapProperty())
            {
                MObjectMap clonedLinkedElemMap, *srcLinkedElemMap = linkProperty->getLinkedModelObjectsMap(srcElem);
                for (const QVariant & key : srcLinkedElemMap->uniqueKeys())
                {
                    QVariant keyCopy     = key;
                    MObjectList linkedElems = srcLinkedElemMap->values(key);
                    auto it = linkedElems.cend(), itStart = linkedElems.cbegin();
                    do
                    { // we do it backwards cause insertMulti insert duplicate in front and values send back in the same order.
                        --it;
                        MObject *srcLinkedElem = *it,
                                *clonedLinkedElem = clonedModel->getModelObjectById(linkedModelObjectType, srcLinkedElem->getId());
                        if (clonedLinkedElem) // It may have been filtered if we use a subModel ;)
                            clonedLinkedElemMap.insertMulti(keyCopy, clonedLinkedElem);
                    } while (it != itStart);
                }
//                for (auto it = srcLinkedElemMap->cbegin() ; it != srcLinkedElemMap->cend() ; ++it)
//                {
//                    QVariant keyCopy       = it.key();
//                    MObject *srcLinkedElem = it.value();
//                    clonedLinkedElemMap.insertMulti(keyCopy, clonedModel->getModelObjectById(linkedModelObjectType, srcLinkedElem->getId()));
//                }
                linkProperty->setValuesFromMap(this, &clonedLinkedElemMap);
            }
            else
            {
                MObjectList clonedLinkedModelObjects;
                for (MObject *srcLinkedElem : linkProperty->getLinkedModelObjects(srcElem))
                {
                    MObject *clonedLinkedElem = clonedModel->getModelObjectById(linkedModelObjectType, srcLinkedElem->getId());
                    if (clonedLinkedElem) // It may have been filtered if we use a subModel ;)
                        clonedLinkedModelObjects.append(clonedLinkedElem);
                }
                linkProperty->setValues(this, clonedLinkedModelObjects);
            }
        }
    }
}

MObjectType* MObject::getLinkedModelObjectType(LinkProperty *linkProperty) const
{
    return linkProperty->getLinkedModelObjectType();
}

void MObject::serialize(XmiWriter *xmiWriter, const QString &tagName, const QString &xmiType)
{
    if (_state == STATE::REMOVED_FROM_MODEL || _isReadOnly)
        return;

    // Start
    xmiWriter->writeStartElement(tagName);
    if (!xmiType.isEmpty())
        xmiWriter->addAttribute("MObjectType", "Cosi7:" + xmiType);
    xmiWriter->addAttribute("id", this->getId());

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
            bool specifyXmiType  = linkProperty->getLinkedModelObjectType()->isDerived();
            QString childTagName = linkProperty->getName();
            for (MObject *childElem : linkProperty->getLinkedModelObjects(this))
            {
                if (!childElem)
                {
                    MObjectList list = linkProperty->getLinkedModelObjects(this);
                    qDebug() << "[MB_TRACE][] ERROR: NULL childElem for childTagName" << childTagName
                             << " elem: " << getName()  << ", nb in list: " << list.size();

                }
                else
                {
                    if (specifyXmiType)
                        childElem->serialize(xmiWriter, childTagName, childElem->getModelObjectTypeName());
                    else
                        childElem->serialize(xmiWriter, childTagName);
                }
            }
        }
    }

    // End
    xmiWriter->writeEndElement();
}

void MObject::exportWithLinksAsNewModelSharingSameModelObjects(Model *subModel, const QSet<MObjectType *> &rootTypesToNotTake)
{
    if (rootTypesToNotTake.contains(getModelObjectType()))
        return;

    if (isInModel() && !subModel->contains(this))
    {
        // add this to the subModel
        subModel->add(getModelObjectType(), this);

        // add recursively all its linked mObjects
        for (Property *property : _propertyValueMap.keys())
        {
            // We only consider "real" link properties
            // (we don't take the handy e-opposites aka not serializable properties)
            if (property->isALinkProperty() && property->isSerializable())
            {
                for (MObject *linkedModelObject : static_cast<LinkProperty*>(property)->getLinkedModelObjects(this) )
                {
//                    if (property->isEcoreContainer())
                        linkedModelObject->exportWithLinksAsNewModelSharingSameModelObjects(subModel, rootTypesToNotTake);
                }
            }
        }
    }
}


#ifdef __USE_HMI__
QIcon MObject::getIcon() {return QIcon();}
#endif
