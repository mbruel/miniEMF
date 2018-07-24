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
#include "Model/Element.h"
#include <QString>
#include <QMap>
#include <QVariant>

class ElementType;

class ElementTypeFactory
{
protected:
    QMap<QString, ElementType*> _elementTypes;

public:
    virtual void initStatics() = 0;
    virtual QList<ElementType*> getRootElementTypes() = 0;

    virtual ~ElementTypeFactory() = default;

    void initElementTypes();

    inline ElementType *getElementTypeByName(const QString &name);
    ElementType *getElementTypeById(int id);
    ElementType *getElementTypeFromQVariant(const QVariant &variant);

protected:
    ElementTypeFactory() : _elementTypes(){}
    ElementType *_createType(int id, const QString &name, const char *label, ElementCreator eltCreator = &Element::createElement, bool isInstanciable = true);

    virtual void defineDerivedElementTypesFromEcore() = 0;

private:
    void _makeAllElementTypeDeriveFromElement();

};


ElementType *ElementTypeFactory::getElementTypeByName(const QString &name)
{
    return _elementTypes.value(name, nullptr);
}

#endif // ELEMENTTYPEFACTORY_H
