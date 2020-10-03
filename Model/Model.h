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

#ifndef MODEL_H
#define MODEL_H

#include "aliases.h"

#include <QSet>


class MObject;
class MObjectType;
class MObjectTypeFactory;


class Model
{
    friend class XmiWriter; // to access _typeFactory
    friend class XMIService; // for exports


private:  
    MObjectTypeFactory *_typeFactory;
    QMap<MObjectType*, QMap<ElemId, MObject*>* > _mObjectTypeMap;
    QMap<MObjectType*, uint> _nextElemId;

    bool          _ownModelObjects; //!< set to false for subModels, no destuction of the ELements in destructor

    const QString _toolName;
    const QString _exportVersion;
    const QString _exportDescription;
          uint    _id;
    const QString _date;


public:
    Model(MObjectTypeFactory *typeFactory, const QString &dataModel,
          const QString &version, const QString &desc, uint id,
          const QString &date, bool ownElements = true);

    // Moveable from but not copyable (so we can use MObject::exportAsModel() )
    Model(Model &&other);
    Model(const Model &other)             = delete;
    Model & operator=(Model &&other)      = delete;
    Model & operator=(const Model &other) = delete;

    ~Model();
    void shallowCopySubsetOfMainModel(const MObjectSet &elementsToCopy,
                                      const QSet<MObjectType*> &rootTypesToNotTake = QSet<MObjectType*>(),
                                      bool onlyContainment = false);
    Model *cloneSubset(const MObjectSet &mainElements);
    QString getCopyName(MObject *mObjToCopy) const;

    static Model *clone(Model *model);

    void add(MObjectType *mObjectType, MObject *mObject, bool updateElemState = true);
    void add(MObject *mObject);
    void remove(MObject *mObject, bool hideFromOtherObjects = true);

    bool contains(MObject *mObject);

    void resetTypesNumberOfModelObjects();
    void clearModel(bool deleteModelObjects = true);

    void validate(QStringList &compilationErrors, const QSet<MObjectType*> &typesToExclude = QSet<MObjectType*>());
    void validateModel(QStringList &compilationErrors);
    void validateBusinessRules(QStringList &compilationErrors, const QSet<MObjectType*> &typesToExclude = QSet<MObjectType*>());

    MObjectSet getModelObjects(MObjectType *mObjectType, bool useDerivedType = false, MObjectSet *filterModelObjects = nullptr);
    QMap<ElemId, MObject *> getModelObjectsAsMap(MObjectType *mObjectType, bool useDerivedType = false, MObjectSet *filterModelObjects = nullptr);
    MObject        *getModelObjectById(MObjectType *mObjectType, const QString &id);
    MObject        *getModelObjectByName(MObjectType *mObjectType, const QString &name);

    MObjectList getModelObjectsOrderedByNames(MObjectType *mObjectType, bool useDerivedType = false, MObjectSet *filterModelObjects = nullptr);

    inline QList<MObjectType*> getModelObjectTypes() const;
    QList<MObjectType*> getRootModelObjectTypes();
    MObjectType *getModelObjectTypeByName(const QString &name);

    void dumpModelObjectTypeMap(const QString &msg = "") const;
    void dumpModel(const QString &msg = "") const;


    inline QString getDate() const;
    inline QString getExportDescription() const;
    inline QString getExportVersion() const;
    inline QString getToolName() const;
    inline uint    getId() const;


    bool operator ==(const Model &m); //!< We check that the ids of the mObjects match (not the object themselves as they will be different)

private:
    QMap<ElemId, MObject*> *_getModelObjectMap(MObjectType* mObjectType);

    void rebuildMapProperty(MapLinkProperty *mapProp);

    typedef bool (*SortElementView)(MObject*, MObject*);
    static QList<MObject *> _convertAndSortQSetToQList(const MObjectSet &elts, SortElementView sortFunction);
};

QList<MObjectType *> Model::getModelObjectTypes() const { return _mObjectTypeMap.keys(); }

QString Model::getDate() const { return _date; }
QString Model::getExportDescription() const { return _exportDescription; }
QString Model::getExportVersion() const { return _exportVersion; }
QString Model::getToolName() const { return _toolName; }
uint Model::getId() const { return _id; }

#endif // MODEL_H
