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

MObject *XMIService::deserializeModelObject(QDomNode node, MObjectType *mObjectType, QSet<MObjectLinkings *> *objectLinks)
{
    QString nodeName (node.toElement().tagName());
    QString strId    (node.toElement().attribute("id", "").trimmed());

    MObject *mObject = _model->getModelObjectById(mObjectType, strId);
    if(!mObject)
    {
#ifdef __MB_TRACE_loadXMI__
        qDebug() << "[MB_TRACE] create new MObject of type: " <<   mObjectType->getName();
#endif
        mObject = mObjectType->createModelObject(0, false); // we don't want the default initialization
        initFromNode(mObject, node);
        _model->add(mObjectType, mObject);
    }
#ifdef __MB_TRACE_loadXMI__
    qDebug() << "\n\n[MB_TRACE] deserializeModelObject mObject: " << mObject->getName()
             << " type: " << mObject->getModelObjectTypeName()
             << " (id: " << mObject->getId();
#endif


    QMap<QString, LinkProperty*> containmentProperties;
    for (Property *const property : mObject->getPropertyList())
    {
#ifdef __MB_TRACE_loadXMI__
        qDebug() << "[MB_TRACE] deserializeModelObject property: " << property->getName();
#endif
        if (property->isALinkProperty()) // 1> Treatment of a link Property
        {
            LinkProperty *linkProperty = static_cast<LinkProperty*>(property);
            if (linkProperty->isEcoreContainment()) // 1.1> Containment properties are treated after this loop, by "Deserialize Child nodes"
                containmentProperties.insert(linkProperty->getName(), linkProperty);
            else if (linkProperty->isEcoreContainer()) // 1.2> Container property is deserialized (both sides of the relationship) directly here
                continue; // Nothing to do, it is set by the parent
            else // 1.3> Other link propeties are stored for a post treatment (treated when all mObjects have been identified)
            {
                // Get literal value from XMI and create intermediate object
                QString strAttr = node.toElement().attribute(property->getName(), "");
                if (!strAttr.isEmpty())
                {
#ifdef __MB_TRACE_loadXMI__
                    qDebug()<< "[MB_TRACE] elementLinkings->insert : mObj: " << mObject->getName()
                            << ", linkProperty: " << linkProperty->getName()
                            <<" (type: " << linkProperty->getModelObjectType()->getName()
                           << ", linkedModelObjectType: " << linkProperty->getLinkedModelObjectType()->getName() << ") value : " << strAttr;
#endif
                    objectLinks->insert(new MObjectLinkings(mObject, linkProperty, strAttr));
                }
            }
        }
        else // 2> Treatment of a not link property
        {
            // Get literal value from XMI and deserialize it
            QString strAttr = node.toElement().attribute(property->getName(), "");
            property->deserializeFromXmiAttribute(mObject, strAttr);
        }
    }

    // Deserialize Child nodes
    QDomNodeList childNodes = node.childNodes();
    for(int i = 0; i < childNodes.length(); ++i)
    {
        QDomNode childNode    = childNodes.item(i);
        QString  childTagName = childNode.toElement().tagName();

        // Identification of Child node's MObject Type
        LinkProperty *linkProperty     = containmentProperties[childTagName];
        if (!linkProperty)
        {
            qDebug() << "[XMIService::deserializeModelObject] ERROR xmi: the property '"
                     << childTagName << "' doesn't exist for the object: " << mObjectType->getName();
            continue;
        }
        MObjectType  *childModelObjectType = linkProperty->getLinkedModelObjectType();

        if (childModelObjectType->isDerived())
        {
            QString xsiTypeValue = childNode.toElement().attribute("MObjectType", "");
            if (!xsiTypeValue.isEmpty())
            {
                QStringList xsiTypeValueSplitted = xsiTypeValue.split(":");
                if (xsiTypeValueSplitted.size() == 2)
                {
                    QString realEltTypeName = xsiTypeValueSplitted.at(1).trimmed();
                    childModelObjectType = _model->getModelObjectTypeByName(realEltTypeName);
                }
            }
        }
        // Deserialization of child node
        MObject *childElement = deserializeModelObject(childNode, childModelObjectType, objectLinks);
        linkProperty->addLink(mObject, childElement);
        LinkToOneProperty *containerProp = static_cast<LinkToOneProperty*>(linkProperty->getReverseLinkProperty());
        if (containerProp)
            containerProp->setValue(childElement, mObject);
    }

    return mObject;
}

