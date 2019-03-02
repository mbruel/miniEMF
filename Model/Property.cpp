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
#include "MObject.h"

const int    Property::INT_INFINITE_POS = std::numeric_limits<int>::max();
const int    Property::INT_INFINITE_NEG = std::numeric_limits<int>::min();
const float  Property::FLT_INFINITE_POS = std::numeric_limits<float>::max();
const float  Property::FLT_INFINITE_NEG = -std::numeric_limits<float>::max();
const double Property::DBL_INFINITE_POS = std::numeric_limits<double>::max();
const double Property::DBL_INFINITE_NEG = -std::numeric_limits<double>::max();


#include <QCoreApplication>
Property::Property(const QString &name, const char *label, bool isSerializable):
    _name(name), _label(label), _unit(""), _serializable(isSerializable)
{}

QString Property::getLabel() const { return QCoreApplication::translate("Property", _label);} //QObject::tr(_label); }

void Property::updateValue(MObject *const mObject, QVariant value)
{
    mObject->setPropertyValueFromQVariant(this, value);
}

QVariant Property::convertIntoUpdatableValue(QVariant value) { return value; }

MObjectList Property::getMapValuesInInsertionOrder(const MObjectMap &map)
{
    MObjectList values;
    for (const QVariant & key : map.uniqueKeys())
    {
        MObjectList mObjects = map.values(key);
        auto it = mObjects.cend(), itStart = mObjects.cbegin();
        do
        { // we do it backwards cause insertMulti insert duplicate in front and values send back in the same order.
            --it;
            values.append(*it);
        } while (it != itStart);
    }
    return values;
}




uIntProperty::uIntProperty(const QString &name, const char *label, const int &defaultValue) : IntProperty(name, label, defaultValue){}
uIntProperty::~uIntProperty(){}

uDoubleProperty::uDoubleProperty(const QString &name, const char *label, const double &defaultValue) : DoubleProperty(name, label, defaultValue){}
uDoubleProperty::~uDoubleProperty(){}

PerCentProperty::PerCentProperty(const QString &name, const char *label, const double &defaultValue) : DoubleProperty(name, label, defaultValue){}
PerCentProperty::~PerCentProperty(){}

// #######################
// #### ENUM PROPERTY ####

EnumProperty::EnumProperty(const QString &name, const char *label, int defaultValue) :
    AttributeProperty<int>(name, label, defaultValue), _enumValues(){}


