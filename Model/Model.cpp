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

#include "Element.h"
#include "Model.h"
#include <QtDebug>
#include "Model/ElementTypeFactory.h"


Model::Model(ElementTypeFactory *typeFactory,
             const QString &dataModel, const QString &version, const QString &desc,
             const QString &user, const QString &date, bool ownElements):
    _typeFactory(typeFactory), _elementTypeMap(), _nextElemId(), _ownElements(ownElements),
    _toolName(dataModel), _exportVersion(version), _exportDescription(desc), _user(user), _date(date)
{
}

Model::Model(Model &&other):
    _typeFactory(other._typeFactory),
    _elementTypeMap(std::move(other._elementTypeMap)),
    _nextElemId(std::move(other._nextElemId)),
    _ownElements(other._ownElements),
    _toolName(other._toolName), _exportVersion(other._exportVersion),
    _exportDescription(other._exportDescription),
    _user(other._user), _date(other._date)
{
    other._ownElements = false;
}

//Model &Model::operator=(Model &&other)
//{
//    _typeFactory       = other._typeFactory;
//    _elementTypeMap    = std::move(other._elementTypeMap);
//    _nextElemId        = std::move(other._nextElemId);
//    _toolName          = other._toolName;
//    _exportVersion     = other._exportVersion;
//    _exportDescription = other._exportDescription;
//    _user              = other._user;
//    _date              = other._date;
//    return this;
//}

Model::~Model()
{
    clearModel();
}

void Model::shallowCopySubsetOfMainModel(Model *mainModel, const QSet<Element *> &elementsToCopy)
{
    _ownElements = false;
    for (Element *elem : elementsToCopy)
        elem->exportWithLinksAsNewModelSharingSameElements(mainModel, this);
}



