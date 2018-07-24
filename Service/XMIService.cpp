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

#include "XMIService.h"
#include "Model/Model.h"
#include "Utils/XmiWriter.h"

#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QDomDocument>

#include <Model/Property.h>


XMIService::XMIService() :Singleton<XMIService>(), _docXMI(nullptr), _model(nullptr)
{}

XMIService::~XMIService()
{
    if (_docXMI)
        delete _docXMI;
}

#define MB_TRACE_loadXMI 1
Element *XMIService::deserializeElement(QDomNode node, ElementType *elementType, QSet<ElementLinking *> *elementLinkings)
{
    QString nodeName (node.toElement().tagName());
    QString strId    (node.toElement().attribute("xmi:id", "").trimmed());

    Element *element = _model->getElementById(elementType, strId);
    if(!element)
    {
#ifdef MB_TRACE_loadXMI
        qDebug() << "[MB_TRACE] create new Element of type: " <<   elementType->getName();
#endif
        element = elementType->createElement();
        initFromNode(element, node);
        _model->add(elementType, element);
    }
#ifdef MB_TRACE_loadXMI
    qDebug() << "\n\n[MB_TRACE] deserializeElement element: " << element->getName()
             << " type: " << element->getElementTypeName()
             << " (id: " << element->getId();
#endif


    QMap<QString, LinkProperty*> containmentProperties;
    for (Property *const property : element->getPropertyList())
    {
#ifdef MB_TRACE_loadXMI
        qDebug() << "[MB_TRACE] deserializeElement property: " << property->getName();
#endif
        if (property->isALinkProperty()) // 1> Treatment of a link Property
        {
            LinkProperty *linkProperty = static_cast<LinkProperty*>(property);
            if (linkProperty->isEcoreContainment()) // 1.1> Containment properties are treated after this loop, by "Deserialize Child nodes"
                containmentProperties.insert(linkProperty->getName(), linkProperty);
            else if (linkProperty->isEcoreContainer()) // 1.2> Container property is deserialized (both sides of the relationship) directly here
                continue; // Nothing to do, it is set by the parent
            else // 1.3> Other link propeties are stored for a post treatment (treated when all elements have been identified)
            {
                // Get literal value from XMI and create intermediate object
                QString strAttr = node.toElement().attribute(property->getName(), "");
                if (!strAttr.isEmpty())
                {
#ifdef MB_TRACE_loadXMI
                    qDebug()<< "[MB_TRACE] elementLinkings->insert : elem: " << element->getName()
                            << ", linkProperty: " << linkProperty->getName()
                            <<" (type: " << linkProperty->getElementType()->getName()
                           << ", linkedElementType: " << linkProperty->getLinkedElementType()->getName() << ") value : " << strAttr;
#endif
                    elementLinkings->insert(new ElementLinking(element, linkProperty, strAttr));
                }
            }
        }
        else // 2> Treatment of a not link property
        {
            // Get literal value from XMI and deserialize it
            QString strAttr = node.toElement().attribute(property->getName(), "");
            property->deserializeFromXmiAttribute(element, strAttr);
        }
    }


    // Deserialize Child nodes
    QDomNodeList childNodes = node.childNodes();
    for(int i = 0; i < childNodes.length(); ++i)
    {
        QDomNode childNode    = childNodes.item(i);
        QString  childTagName = childNode.toElement().tagName();

        // Identification of Child node's Element Type
        LinkProperty *linkProperty     = containmentProperties[childTagName];
        ElementType  *childElementType = linkProperty->getLinkedElementType();

        if (childElementType->isDerived())
        {
            QString xsiTypeValue = childNode.toElement().attribute("xsi:type", "");
            if (!xsiTypeValue.isEmpty())
            {
                QStringList xsiTypeValueSplitted = xsiTypeValue.split(":");
                if (xsiTypeValueSplitted.size() == 2)
                {
                    QString realEltTypeName = xsiTypeValueSplitted.at(1).trimmed();
                    childElementType = _model->getElementTypeByName(realEltTypeName);
                }
            }
        }
        // Deserialization of child node
        Element *childElement = deserializeElement(childNode, childElementType, elementLinkings);

if (elementType->getName() == "DataSet")
    qDebug() << "[MB_TRACE][deserializeElement] child for property " << linkProperty->getName();

        linkProperty->addLink(element, childElement);

        LinkToOneProperty *containerProp = static_cast<LinkToOneProperty*>(linkProperty->getReverseLinkProperty());
        if (containerProp)
            containerProp->setValue(childElement, element);
    }

    return element;
}

void XMIService::initFromNode(Element *element, const QDomNode &node)
{
    QString strId          (node.toElement().attribute("xmi:id", ""));
    QString strName        (node.toElement().attribute("name", ""));
//    QString strDescription (node.toElement().attribute("description", ""));

    element->setId(strId);
    element->setName(strName);
//    element->setDescription(strDescription);
}

