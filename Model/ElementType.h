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

#ifndef ELEMENTTYPE_H
#define ELEMENTTYPE_H

#include <QString>
#include <QMap>
#include <QSet>
#include <QVariant>

class Element;
class LinkProperty;
class LinkToOneProperty;

using ElementCreator = Element* (*)(void);
//typedef Element* (*ElementCreator)(void);

class ElementType
{
public:
    explicit ElementType(int id, const QString &name, const char *label, ElementCreator eltCreator, bool isInstanciable = true);
    ~ElementType();

    inline int     getId() const;
    inline QString getName() const;
    QString getLabel() const;
    inline bool    isInstanciable() const;
    inline bool    isDerived() const;

    QSet<ElementType*> getDerivedElementTypes();
    QSet<ElementType*> getInstanciableElementTypes();
    QSet<ElementType*> getSuperElementTypes();
    void addDerivedElementType(ElementType* eltType);
    void addSuperElementType(ElementType* superElementType);

    inline bool isA(ElementType *type);

    inline LinkProperty *getContainerProperty() const;
    inline void setContainerProperty(LinkProperty *containerProperty);

    Element *createElement();

    void initElementWithDefaultValues(Element *element);


private:
    const int     _id;
    const QString _name;
    const char    *_label;
    const bool    _isInstanciable;

    QSet<ElementType*>   _derivedElementTypes; // ElementTypes derived from this ElementType (inherited classes in Ecore)
    QSet<ElementType*>   _superElementTypes; // ElementTypes from which this ElementType inherits

    LinkProperty *_containerProperty;

    const ElementCreator _elementCreator;

    uint _nbElements;
};


int     ElementType::getId()          const { return _id; }
QString ElementType::getName()        const { return _name; }
bool    ElementType::isInstanciable() const { return _isInstanciable;}
bool    ElementType::isDerived()      const { return !_derivedElementTypes.isEmpty(); }

bool ElementType::isA(ElementType *type)
{
    if (!type)
        return false;
    else if (type == this || _superElementTypes.contains(type))
        return true;
    else
        return false;
}

LinkProperty *ElementType::getContainerProperty() const { return _containerProperty; }
void ElementType::setContainerProperty(LinkProperty *containerProperty) { _containerProperty = containerProperty; }

#endif // ELEMENTTYPE_H

