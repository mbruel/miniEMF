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

#include "XmiWriter.h"
#include "Model/MObject.h"
#include "Model/Property.h"
#include <QXmlStreamWriter>
#include <QFile>

#include <Model/Model.h>

const QMap<XmiWriter::XMI_TYPE, QString> XmiWriter::sXmiStartTags = {
    {XmiWriter::XMI_TYPE::FULL_DUMP, "Model"},
    {XmiWriter::XMI_TYPE::EXPORT, "Export"}
};



XmiWriter::XmiWriter(Model *model, QFile *file)
    : _model(model),
      _xmlStreamWriter(new QXmlStreamWriter(file)),
      _ownStreamWriter(true)
{
    init();
}

XmiWriter::XmiWriter(Model *model, QXmlStreamWriter *xmlWriter)
    : _model(model),
      _xmlStreamWriter(xmlWriter),
      _ownStreamWriter(false)
{}

XmiWriter::~XmiWriter()
{
    if (_ownStreamWriter)
        delete _xmlStreamWriter;
}

void XmiWriter::init()
{
    _xmlStreamWriter->setAutoFormatting(true);
}

void XmiWriter::writeStartTag(const QString &applicationName, XMI_TYPE xmiType)
{
    // Append tag XML
    _xmlStreamWriter->writeStartDocument();

    // Create tag Cosi7:Model
    QString dataModel = _model->getToolName();
    _xmlStreamWriter->writeStartElement(dataModel+":"+sXmiStartTags.value(xmiType));
    _xmlStreamWriter->writeAttribute("xmlns:"+dataModel, "http://"+dataModel);
    _xmlStreamWriter->writeAttribute("xmlns:xmi", "http://www.omg.org/XMI");
    _xmlStreamWriter->writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    _xmlStreamWriter->writeAttribute("ToolName", applicationName);
    _xmlStreamWriter->writeAttribute("ExportVersion", _model->getExportVersion());
    _xmlStreamWriter->writeAttribute("Date", QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"));
    _xmlStreamWriter->writeAttribute("ExportDescription", _model->getExportDescription());
    _xmlStreamWriter->writeAttribute("ModelId", QString::number(_model->getId()));
}

void XmiWriter::writeEndDocument()
{
    _xmlStreamWriter->writeEndDocument();
}

void XmiWriter::write(MObjectType *mObjectType)
{
    QMap<QString, MObject *> *mObjects = _model->_getModelObjectMap(mObjectType);
    QString tagName(mObjectType->getName());
    for (auto it = mObjects->cbegin() , itEnd = mObjects->cend(); it != itEnd ; ++it)
        it.value()->serialize(this, tagName);
}

void XmiWriter::writeStartElement(const QString &tagName)
{
    _xmlStreamWriter->writeStartElement(tagName);
}

void XmiWriter::writeEndElement()
{
    _xmlStreamWriter->writeEndElement();
}

void XmiWriter::addAttribute(const QString &name, const QString &value)
{
    if ( !(name.isEmpty() || value.isEmpty()) )
        _xmlStreamWriter->writeAttribute(name, value.trimmed());
}

void XmiWriter::addAttribute(const QString &name, bool value)
{
    if (!name.isEmpty())
    {
        if (value)
            addAttribute(name, QString("true"));
        else
            addAttribute(name, QString("false"));
    }
}

void XmiWriter::addAttribute(const QString &name, int value)
{
    if (!name.isEmpty())
    {
        QString valStr;
        if (value == Property::INT_INFINITE_NEG)
            valStr = "-∞";
        else if (value == Property::INT_INFINITE_POS)
            valStr = "+∞";
        else
            valStr = QString::number(value);

        addAttribute(name, valStr);
    }}

void XmiWriter::addAttribute(const QString &name, double value)
{
    if (!name.isEmpty())
    {
        QString valStr;
        if (value == Property::DBL_INFINITE_NEG)
            valStr = "-∞";
        else if (value == Property::DBL_INFINITE_POS)
            valStr = "+∞";
        else
            valStr = QString("%1").arg(value, 0, 'g', 13);

        addAttribute(name, valStr);
    }}

void XmiWriter::addAttribute(const QString &name, float value)
{
    if (!name.isEmpty())
    {
        QString valStr;
        if (value == Property::FLT_INFINITE_NEG)
            valStr = "-∞";
        else if (value == Property::FLT_INFINITE_POS)
            valStr = "+∞";
        else
            valStr = QString("%1").arg(value, 0, 'g', 7);

        addAttribute(name, valStr);
    }
}

void XmiWriter::addAttribute(const QString &name, const QDateTime &value)
{
    addAttribute(name, value.toString("yyyy/MM/dd hh:mm:ss"));
}

void XmiWriter::addAttribute(const QString &name, MObject *mObject)
{
    if (mObject && !name.isEmpty())
        addAttribute(name, mObject->getId());
}


void XmiWriter::addAttribute(const QString &name, const QList<MObject *> &mObjects)
{
    if ( !(mObjects.isEmpty() || name.isEmpty()) )
    {
        QString value;
        ushort nb;
        for (MObject *mObj : mObjects)
        {
            if (nb++ != 0)
                value += " ";
            value += mObj->getId();
        }
        addAttribute(name, value);
    }
}