QString EnumProperty::getValueAsString(MObject *mObject)
{
    return QCoreApplication::translate("Constant", _enumValues.value(getValue(mObject)).toStdString().c_str());
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

void EnumProperty::serializeAsXmiAttribute(XmiWriter *xmiWriter, MObject *mObject)
{
    xmiWriter->addAttribute(_name, getEnumValueByKey(getValue(mObject)));
}

void EnumProperty::deserializeFromXmiAttribute(MObject *mObject, const QString &xmiValue)
{
    setValue(mObject, getEnumKeyByValue(xmiValue));
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
QWidget *EnumProperty::getEditor(MObject * const mObj, Model *model, QWidget *parent)
{
    Q_UNUSED(model)
    QComboBox *combobox = new QComboBox(parent);
    ushort index = 0;
    int currentValue = 0, currentValueIndex = 0;
    if (mObj)
        currentValue = getValue(mObj);
    for (auto it = _enumValues.cbegin() , itEnd = _enumValues.cend(); it != itEnd ; ++it)
    {
        combobox->addItem(it.value(), it.key());
        if (it.key() == currentValue)
            currentValueIndex = index;
        ++index;
    }
    combobox->setCurrentIndex(currentValueIndex);
    if (mObj && mObj->isReadOnly())
        combobox->setEnabled(false);

    return combobox;
}

int EnumProperty::getEditorValue(QWidget *editor)
{
    return static_cast<QComboBox*>(editor)->currentData().toInt();
}

void EnumProperty::setEditorValue(QWidget *editor, int value)
{
    QComboBox *combobox = static_cast<QComboBox*>(editor);
    int index = combobox->findData(value);
    if (index != -1)
        combobox->setCurrentIndex(index);
}

QVariant EnumProperty::getEditorUpdatedVariant(MObject * const mObj, QWidget *editor)
{
    QComboBox *combobox = static_cast<QComboBox*>(editor);
    int newVal = combobox->currentData().toInt(), oldVal = getValue(mObj);
    if (newVal != oldVal)
        return combobox->currentData();
    else
        return QVariant();
}
#endif



// #######################
// #### LINK PROPERTY ####

LinkProperty::LinkProperty(MObjectType *const eltType,
                           MObjectType *const linkedEltType,
                           const QString &name,
                           const char *label, bool isMandatory,
                           bool isSerializable) :
    Property(name, label, isSerializable),
    _mObjectType(eltType),
    _linkedModelObjectType(linkedEltType),
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

void LinkProperty::serializeAsXmiAttribute(XmiWriter *xmiWriter, MObject *mObject)
{
    xmiWriter->addAttribute(_name, getLinkedModelObjects(mObject, true));
}


void LinkProperty::setReverseLinkProperty(LinkProperty *reverseLinkProperty)
{
    _reverseLinkProperty = reverseLinkProperty;
//    this->mObjectType = reverseLinkProperty->getLinkedModelObjectType();
}


void LinkProperty::deserializeFromXmiAttribute(MObject *mObject, const QString &xmiValue)
{
    Q_UNUSED(mObject);
    Q_UNUSED(xmiValue);
    // No action : the deserialization is managed directly by ElementDao::deserializeModelObject(QDomNode node)
}

void LinkProperty::validateModelObject(MObject *mObject, QStringList &ecoreErrors)
{
    if (_isMandatory && getLinkedModelObjects(mObject).isEmpty())
        ecoreErrors << QString("Error on '%1': the property '%2' should not be null...").arg(
                           mObject->getName()).arg(getLabel());
}


// ############################
// #### LinkToOne PROPERTY ####

QVariant LinkToOneProperty::createNewInitValue()
{
    if (_defaultLinkedObject)
        return _defaultLinkedObject->toVariant();
    else
        return QVariant::fromValue(static_cast<void*>(_defaultLinkedObject));
}


void LinkToOneProperty::setValues(MObject *mObject, const MObjectList &values)
{
    MObject *value = nullptr;
    if (!values.isEmpty())
        value = *(values.begin());
    setValue(mObject, value);
}

void LinkToOneProperty::setValueFromXMIStringIdList(MObject *mObject, const QString &ids, Model *model)
{
    QSet<MObjectType *> allPossibleTypes = _linkedModelObjectType->getSuperInstanciableModelObjectTypes();
    allPossibleTypes.insert(_linkedModelObjectType);
    QString objectId = ids.trimmed();
    for (MObjectType *linkedType : allPossibleTypes)
    {
        MObject *linkedElt = model->getModelObjectById(linkedType, objectId);
        if (linkedElt)
        {
            updateValue(mObject, linkedElt->toVariant());
            break;
        }
    }
}

MObject* LinkToOneProperty::getValue(MObject* mObject)
{
    if(mObject)
        return mObject->getLinkPropertyValue<MObject>(this);

    return nullptr;
}

void LinkToOneProperty::setValue(MObject* mObject, MObject* value)
{
    mObject->setPropertyValueFromElement(this, value);
}

void LinkToOneProperty::updateValue(MObject* mObject, QVariant value)
{
    if (mObject && value.canConvert<void*>())
    {
        MObject* oldLink = getValue(mObject);

        MObject *newLink = static_cast<MObject*>(value.value<void*>());
        setValue(mObject, newLink);

        // update reverse property
        if (_reverseLinkProperty) // Example of linkProperty for which reverseLinkProperty is null : ConstraintSet -> MObject)
        {
            if (oldLink)
                _reverseLinkProperty->removeLink(oldLink, mObject);
            if (newLink)
                _reverseLinkProperty->addLink(newLink, mObject);
        }
    }
}

void LinkToOneProperty::updateValue(MObject * const mObject, const MObjectList &values)
{
    MObject *value = nullptr;
    if (values.size())
        value = *(values.begin());
    updateValue(mObject, value->toVariant());
}

void LinkToOneProperty::addLink(MObject* mObject, MObject* mObjectToAdd)
{
    setValue(mObject, mObjectToAdd);
}

void LinkToOneProperty::removeLink(MObject* mObject, MObject* mObjectToRemove)
{
    Q_UNUSED(mObjectToRemove);
    setValue(mObject, nullptr);
}


QList<MObject*> LinkToOneProperty::getLinkedModelObjects(MObject* mObject, bool ordered)
{
    Q_UNUSED(ordered)
    QList<MObject*> linkedModelObjects;
    MObject *linkedModelObject = getValue(mObject);
    if (linkedModelObject)
        linkedModelObjects.append(linkedModelObject);

    return linkedModelObjects;
}

/*
MObject *LinkToOneProperty::getValue(MObject * const mObject)
{
    MObjectSet *values = getValues(mObject);
    if (values->isEmpty())
        return nullptr;
    else
        return *(values->begin());
}

void LinkToOneProperty::setValue(MObject * const mObject, MObject * const value)
{
    MObjectSet values;
    if (value)
        values.insert(value);
    setValues(mObject, &values);
}

void LinkToOneProperty::addLink(MObject* mObject, MObject* mObjectToAdd)
{
    setValue(mObject, mObjectToAdd);
}

void LinkToOneProperty::removeLink(MObject* mObject, MObject* mObjectToRemove)
{
    Q_UNUSED(mObjectToRemove);
    setValue(mObject, nullptr);
}

void LinkToOneProperty::updateValue(MObject* mObject, QVariant value)
{
    if (mObject && value.canConvert<void*>())
    {
        MObject *newLink = static_cast<MObject*>(value.value<void*>());

        // update reverse property
        MObject* oldLink = getValue(mObject);
        if (_reverseLinkProperty) // Example of linkProperty for which reverseLinkProperty is null : ConstraintSet -> MObject)
        {
            if (oldLink)
                _reverseLinkProperty->removeLink(oldLink, mObject);
            if (newLink)
                _reverseLinkProperty->addLink(newLink, mObject);
        }

        setValue(mObject, newLink);
    }
}

void LinkToOneProperty::setValueFromXMIStringIdList(MObject *mObject, const QString &ids, Model *model)
{
    MObject *linkedElt = model->getModelObjectById(_linkedModelObjectType, ids.trimmed());
    if (linkedElt)
        updateValue(mObject, linkedElt->toVariant());
}
*/

#ifdef __USE_HMI__
#include <QVariant>
QWidget *LinkToOneProperty::getEditor(MObject * const mObj, Model *model, QWidget *parent)
{
    QComboBox *combobox = new QComboBox(parent);
    ushort index = 0;
    MObject *currentValue = getValue(mObj);
    if (model)
    {
        if (!isMandatory() || !currentValue)
        {
            MObject *dummyElem = nullptr;
            combobox->addItem("", QVariant::fromValue(static_cast<void*>(dummyElem)));
            ++index;
        }
        int currentValueIndex = 0;
        QList<MObject*> possibleLinkedObjects = model->getModelObjectsOrderedByNames(_linkedModelObjectType);
        if (_defaultLinkedObject && !possibleLinkedObjects.contains(_defaultLinkedObject))
            possibleLinkedObjects.prepend(_defaultLinkedObject);

        for (MObject *e : possibleLinkedObjects)
        {
            combobox->addItem(e->getIcon(), e->getName(), e->toVariant());

            if (e == currentValue)
                currentValueIndex = index;
            ++index;
        }
        combobox->setCurrentIndex(currentValueIndex);
    }
    if (mObj && mObj->isReadOnly())
        combobox->setEnabled(false);
    return combobox;
}

QVariant LinkToOneProperty::getEditorUpdatedVariant(MObject * const mObj, QWidget *editor)
{
    QComboBox *combobox = static_cast<QComboBox*>(editor);
    MObject *newVal = static_cast<MObject*>(combobox->currentData().value<void*>()), *oldVal = getValue(mObj);
    if (newVal != oldVal)
        return combobox->currentData();
    else
        return QVariant();
}

MObject *LinkToOneProperty::getEditorValue(QWidget *editor)
{
    QComboBox *combobox = static_cast<QComboBox*>(editor);
    return static_cast<MObject*>(combobox->currentData().value<void*>());
}

void LinkToOneProperty::setEditorValue(QWidget *editor, MObject *value)
{
    QComboBox *combobox = static_cast<QComboBox*>(editor);
    int index = combobox->findData(QVariant::fromValue(static_cast<void*>(value)));
    if (index != -1)
        combobox->setCurrentIndex(index);
}

QWidget *LinkToOneProperty::getEditorUsingAlsoDerivedLinkedObjects(MObject * const mObj, Model *model, QWidget *parent)
{
    QComboBox *combobox = new QComboBox(parent);
    ushort index = 0;
    MObject *currentValue = getValue(mObj);
    if (model)
    {
        if (!isMandatory() || !currentValue)
        {
            MObject *dummyElem = nullptr;
            combobox->addItem("", QVariant::fromValue(static_cast<void*>(dummyElem)));
            ++index;
        }
        int currentValueIndex = 0;
        QList<MObject*> possibleLinkedObjects = model->getModelObjectsOrderedByNames(_linkedModelObjectType, true);
        if (_defaultLinkedObject && !possibleLinkedObjects.contains(_defaultLinkedObject))
            possibleLinkedObjects.prepend(_defaultLinkedObject);

        for (MObject *e : possibleLinkedObjects)
        {
            combobox->addItem(e->getIcon(), e->getName(), e->toVariant());

            if (e == currentValue)
                currentValueIndex = index;
            ++index;
        }
        combobox->setCurrentIndex(currentValueIndex);
    }
    if (mObj && mObj->isReadOnly())
        combobox->setEnabled(false);
    return combobox;
}
#endif

