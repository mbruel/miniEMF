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
#include "MObjectType.h"
#include "MObject.h"
#include <QCoreApplication>


MObjectType::MObjectType(int id, const QString &name, const char *label, ModelObjectCreator eltCreator, bool isInstanciable):
    _id(id), _name(name), _label(label), _isInstanciable(isInstanciable),
    _derivedModelObjectTypes(), _superModelObjectTypes(),
    _containerProperty(nullptr),
    _elementCreator(eltCreator), _nbModelObjects(0)
{}

MObjectType::~MObjectType() {}

QString MObjectType::getLabel() const { return QCoreApplication::translate("MObjectType", _label);} //QObject::tr(_label); }

QSet<MObjectType *> MObjectType::getDerivedModelObjectTypes()
{
    QSet<MObjectType *> derivedModelObjectTypes(_derivedModelObjectTypes);
    for (MObjectType * mObjectType : _derivedModelObjectTypes)
    {
        if(mObjectType->isDerived())
            derivedModelObjectTypes.unite(mObjectType->getDerivedModelObjectTypes());
    }

    return derivedModelObjectTypes;
}

QSet<MObjectType *> MObjectType::getInstanciableModelObjectTypes()
{
    QSet<MObjectType *> instanciableModelObjectTypes;
    if (_isInstanciable)
        instanciableModelObjectTypes.insert(this);

    for (MObjectType * mObjectType : _derivedModelObjectTypes)
        instanciableModelObjectTypes.unite(mObjectType->getInstanciableModelObjectTypes());

    return instanciableModelObjectTypes;
}

QSet<MObjectType *> MObjectType::getSuperModelObjectTypes() { return _superModelObjectTypes; }

QSet<MObjectType *> MObjectType::getSuperInstanciableModelObjectTypes()
{
    QSet<MObjectType *> superInstanciableTypes;
    for (MObjectType *superType : _superModelObjectTypes)
    {
        if (superType->_isInstanciable)
            superInstanciableTypes.insert(superType);
    }
    return superInstanciableTypes;
}

void MObjectType::addDerivedModelObjectType(MObjectType *eltType)
{
    if (eltType)
    {
        _derivedModelObjectTypes.insert(eltType);
        eltType->addSuperModelObjectType(this);
    }
}

void MObjectType::addSuperModelObjectType(MObjectType *superModelObjectType) {
    _superModelObjectTypes.insert(superModelObjectType);
}

#include "Model/Property.h"
MObject *MObjectType::createModelObject(uint projectId, bool doDefaultInit, const QMap<Property *, QVariant> &properties)
{
    if (_elementCreator == &MObject::createModelObject)
        return nullptr;
    else
    {
        MObject *mObject = _elementCreator();
        mObject->setId(QString("%1_%2_%3").arg(getId()).arg(projectId).arg(++_nbModelObjects));

        Property *containerProp = nullptr;
        for (auto it = properties.cbegin(), itEnd = properties.cend() ; it != itEnd ; ++it)
        {
            Property *property = it.key();
            if (property->isEcoreContainer())
                containerProp = property;
            else
                property->updateValue(mObject, it.value());
        }

        // we should do the container property in last in case
        // the reverse property is a Map that would need getPropertyMapKey to have been set
        if (containerProp)
            containerProp->updateValue(mObject, properties.value(containerProp));

        if (doDefaultInit)
            mObject->initDefaultProperties();

        return mObject;
    }
}

void MObjectType::initModelObjectWithDefaultValues(MObject *mObject, uint modelId)
{
    mObject->setId(QString("%1_%2_%3").arg(getId()).arg(modelId).arg(_nbModelObjects));
    mObject->setName(QString("%1 %2").arg(getLabel()).arg(_nbModelObjects));
}

void MObjectType::updateMaxId(int elemId)
{
    if (elemId > (int)_nbModelObjects)
        _nbModelObjects = elemId;
}
