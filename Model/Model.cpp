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

#include "MObject.h"
#include "Model.h"
#include <QtDebug>
#include "Model/MObjectTypeFactory.h"


Model::Model(MObjectTypeFactory *typeFactory,
             const QString &dataModel, const QString &version, const QString &desc,
             uint id, const QString &date, bool ownElements):
    _typeFactory(typeFactory), _mObjectTypeMap(), _nextElemId(), _ownModelObjects(ownElements),
    _toolName(dataModel), _exportVersion(version), _exportDescription(desc), _id(id), _date(date)
{
}

Model::Model(Model &&other):
    _typeFactory(other._typeFactory),
    _mObjectTypeMap(std::move(other._mObjectTypeMap)),
    _nextElemId(std::move(other._nextElemId)),
    _ownModelObjects(other._ownModelObjects),
    _toolName(other._toolName), _exportVersion(other._exportVersion),
    _exportDescription(other._exportDescription),
    _id(other._id), _date(other._date)
{
    other._ownModelObjects = false;
}

//Model &Model::operator=(Model &&other)
//{
//    _typeFactory       = other._typeFactory;
//    _mObjectTypeMap    = std::move(other._mObjectTypeMap);
//    _nextElemId        = std::move(other._nextElemId);
//    _toolName          = other._toolName;
//    _exportVersion     = other._exportVersion;
//    _exportDescription = other._exportDescription;
//    _id              = other._id;
//    _date              = other._date;
//    return this;
//}

Model::~Model()
{
    qDebug() << "[MB_TRACE][Model::~Model] deleting model... _ownModelObjects: " << _ownModelObjects;
#ifdef __CASCADE_DELETION__
    clearModel(false);
#else
    clearModel();
#endif
}

void Model::shallowCopySubsetOfMainModel(const MObjectSet &elementsToCopy, const QSet<MObjectType *> &rootTypesToNotTake, bool onlyContainment)
{
    _ownModelObjects = false;
    for (MObject *mObj : elementsToCopy)
        mObj->exportWithLinksAsNewModelSharingSameModelObjects(this, rootTypesToNotTake, onlyContainment);
}



Model *Model::cloneSubset(const MObjectSet &mainElements)
{
    // First create the subModel without new Elements
    Model subModel(_typeFactory, _toolName, _exportVersion,
                   _exportDescription, _id, _date);
    subModel.shallowCopySubsetOfMainModel(mainElements);

    // Then we clone it
    return clone(&subModel);
}
#include <QRegularExpression>
QString Model::getCopyName(MObject *mObjToCopy) const
{
    QString srcName = mObjToCopy->getName();
    int copyNumber = 0;
    const QRegularExpression copyReg(QString("^%1_copy(_(\\d+))?").arg(srcName));
    QMap<QString, MObject*> *sameTypeElements = _mObjectTypeMap.value(mObjToCopy->getModelObjectType(), nullptr);
    // we have at least one mObject, the one to copy so the Map exists
    for (auto it = sameTypeElements->cbegin(), itEnd = sameTypeElements->cend(); it != itEnd ; ++it)
    {
        MObject *elemSameType = it.value();
        QRegularExpressionMatch match = copyReg.match(elemSameType->getName());
        if (match.hasMatch())
        {
            if (!match.captured(1).isEmpty())
            {
                int currentCopyNumber =  match.captured(2).toInt();
                if (currentCopyNumber > copyNumber)
                    copyNumber = currentCopyNumber;
            }
            else if (!copyNumber)
                copyNumber = 1;
        }
    }

    QString copyName = mObjToCopy->getName();
    copyName += "_copy";
    if (copyNumber)
        copyName += QString("_%1").arg(++copyNumber);

    return copyName;
}


