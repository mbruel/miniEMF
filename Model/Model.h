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

#include <QString>
#include <QMap>
#include <QSet>


class Element;
class ElementType;
class ElementTypeFactory;
class PropertyFactory;

class Model
{
    friend class XmiWriter; // to access _typeFactory
    friend class XMIService; // for exports

private:
    ElementTypeFactory *_typeFactory;
    QMap<ElementType*, QMap<QString, Element*>* > _elementTypeMap;
    QMap<ElementType*, uint> _nextElemId;

    bool          _ownElements; //!< set to false for subModels, no destuction of the ELements in destructor

    const QString _toolName;
    const QString _exportVersion;
    const QString _exportDescription;
    const QString _user;
    const QString _date;


public:
    Model(ElementTypeFactory *typeFactory, const QString &dataModel,
          const QString &version, const QString &desc, const QString &user,
          const QString &date, bool ownElements = true);

    // Moveable from but not copyable (so we can use Element::exportAsModel() )
    Model(Model &&other);
    Model(const Model &other)             = delete;
    Model & operator=(Model &&other)      = delete;
    Model & operator=(const Model &other) = delete;

    ~Model();
    void shallowCopySubsetOfMainModel(Model *mainModel, const QSet<Element*> &elementsToCopy);
    Model *cloneSubset(const QSet<Element*> &mainElements);
    QString getCopyName(Element *elemToCopy) const;

    static Model *clone(Model *model);

    void addNewElementWithIdCreation(Element *element);
    void add(ElementType *elementType, Element *element, bool updateElemState = true);
    void add(Element *element);
    void remove(Element *element);

    bool contains(Element *element);

    void clearModel();

    QSet<Element *> getElements(ElementType *elementType, QSet<Element *> *filterElements = nullptr);
    Element*        getElementById(ElementType *elementType, QString id);

    QList<Element *> getElementsOrderedByNames(ElementType *elementType, QSet<Element *> *filterElements = nullptr);

    QList<ElementType*> getRootElementTypes();
    ElementType *getElementTypeByName(const QString &name);

    void dumpElementTypeMap();

    inline QString getDate() const;
    inline QString getExportDescription() const;
    inline QString getExportVersion() const;
    inline QString getToolName() const;
    inline QString getUser() const;


    bool operator ==(const Model &m); //!< We check that the ids of the elements match (not the object themselves as they will be different)

private:
    QMap<QString, Element*> *_getElementMap(ElementType* elementType);

    typedef bool (*SortElementView)(Element*, Element*);
    static QList<Element *> _convertAndSortQSetToQList(const QSet<Element*> &elts, SortElementView sortFunction);
};

QString Model::getDate() const { return _date; }
QString Model::getExportDescription() const { return _exportDescription; }
QString Model::getExportVersion() const { return _exportVersion; }
QString Model::getToolName() const { return _toolName; }
QString Model::getUser() const { return _user; }

#endif // MODEL_H