#include <QRegularExpression>
void XMIService::initFromNode(MObject *mObject, const QDomNode &node)
{
    QString strId          (node.toElement().attribute("id", ""));
    QString strName        (node.toElement().attribute("name", ""));
//    QString strDescription (node.toElement().attribute("description", ""));

    mObject->setId(strId);
    mObject->setName(strName);
//    mObject->setDescription(strDescription);
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

void XMIService::loadXMI(Model *model, bool createDefaultObjects)
{
    Q_UNUSED(createDefaultObjects);

    _model = model;

    QSet<MObjectLinkings*> objectLinks;
    for(QDomNode node = _docXMI->documentElement().firstChild(); !node.isNull(); node = node.nextSibling())
    {
        QString nodeType(node.nodeName()); //osam.functional:Function
        QStringList nodeTypePath = nodeType.split(':');
        if (nodeTypePath.size() == 2)
            nodeType = nodeTypePath.at(1);
        MObjectType *mObjectType = _model->getModelObjectTypeByName(nodeType);
        Q_ASSERT( mObjectType != nullptr);

        deserializeModelObject(node, mObjectType, &objectLinks);
    }


    // add default objects, if needed
//    if (createDefaultObjects)
//        MyApplication::getInstance()->createModelDefaultObjects();



    // Deserialize links between mObjects
    for (MObjectLinkings *objLink : objectLinks)
    {
        LinkProperty *linkProperty = objLink->linkProp;
        linkProperty->setValueFromXMIStringIdList(objLink->mObj, objLink->linkValue, model);
    }

    qDeleteAll(objectLinks);

    delete _docXMI;
    _docXMI = nullptr;
    _model = nullptr;
}



bool XMIService::writeXMI(Model *model, const QString &xmiPath, const QString &applicationName, XmiWriter::XMI_TYPE xmiType)
{
model->dumpModelObjectTypeMap("[MB_TRACE] saving xmi...");

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

    for (MObjectType *rootType : _model->getRootModelObjectTypes())
        xmiWriter.write(rootType);

    xmiWriter.writeEndDocument();
    file.close();
    _model = nullptr;
    return true;
}

bool XMIService::exportXMI(MObject *elemToExport, Model *model, const QString &xmiPath, const QString &applicationName)
{
    Model subModel(model->_typeFactory, model->_toolName, model->_exportVersion,
                   model->_exportDescription, model->_id, model->_date);

    subModel.shallowCopySubsetOfMainModel({elemToExport});
    return writeXMI(&subModel, xmiPath, applicationName, XmiWriter::XMI_TYPE::EXPORT);
}

#include <QXmlStreamReader>


MObject *XMIService::deserializeModelObject(
        Model *model,
        QXmlStreamReader &xmlReader,
        MObjectType *mObjectType,
        const QString &endTag,
        QSet<MObjectLinkings *> &elementLinkings)
{
    MObject *mObject = mObjectType->createModelObject(0, false); // we don't want the default initialization
    mObject->setId(xmlReader.attributes().value("id").toString());

    model->add(mObjectType, mObject);

    qDebug() << "[XMIService::deserializeModelObject] >>>>> new " << mObjectType->getName()
             << ": " << mObject->getName() << " (id: " << mObject->getId() << ")"
             << " endTag: " << endTag;

    // First do all the non containment properties that are on the current line
    QMap<QString, Property *> nonContainmentProps = mObject->getNonContainmentProperties();
    for (auto it = nonContainmentProps.cbegin(), itEnd = nonContainmentProps.cend(); it != itEnd ; ++it)
    {
        const QString & propName = it.key();
        Property *const property = it.value();
        QString strAttr = xmlReader.attributes().value(propName).toString();
        if (property->isAttributeProperty())
            property->deserializeFromXmiAttribute(mObject, strAttr);
        else
            elementLinkings.insert(new MObjectLinkings(mObject, static_cast<LinkProperty*>(property), strAttr));
    }

    // Now all the containment properties (childs) until we reach the endTag
    QMap<QString, LinkProperty *>  containmentProps = mObject->getContainmentProperties();
    do
    {
        QXmlStreamReader::TokenType tokenType = xmlReader.readNext();
        if (xmlReader.name() == endTag && tokenType == QXmlStreamReader::EndElement)
        {
            qDebug() << "[XMIService::deserializeModelObject] <<<< end " << mObjectType->getName()
                     << ": " << mObject->getName() << " (id: " << mObject->getId() << ")"
                     << " endTag: " << endTag;
            break;
        }

        QString       propertyName = xmlReader.name().toString();
        if (propertyName.isEmpty())
            continue; // avoid TokenType::Characters

        LinkProperty *linkProperty = containmentProps.value(propertyName);
        if (!linkProperty)
        {
            qDebug() << "[XMIService::loadProject] Error on " << mObjectType->getName()
                     << ": couldn't find child named: " << propertyName
                     << " (object id: " << mObject->getId() << ")"
                     << " endTag: " << endTag
                     << ", tokenType: " << tokenType
                     << ", error: " << xmlReader.errorString();
        }
        else
        {
            MObjectType *childType = linkProperty->getLinkedModelObjectType();
            if (childType->isDerived())
            {
                QString xsiTypeValue = xmlReader.attributes().value("MObjectType").toString();
                if (!xsiTypeValue.isEmpty())
                {
                    QStringList xsiTypeValueSplitted = xsiTypeValue.split(":");
                    if (xsiTypeValueSplitted.size() == 2)
                    {
                        QString realChildType = xsiTypeValueSplitted.at(1).trimmed();
                        childType = model->getModelObjectTypeByName(realChildType);
                        qDebug() << "[XMIService::deserializeModelObject] " << mObjectType->getName()
                                 << ": " << mObject->getName() << " (id: " << mObject->getId() << ")"
                                 << " has a derived child: " << realChildType;

                    }
                }
            }
            MObject *childElement = deserializeModelObject(model, xmlReader, childType, propertyName, elementLinkings);
            linkProperty->addLink(mObject, childElement);
            LinkToOneProperty *containerProp = static_cast<LinkToOneProperty*>(linkProperty->getReverseLinkProperty());
            if (containerProp)
                containerProp->setValue(childElement, mObject);
        }
    } while (!xmlReader.atEnd());

    return mObject;
}