Model *Model::clone(Model *model)
{
    Model *clone = new Model(model->_typeFactory, model->_toolName, model->_exportVersion,
                             model->_exportDescription, model->_id, model->_date);

    // clone all the mObjects without the property map
    auto itStart = model->_mObjectTypeMap.cbegin(), itEnd = model->_mObjectTypeMap.cend();
    for (auto itType = itStart ; itType != itEnd ; ++itType)
    {
        MObjectType             *type       = itType.key();
        QMap<ElemId, MObject*> *mObjects   = itType.value();
        if (!mObjects->isEmpty())
        {
            QMap<ElemId, MObject*> *newModelObjects = new QMap<ElemId, MObject*>();
            for (auto itElem = mObjects->cbegin(), itElemEnd =  mObjects->cend(); itElem != itElemEnd ; ++itElem)
               newModelObjects->insert(itElem.key(), itElem.value()->shallowCopy());
            clone->_mObjectTypeMap[type] = newModelObjects;
        }
    }

    // Now update the property map
    for (MObjectType *type : model->_mObjectTypeMap.keys())
    {
        QMap<QString, MObject*> *srcElements = model->_mObjectTypeMap.value(type),
                *newModelObjects = clone->_mObjectTypeMap.value(type);
        if (!srcElements->isEmpty())
        {
            for (const QString &elemId : srcElements->keys())
            {
                MObject *srcElem = srcElements->value(elemId),
                        *newModelObject = newModelObjects->value(elemId);

                // copy all the property of mObj into newModelObject using linked mObjects from the cloned model
                if (newModelObject)
                    newModelObject->copyPropertiesFromSourceElementWithCloneElements(srcElem, clone);
                else
                    qCritical() << "[Model::clone] the object wasn't clone..." << srcElem->getModelObjectTypeName()
                                << ", id: " << elemId;
            }
        }
    }
    return clone;
}





QMap<ElemId, MObject *> *Model::_getModelObjectMap(MObjectType *mObjectType)
{
    if (!mObjectType->isInstanciable())
        qDebug() << "[ERROR][Model::_getModelObjectMap] should not be called for non Instanciable MObjectType " << mObjectType->getName();

    QMap<ElemId, MObject*> *map = _mObjectTypeMap.value(mObjectType, nullptr);
    if (!map){
        map = new QMap<ElemId, MObject*>();
        _mObjectTypeMap.insert(mObjectType, map);
    }

    return map;
}

#include "Model/Property.h"
void Model::rebuildMapProperty(MapLinkProperty *mapProp)
{
    MObjectType *objType = mapProp->getModelObjectType();
    for (MObject *obj : getModelObjects(objType))
    {
        MObjectMap map;
        for (MObject *propObj : mapProp->getLinkedModelObjects(obj))
            map.insert(propObj->getPropertyMapKey(mapProp), propObj);
        mapProp->setValuesFromMap(obj, &map);
    }
}

MObjectList Model::_convertAndSortQSetToQList(const MObjectSet &elts, Model::SortElementView sortFunction)
{
    MObjectList eltList(elts.toList());
    std::sort(eltList.begin(),eltList.end(), *sortFunction);
    return eltList;
}

void Model::add(MObjectType *mObjectType, MObject *mObject, bool updateElemState)
{
    if (mObject)
    {
        QMap<QString, MObject*> *mObjectMap = _getModelObjectMap(mObjectType);
        (*mObjectMap)[mObject->getId()] =  mObject;

        if (mObject->_state == MObject::STATE::REMOVED_FROM_MODEL)
            mObject->makeVisibleForLinkedModelObjects();

        if (updateElemState)
            mObject->_state = MObject::STATE::ADDED_IN_MODEL;
    }
}

void Model::add(MObject *mObject)
{
    add(mObject->getModelObjectType(), mObject);
}

void Model::remove(MObject *mObject, bool hideFromOtherObjects)
{
    if (mObject)
    {
        QMap<QString, MObject*> *mObjectMap = _getModelObjectMap(mObject->getModelObjectType());
        QMap<QString, MObject*>::iterator it = mObjectMap->find(mObject->getId());
        if (it != mObjectMap->end())
            mObjectMap->erase(it);

        mObject->_state = MObject::STATE::REMOVED_FROM_MODEL;
        if (hideFromOtherObjects)
            mObject->hideFromLinkedModelObjects();
    }
}

bool Model::contains(MObject *mObject)
{
    QMap<QString, MObject*> *map = _mObjectTypeMap.value(mObject->getModelObjectType(), nullptr);
    if (map)
    {
        if (map->find(mObject->getId()) != map->cend())
            return true;
    }
    return false;
}