bool XMIService::initImportXMI(const QString &xmiPath)
{
    if(_docXMI)
        delete _docXMI;

    _docXMI = new QDomDocument("XMI");
    QFile xmiFile(xmiPath);
    QString error;
    int errorLine, errorColumn;
    if (!_docXMI->setContent(&xmiFile, false, &error, &errorLine, &errorColumn))
    {
        QString errorMsg = "XMI Import Module: Error found in " + xmiPath + " at line ";
        errorMsg += QString::number(errorLine);
        errorMsg += ", column ";
        errorMsg += QString::number(errorColumn);
        errorMsg += " : ";
        errorMsg += error;
        qDebug() << errorMsg;

        return false;
    }

    return true;
}

void XMIService::loadXMI(Model *model)
{
    _model = model;

    QSet<ElementLinking*> elementLinks;
    for(QDomNode node = _docXMI->documentElement().firstChild(); !node.isNull(); node = node.nextSibling())
    {
        QString nodeType(node.nodeName()); //osam.functional:Function
        QStringList nodeTypePath = nodeType.split(':');
        if (nodeTypePath.size() == 2)
            nodeType = nodeTypePath.at(1);
        ElementType *elementType = _model->getElementTypeByName(nodeType);
        Q_ASSERT( elementType != 0);

        deserializeElement(node, elementType, &elementLinks);
    }

    // Log the number of elements loaded by elementType
    _model->dumpElementTypeMap();


    // Deserialize links between elements
    for (ElementLinking *elementLinking : elementLinks)
    {
        Element      *element      = elementLinking->getElement();
        LinkProperty *linkProperty = elementLinking->getLinkProperty();
        QString       linkValue(elementLinking->getLinkValue().trimmed());

        linkProperty->setValueFromXMIStringIdList(element, linkValue, _model);
/*
        if (linkProperty->isALinkToOneProperty())
        {

//if (linkProperty->getName() == "failedFct")
//{
//     qDebug() << "[MB_TRACE] [loadXMI]: elementLinking failedFct !!!"
//              << " linkToOneProperty->getLinkedElementType(): " << linkProperty->getLinkedElementType()->getName();
//}
            // Deserialization of LinkToOneProperty
            LinkToOneProperty *linkToOneProperty = static_cast<LinkToOneProperty*>(linkProperty);
            Element *linkedElt = _model->getElementById(linkToOneProperty->getLinkedElementType(), linkValue);
            linkToOneProperty->setValue(element, linkedElt);
#ifdef MB_TRACE_loadXMI
            if (linkedElt)
            {
                qDebug() << "[MB_TRACE] Element: " << element->getName()
                         << ", linkToOneProperty->getName: " << linkToOneProperty->getName()
                         << ", linkedElt: " << linkedElt->getName();
            } else
            {
                qDebug() << "[MB_TRACE] Element: " << element->getName()
                         << ", linkToOneProperty->getName: " << linkToOneProperty->getName()
                         << ", linkedElt: IS NULL!!!!";
            }
#endif
        }
        else
        {
            // Deserialization of LinkToManyProperty or OrderedLinkToManyProperty
#ifdef MB_TRACE_loadXMI
            qDebug() << "[MB_TRACE] Element: " << element->getName()
                     << ", linkProperty: " << linkProperty->getName();
            qDebug() << ", linkProperty->getLinkedElementType: " << linkProperty->getLinkedElementType()->getName()
                     << ", linkValue: " << linkValue;
#endif
            linkProperty->setValueFromXMIStringIdList(element, linkValue, _model);
        }
*/
    }

    qDeleteAll(elementLinks);

    delete _docXMI;
    _docXMI = nullptr;
    _model = nullptr;
}



bool XMIService::writeXMI(Model *model, const QString &xmiPath, const QString &applicationName, XmiWriter::XMI_TYPE xmiType)
{
    _model = model;
    // Open file in write mode
    QFile file(xmiPath);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "[ERROR][XMIService::writeXMI] Failed to create destination file: " << xmiPath;
        return false;
    }

    XmiWriter xmiWriter(model, &file);
    xmiWriter.writeStartTag(applicationName, xmiType);

    for (ElementType *rootType : _model->getRootElementTypes())
        xmiWriter.write(rootType);

    xmiWriter.writeEndDocument();
    file.close();
    _model = nullptr;
    return true;
}

bool XMIService::exportXMI(Element *elemToExport, Model *model, const QString &xmiPath, const QString &applicationName)
{
    Model subModel(model->_typeFactory, model->_toolName, model->_exportVersion,
                   model->_exportDescription, model->_user, model->_date);

    subModel.shallowCopySubsetOfMainModel(model, {elemToExport});
    return writeXMI(&subModel, xmiPath, applicationName, XmiWriter::XMI_TYPE::EXPORT);
}


