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

#ifndef ELEMENTTYPEFACTORY_H
#define ELEMENTTYPEFACTORY_H


#include "Utils/PureStaticClass.h"
#include "MObject.h"
#include <QString>
#include <QMap>
#include <QVariant>

class MObjectType;

class MObjectTypeFactory
{
protected:
    QMap<QString, MObjectType*> _mObjectTypes;

public:
    virtual void initStatics() = 0;
    virtual QList<MObjectType*> getRootModelObjectTypes() = 0;

    virtual ~MObjectTypeFactory() = default;

    void initModelObjectTypes();

    inline MObjectType *getModelObjectTypeByName(const QString &name);
    MObjectType *getModelObjectTypeById(int id);
//    MObjectType *getModelObjectTypeFromQVariant(const QVariant &variant);

protected:
    MObjectTypeFactory();
    MObjectType *_createType(int id, const QString &name, const char *label, ModelObjectCreator eltCreator = &MObject::createModelObject, bool isInstanciable = true);

    virtual void defineDerivedModelObjectTypesFromEcore() = 0;

private:
    void _makeAllModelObjectTypeDeriveFromModelObject();

};


MObjectType *MObjectTypeFactory::getModelObjectTypeByName(const QString &name)
{
    return _mObjectTypes.value(name, nullptr);
}

#endif // ELEMENTTYPEFACTORY_H