MObject *Model::getModelObjectById(MObjectType* mObjectType, const QString &id)
{
    QSet<MObjectType*> eltTypes(mObjectType->getInstanciableModelObjectTypes());
    if (eltTypes.isEmpty())
    {
        qDebug() << "[ERROR][Model::getModelObjects] MObjectType '"
                 << mObjectType->getName() << "' is not instanciable and has no derived class that is";
    }

    for (MObjectType * const eltType : eltTypes){
        QMap<QString, MObject*> *mObjectMap = _mObjectTypeMap.value(eltType, nullptr);
        if (mObjectMap){
            auto it = mObjectMap->constFind(id);
            if ( it != mObjectMap->constEnd())
                return it.value();
        }
    }

    return nullptr;
}

MObject *Model::getModelObjectByName(MObjectType *mObjectType, const QString &name)
{
    QSet<MObjectType*> eltTypes(mObjectType->getInstanciableModelObjectTypes());
    if (eltTypes.isEmpty())
    {
        qDebug() << "[ERROR][Model::getModelObjects] MObjectType '"
                 << mObjectType->getName() << "' is not instanciable and has no derived class that is";
    }

    for (MObjectType * const eltType : eltTypes){
        QMap<QString, MObject*> *mObjectMap = _mObjectTypeMap.value(eltType, nullptr);
        if (mObjectMap)
        {
            for (auto it = mObjectMap->cbegin() , itEnd = mObjectMap->cend(); it != itEnd ; ++it)
            {
                if (it.value()->getName() == name)
                    return it.value();
            }
        }
    }

    return nullptr;
}

MObjectList Model::getModelObjectsOrderedByNames(MObjectType *mObjectType, bool useDerivedType, MObjectSet *filterModelObjects)
{
    return _convertAndSortQSetToQList(
                getModelObjects(mObjectType, useDerivedType, filterModelObjects),
                &MObject::elementNameInsensitiveLessThan);
}

QList<MObjectType *> Model::getRootModelObjectTypes()
{
    return _typeFactory->getRootModelObjectTypes();
}

MObjectType *Model::getModelObjectTypeByName(const QString &name)
{
    return _typeFactory->getModelObjectTypeByName(name);
}

void Model::dumpModelObjectTypeMap(const QString &msg) const
{
    qDebug() << "[Model::dumpModelObjectTypeMap] Dumping Model: "  << msg;
    for (auto it = _mObjectTypeMap.cbegin(), itEnd = _mObjectTypeMap.cend(); it != itEnd; ++it)
    {
        MObjectType             *mObjectType = it.key();
        QMap<QString, MObject*> *mObjectMap  = it.value();
        qDebug() << "\tNb " << mObjectType->getLabel() << " : " << mObjectMap->size()
                 << " (addr: " << mObjectType << ")";
    }
}


bool Model::operator ==(const Model &m)
{
    if (m._typeFactory != _typeFactory)
        return false;

    QList<MObjectType*> eltTypes = _mObjectTypeMap.keys();
    if (m._mObjectTypeMap.keys() != eltTypes)
        return false;

    for (MObjectType *type : eltTypes)
    {
        if (m._mObjectTypeMap.value(type)->keys() != _mObjectTypeMap.value(type)->keys())
            return false;
    }

    return true;
}


void Model::resetTypesNumberOfModelObjects()
{
    for (auto itType = _mObjectTypeMap.cbegin(), itTypeEnd = _mObjectTypeMap.cend(); itType != itTypeEnd; ++itType)
        itType.key()->_nbModelObjects = 0;
}

void Model::clearModel(bool deleteModelObjects)
{
    if (_ownModelObjects)
    {
        auto itType = _mObjectTypeMap.begin(), itTypeEnd = _mObjectTypeMap.end();
        while (itType != itTypeEnd){
            QMap<QString, MObject*> *mObjectMap = itType.value();
            if (deleteModelObjects)
                qDeleteAll(*mObjectMap);
            delete mObjectMap;
            itType = _mObjectTypeMap.erase(itType);
        }
    }
}

void Model::validate(QStringList &compilationErrors, const QSet<MObjectType *> &typesToExclude)
{
    for (auto itType = _mObjectTypeMap.cbegin(), itTypeEnd = _mObjectTypeMap.cend() ; itType != itTypeEnd ; ++itType)
    {
        QMap<QString, MObject*> *modelObjects = itType.value();
        for (auto itObj = modelObjects->begin(), itObjEnd = modelObjects->end() ; itObj != itObjEnd ; ++itObj)
        {
            MObject *modelObj = itObj.value();
            modelObj->validateLinkProperties(compilationErrors);
            if (!typesToExclude.contains(itType.key()))
                modelObj->validateBusinessRules(compilationErrors);
        }
    }
}

