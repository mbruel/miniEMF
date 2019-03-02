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

class MObject;
class LinkProperty;
class LinkToOneProperty;
class Property;

using ModelObjectCreator = MObject* (*)(void);
//typedef MObject* (*ModelObjectCreator)(void);

class MObjectType
{
public:
    friend class Model; // to reset _nbModelObjects

    explicit MObjectType(int id, const QString &name, const char *label, ModelObjectCreator eltCreator, bool isInstanciable = true);
    ~MObjectType();

    inline int     getId() const;
    inline QString getName() const;
    QString getLabel() const;
    inline bool    isInstanciable() const;
    inline bool    isDerived() const;

    QSet<MObjectType*> getDerivedModelObjectTypes();
    QSet<MObjectType*> getInstanciableModelObjectTypes();
    QSet<MObjectType*> getSuperModelObjectTypes();
    QSet<MObjectType*> getSuperInstanciableModelObjectTypes();
    void addDerivedModelObjectType(MObjectType* eltType);
    void addSuperModelObjectType(MObjectType* superModelObjectType);

    inline bool isA(MObjectType *type) const;

    inline LinkProperty *getContainerProperty() const;
    inline void setContainerProperty(LinkProperty *containerProperty);

    MObject *createModelObject(uint projectId, const QMap<Property *, QVariant> &properties = QMap<Property *, QVariant>());

    void initModelObjectWithDefaultValues(MObject *mObject, uint modelId);

    void updateMaxId(int elemId);

private:
    const int     _id;
    const QString _name;
    const char    *_label;
    const bool    _isInstanciable;

    QSet<MObjectType*>   _derivedModelObjectTypes; // ModelObjectTypes derived from this MObjectType (inherited classes in Ecore)
    QSet<MObjectType*>   _superModelObjectTypes; // ModelObjectTypes from which this MObjectType inherits

    LinkProperty *_containerProperty;

    const ModelObjectCreator _elementCreator;

    uint _nbModelObjects;
};


int     MObjectType::getId()          const { return _id; }
QString MObjectType::getName()        const { return _name; }
bool    MObjectType::isInstanciable() const { return _isInstanciable;}
bool    MObjectType::isDerived()      const { return !_derivedModelObjectTypes.isEmpty(); }

bool MObjectType::isA(MObjectType *type) const
{
    if (!type)
        return false;
    else if (type == this || _superModelObjectTypes.contains(type))
        return true;
    else
    {
        for (MObjectType *parentType : _superModelObjectTypes)
        {
            if (parentType->isA(type))
                return true;
        }
        return false;
    }
}

LinkProperty *MObjectType::getContainerProperty() const { return _containerProperty; }
void MObjectType::setContainerProperty(LinkProperty *containerProperty) { _containerProperty = containerProperty; }

#endif // ELEMENTTYPE_H

