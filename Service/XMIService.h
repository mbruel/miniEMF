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
#include "Model/MObject.h"
#include "Utils/XmiWriter.h"

class QDomDocument;
class QDomNode;
class QXmlStreamWriter;
class QFile;

class Model;
class MObjectLinkings;
class QXmlStreamReader;

class XMIService : public Singleton<XMIService>
{
    friend class Singleton<XMIService>;

public:

    bool initImportXMI(const QString &xmiPath);
    void loadXMI(Model *model, bool createDefaultObjects = true);
    bool writeXMI(Model *model, const QString &xmiPath, const QString &applicationName,
                  XmiWriter::XMI_TYPE xmiType = XmiWriter::XMI_TYPE::FULL_DUMP);

    bool exportXMI(MObject *elemToExport, Model *model, const QString &xmiPath, const QString &applicationName);



    MObject *deserializeModelObject(Model *model,
                                    QXmlStreamReader &xmlReader,
                                    MObjectType *mObjectType,
                                    const QString &endTag,
                                    QSet<MObjectLinkings *> &elementLinkings);

private:
    XMIService();
    ~XMIService();

    QDomDocument *_docXMI;
    Model        *_model;

    MObject *deserializeModelObject(QDomNode node, MObjectType *mObjectType, QSet<MObjectLinkings *> *objectLinks);

    void initFromNode(MObject *mObject, const QDomNode &node);

    void setNNPropertyValue(QString ids, LinkToManyProperty *property, MObject *mObject);
    void setNNOrderedPropertyValue(QString ids, OrderedLinkToManyProperty *property, MObject *mObject);
};


#endif // XMISERVICE_H