void Model::validateModel(QStringList &compilationErrors)
{
    for (auto itType = _mObjectTypeMap.cbegin(), itTypeEnd = _mObjectTypeMap.cend() ; itType != itTypeEnd ; ++itType)
    {
        QMap<QString, MObject*> *modelObjects = itType.value();
        for (auto itObj = modelObjects->begin(), itObjEnd = modelObjects->end() ; itObj != itObjEnd ; ++itObj)
            itObj.value()->validateLinkProperties(compilationErrors);
    }
}

void Model::validateBusinessRules(QStringList &compilationErrors, const QSet<MObjectType *> &typesToExclude)
{
    for (auto itType = _mObjectTypeMap.cbegin(), itTypeEnd = _mObjectTypeMap.cend() ; itType != itTypeEnd ; ++itType)
    {
        QMap<QString, MObject*> *modelObjects = itType.value();
        for (auto itObj = modelObjects->begin(), itObjEnd = modelObjects->end() ; itObj != itObjEnd ; ++itObj)
        {
            if (!typesToExclude.contains(itType.key()))
                itObj.value()->validateBusinessRules(compilationErrors);
        }
    }
}


MObjectSet Model::getModelObjects(MObjectType* mObjectType, bool useDerivedType, MObjectSet* filterModelObjects)
{
    QSet<MObjectType*> eltTypes = {mObjectType};

    if (useDerivedType)
    {
        eltTypes = mObjectType->getInstanciableModelObjectTypes();
        if (eltTypes.isEmpty())
            qDebug() << "[ERROR][Model::getModelObjects] MObjectType '"
                     << mObjectType->getName() << "' is not instanciable and has no derived class that is";
    }

    MObjectSet mObjects;
    for (MObjectType *eltType : eltTypes)
    {
        QMap<QString, MObject*>* mObjectMap = _mObjectTypeMap.value(eltType, nullptr);
#ifdef __MB_TRACE_MODEL__
        if (!mObjectMap)
        {
            qDebug() << "[ERROR][Model::getModelObjects] can't find MObjectType '"
                     << eltType->getName() << "' in global map elementTypeMap...";
        }
        else
#else
        if (mObjectMap)
#endif
        {
            for (auto it = mObjectMap->begin(), itEnd = mObjectMap->end(); it != itEnd; ++it)
            {
                MObject *mObj = it.value();
                if(filterModelObjects == nullptr || !filterModelObjects->contains(mObj))
                    mObjects.insert(mObj);
            }
        }
    }
    return mObjects;
}

QMap<ElemId, MObject *> Model::getModelObjectsAsMap(MObjectType *mObjectType, bool useDerivedType, MObjectSet *filterModelObjects)
{
    QSet<MObjectType*> eltTypes = {mObjectType};

    if (useDerivedType)
    {
        eltTypes = mObjectType->getInstanciableModelObjectTypes();
        if (eltTypes.isEmpty())
            qDebug() << "[ERROR][Model::getModelObjects] MObjectType '"
                     << mObjectType->getName() << "' is not instanciable and has no derived class that is";
    }

    QMap<ElemId, MObject *> resMap;
    for (MObjectType *eltType : eltTypes)
    {
        QMap<QString, MObject*>* mObjectMap = _mObjectTypeMap.value(eltType, nullptr);
#ifdef __MB_TRACE_MODEL__
        if (!mObjectMap)
        {
            qDebug() << "[ERROR][Model::getModelObjects] can't find MObjectType '"
                     << eltType->getName() << "' in global map elementTypeMap...";
        }
        else
#else
        if (mObjectMap)
#endif
        {
            for (auto it = mObjectMap->begin(), itEnd = mObjectMap->end(); it != itEnd; ++it)
            {
                MObject *mObj = it.value();
                if(filterModelObjects == nullptr || !filterModelObjects->contains(mObj))
                    resMap.insert(mObj->getId(), mObj);
            }
        }
    }
    return resMap;
}
