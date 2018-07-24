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

#ifndef XMISERVICE_H
#define XMISERVICE_H

#include "Utils/Singleton.h"
#include <QString>
#include <QVariant>
#include "Model/Element.h"
#include "Utils/XmiWriter.h"

class QDomDocument;
class QDomNode;
class QXmlStreamWriter;
class QFile;

class Model;
class ElementLinking;


class XMIService : public Singleton<XMIService>
{
    friend class Singleton<XMIService>;

public:

    bool initImportXMI(const QString &xmiPath);
    void loadXMI(Model *model);
    bool writeXMI(Model *model, const QString &xmiPath, const QString &applicationName,
                  XmiWriter::XMI_TYPE xmiType = XmiWriter::XMI_TYPE::FULL_DUMP);

    bool exportXMI(Element *elemToExport, Model *model, const QString &xmiPath, const QString &applicationName);

private:
    XMIService();
    ~XMIService();

    QDomDocument *_docXMI;
    Model        *_model;

    Element *deserializeElement(QDomNode node, ElementType *elementType, QSet<ElementLinking *> *elementLinkings);

    void initFromNode(Element *element, const QDomNode &node);

    void setNNPropertyValue(QString ids, LinkToManyProperty *property, Element *element);
    void setNNOrderedPropertyValue(QString ids, OrderedLinkToManyProperty *property, Element *element);
};


class ElementLinking
{
public:
    ElementLinking(Element *const element, LinkProperty *const linkProperty, const QString &linkValue):
        _element(element), _linkProperty(linkProperty), _linkValue(linkValue) {}
    ~ElementLinking(){}

    Element      *getElement()      const { return _element; }
    LinkProperty *getLinkProperty() const { return _linkProperty; }
    QString       getLinkValue()    const { return _linkValue; }

private:
    Element       *const _element;
    LinkProperty  *const _linkProperty;
    const QString        _linkValue;
};

#endif // XMISERVICE_H