Model *Model::cloneSubset(const QSet<Element *> &mainElements)
{
    // First create the subModel without new Elements
    Model subModel(_typeFactory, _toolName, _exportVersion,
                   _exportDescription, _user, _date);
    subModel.shallowCopySubsetOfMainModel(this, mainElements);

    // Then we clone it
    return clone(&subModel);
}
#include <QRegularExpression>
QString Model::getCopyName(Element *elemToCopy) const
{
    QString srcName = elemToCopy->getName();
    ushort copyNumber = 0;
    const QRegularExpression copyReg(QString("^%1_copy(_(\\d+))?").arg(srcName));
    QMap<QString, Element*> *sameTypeElements = _elementTypeMap.value(elemToCopy->getElementType(), nullptr);
    // we have at least one element, the one to copy so the Map exists
    for (auto it = sameTypeElements->cbegin() ; it != sameTypeElements->cend() ; ++it)
    {
        Element *elemSameType = it.value();
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

    QString copyName = elemToCopy->getName();
    copyName += "_copy";
    if (copyNumber)
        copyName += QString("_%1").arg(++copyNumber);

    return copyName;
}


Model *Model::clone(Model *model)
{
    Model *clone = new Model(model->_typeFactory, model->_toolName, model->_exportVersion,
                             model->_exportDescription, model->_user, model->_date);

    // clone all the elements without the property map
    auto itStart = model->_elementTypeMap.cbegin(), itEnd = model->_elementTypeMap.cend();
    for (auto itType = itStart ; itType != itEnd ; ++itType)
    {
        ElementType             *type       = itType.key();
        QMap<QString, Element*> *elements   = itType.value();
        if (!elements->isEmpty())
        {
            QMap<QString, Element*> *newElements = new QMap<QString, Element*>();
            for (auto itElem = elements->cbegin() ; itElem != elements->cend() ; ++itElem)
               newElements->insert(itElem.key(), itElem.value()->shallowCopy());
            clone->_elementTypeMap[type] = newElements;
        }
    }

    // Now update the property map
    for (ElementType *type : model->_elementTypeMap.keys())
    {
        QMap<QString, Element*> *srcElements = model->_elementTypeMap.value(type),
                *newElements = clone->_elementTypeMap.value(type);
        if (!srcElements->isEmpty())
        {
            for (const QString &elemId : srcElements->keys())
            {
                Element *srcElem = srcElements->value(elemId),
                        *newElem = newElements->value(elemId);

                // copy all the property of elem into newElem using linked elements from the cloned model
                newElem->copyPropertiesFromSourceElementWithCloneElements(srcElem, clone);
            }
        }
    }
    return clone;
}





QMap<QString, Element*> *Model::_getElementMap(ElementType *elementType)
{
    if (!elementType->isInstanciable())
        qDebug() << "[ERROR][Model::_getElementMap] should not be called for non Instanciable ElementType " << elementType->getName();

    QMap<QString, Element*> *map = _elementTypeMap.value(elementType, nullptr);
    if (!map){
        map = new QMap<QString, Element*>();
        _elementTypeMap.insert(elementType, map);
    }

    return map;
}

QList<Element *> Model::_convertAndSortQSetToQList(const QSet<Element *> &elts, Model::SortElementView sortFunction)
{
    QList<Element*> eltList(elts.toList());
    std::sort(eltList.begin(),eltList.end(), *sortFunction);
    return eltList;
}

void Model::addNewElementWithIdCreation(Element *element)
{
    if (element)
    {
        ElementType *elementType = element->getElementType();

        uint nextId = _nextElemId.value(elementType, 0);
        _nextElemId[elementType] = ++nextId;
        element->setId(QString("%1_%2_%3").arg(elementType->getId()).arg(_user).arg(nextId));

        QMap<QString, Element*> *elementMap = _getElementMap(elementType);
        (*elementMap)[element->getId()] =  element;
        element->_state = Element::STATE::ADDED_IN_MODEL;
    }

}

void Model::add(ElementType *elementType, Element *element, bool updateElemState)
{
    if (element)
    {
        QMap<QString, Element*> *elementMap = _getElementMap(elementType);
        (*elementMap)[element->getId()] =  element;

        if (element->_state == Element::STATE::REMOVED_FROM_MODEL)
            element->makeVisibleForLinkedElements();

        if (updateElemState)
            element->_state = Element::STATE::ADDED_IN_MODEL;
    }
}

void Model::add(Element *element)
{
    add(element->getElementType(), element);
}

void Model::remove(Element *element)
{
    if (element)
    {
        QMap<QString, Element*> *elementMap = _getElementMap(element->getElementType());
        QMap<QString, Element*>::iterator it = elementMap->find(element->getId());
        if (it != elementMap->end())
        {
            elementMap->erase(it);

            element->_state = Element::STATE::REMOVED_FROM_MODEL;
            element->hideFromLinkedElements();
        }
    }
}

bool Model::contains(Element *element)
{
    QMap<QString, Element*> *map = _elementTypeMap.value(element->getElementType(), nullptr);
    if (map)
    {
        if (map->find(element->getId()) != map->cend())
            return true;
    }
    return false;
}

Element *Model::getElementById(ElementType* elementType, QString id)
{
    QSet<ElementType*> eltTypes(elementType->getInstanciableElementTypes());
    if (eltTypes.isEmpty())
    {
        qDebug() << "[ERROR][Model::getElements] ElementType '"
                 << elementType->getName() << "' is not instanciable and has no derived class that is";
    }

    for (ElementType * const eltType : eltTypes){
        QMap<QString, Element*> *elementMap = _elementTypeMap.value(eltType, nullptr);
        if (elementMap){
            auto it = elementMap->constFind(id);
            if ( it != elementMap->constEnd())
                return it.value();
        }
    }

    return nullptr;
}

QList<Element *> Model::getElementsOrderedByNames(ElementType *elementType, QSet<Element *> *filterElements)
{
    return _convertAndSortQSetToQList(
                getElements(elementType, filterElements),
                &Element::elementNameInsensitiveLessThan);
}

QList<ElementType *> Model::getRootElementTypes()
{
    return _typeFactory->getRootElementTypes();
}

ElementType *Model::getElementTypeByName(const QString &name)
{
    return _typeFactory->getElementTypeByName(name);
}

void Model::dumpElementTypeMap()
{
    qDebug() << "[Model::dumpElementTypeMap] Dumping Model:";
    for (auto it = _elementTypeMap.begin(); it != _elementTypeMap.end(); ++it)
    {
        ElementType             *elementType = it.key();
        QMap<QString, Element*> *elementMap  = it.value();
        qDebug() << "\tNb " << elementType->getLabel() << " : " << elementMap->size()
                 << " (addr: " << elementType << ")";
    }
}


bool Model::operator ==(const Model &m)
{
    if (m._typeFactory != _typeFactory)
        return false;

    QList<ElementType*> eltTypes = _elementTypeMap.keys();
    if (m._elementTypeMap.keys() != eltTypes)
        return false;

    for (ElementType *type : eltTypes)
    {
        if (m._elementTypeMap.value(type)->keys() != _elementTypeMap.value(type)->keys())
            return false;
    }

    return true;
}




void Model::clearModel()
{
    if (_ownElements)
    {
        auto itType = _elementTypeMap.begin();
        while (itType != _elementTypeMap.end()){
            QMap<QString, Element*> *elementMap = itType.value();
            qDeleteAll(*elementMap);
            delete elementMap;
            itType = _elementTypeMap.erase(itType);
        }
    }
}


QSet<Element*> Model::getElements(ElementType* elementType, QSet<Element*>* filterElements)
{
    QSet<ElementType*> eltTypes(elementType->getInstanciableElementTypes());
    if (eltTypes.isEmpty())
    {
        qDebug() << "[ERROR][Model::getElements] ElementType '"
                 << elementType->getName() << "' is not instanciable and has no derived class that is";
    }

    QSet<Element*> elements;
    for (ElementType *eltType : eltTypes)
    {
        QMap<QString, Element*>* elementMap = _elementTypeMap.value(eltType, nullptr);
        if (!elementMap)
        {
            qDebug() << "[ERROR][Model::getElements] can't find ElementType '"
                     << eltType->getName() << "' in global map elementTypeMap...";
        }
        else
        {
            auto itEnd = elementMap->end();
            for (auto it = elementMap->begin(); it != itEnd; ++it)
            {
                Element *elem = it.value();
                if(filterElements == nullptr || !filterElements->contains(elem))
                    elements.insert(elem);
            }
        }
    }
    return elements;
}
