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
#include "ElementType.h"
#include "Element.h"
#include <QCoreApplication>


ElementType::ElementType(int id, const QString &name, const char *label, ElementCreator eltCreator, bool isInstanciable):
    _id(id), _name(name), _label(label), _isInstanciable(isInstanciable),
    _derivedElementTypes(), _superElementTypes(),
    _containerProperty(nullptr),
    _elementCreator(eltCreator), _nbElements(0)
{}

ElementType::~ElementType() {}

QString ElementType::getLabel() const { return QCoreApplication::translate("ElementType", _label);} //QObject::tr(_label); }

QSet<ElementType *> ElementType::getDerivedElementTypes()
{
    QSet<ElementType *> derivedElementTypes(_derivedElementTypes);
    for (ElementType * elementType : _derivedElementTypes)
    {
        if(elementType->isDerived())
            derivedElementTypes.unite(elementType->getDerivedElementTypes());
    }

    return derivedElementTypes;
}

QSet<ElementType *> ElementType::getInstanciableElementTypes()
{
    QSet<ElementType *> instanciableElementTypes;
    if (_isInstanciable)
        instanciableElementTypes.insert(this);

    for (ElementType * elementType : _derivedElementTypes)
        instanciableElementTypes.unite(elementType->getInstanciableElementTypes());

    return instanciableElementTypes;
}

QSet<ElementType *> ElementType::getSuperElementTypes() { return _superElementTypes; }

void ElementType::addDerivedElementType(ElementType *eltType)
{
    if (eltType)
    {
        _derivedElementTypes.insert(eltType);
        eltType->addSuperElementType(this);
    }
}

void ElementType::addSuperElementType(ElementType *superElementType) {
    _superElementTypes.insert(superElementType);
}

Element *ElementType::createElement()
{
    if (_elementCreator == &Element::createElement)
        return nullptr;
    else
    {
        ++_nbElements;
        return _elementCreator();
    }
}

// MB_TODO: we shouldn't use the modelId in hard...
// Use a class ID instead
void ElementType::initElementWithDefaultValues(Element *element)
{
    QString modelId("42");
    element->setId(QString("%1_%2_%3").arg(getId()).arg(modelId).arg(_nbElements));
    element->setName(getLabel()+"_"+element->getId());
}
