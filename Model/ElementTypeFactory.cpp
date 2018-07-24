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

#include "ElementTypeFactory.h"

void ElementTypeFactory::initElementTypes()
{
    defineDerivedElementTypesFromEcore();
    _makeAllElementTypeDeriveFromElement();
}

ElementType* ElementTypeFactory::getElementTypeById(int id)
{
    for (auto it = _elementTypes.cbegin(); it != _elementTypes.cend(); ++it)
    {
        ElementType *elementType = it.value();
        if (elementType->getId() == id)
            return elementType;
    }
    return nullptr;
}

ElementType *ElementTypeFactory::getElementTypeFromQVariant(const QVariant &variant)
{
    if (variant.canConvert<ElementType*>())
        return static_cast<ElementType*>(variant.value<ElementType*>());
    return nullptr;
}


ElementType* ElementTypeFactory::_createType(int id, const QString &name, const char *label, ElementCreator eltCreator, bool isInstanciable)
{
    ElementType* elementType = new ElementType(id, name, label, eltCreator, isInstanciable);
    _elementTypes.insert(name, elementType);
    return elementType;
}


void ElementTypeFactory::_makeAllElementTypeDeriveFromElement()
{
    for (ElementType *const elementType : _elementTypes){
        if(elementType != Element::TYPE && elementType->getSuperElementTypes().isEmpty())
            Element::TYPE->addDerivedElementType(elementType);
    }
}

