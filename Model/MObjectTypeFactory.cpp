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

#include "MObjectTypeFactory.h"

MObjectTypeFactory::MObjectTypeFactory() : _mObjectTypes(){}

void MObjectTypeFactory::initModelObjectTypes()
{
    defineDerivedModelObjectTypesFromEcore();
    _makeAllModelObjectTypeDeriveFromModelObject();
}

MObjectType* MObjectTypeFactory::getModelObjectTypeById(int id)
{
    for (auto it = _mObjectTypes.cbegin(), itEnd = _mObjectTypes.cend(); it != itEnd; ++it)
    {
        MObjectType *mObjectType = it.value();
        if (mObjectType->getId() == id)
            return mObjectType;
    }
    return nullptr;
}


//MObjectType *MObjectTypeFactory::getModelObjectTypeFromQVariant(const QVariant &variant)
//{
//    if (variant.canConvert<void*>())
//        return static_cast<MObjectType*>(variant.value<void*>());
//    return nullptr;
//}


MObjectType* MObjectTypeFactory::_createType(int id, const QString &name, const char *label, ModelObjectCreator eltCreator, bool isInstanciable)
{
    MObjectType* mObjectType = new MObjectType(id, name, label, eltCreator, isInstanciable);
    _mObjectTypes.insert(name, mObjectType);
    return mObjectType;
}


void MObjectTypeFactory::_makeAllModelObjectTypeDeriveFromModelObject()
{
    for (MObjectType *const mObjectType : _mObjectTypes){
        if(mObjectType != MObject::TYPE && mObjectType->getSuperModelObjectTypes().isEmpty())
            MObject::TYPE->addDerivedModelObjectType(mObjectType);
    }
}

