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
#include "Model/Element.h"

#include <QXmlStreamWriter>
#include <QFile>

#include <Model/Model.h>

const QMap<XmiWriter::XMI_TYPE, QString> XmiWriter::sXmiStartTags = {
    {XmiWriter::XMI_TYPE::FULL_DUMP, "Model"},
    {XmiWriter::XMI_TYPE::EXPORT, "Export"}
};



XmiWriter::XmiWriter(Model *model, QFile *file)
    : _model(model), _xmlStreamWriter(new QXmlStreamWriter(file))
{
    init();
}

XmiWriter::~XmiWriter()
{
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

    // Create tag PamDataModel:Model
    QString dataModel = _model->getToolName();
    _xmlStreamWriter->writeStartElement(dataModel+":"+sXmiStartTags.value(xmiType));
    _xmlStreamWriter->writeAttribute("xmlns:"+dataModel, "http://"+dataModel);
    _xmlStreamWriter->writeAttribute("xmlns:xmi", "http://www.omg.org/XMI");
    _xmlStreamWriter->writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    _xmlStreamWriter->writeAttribute("ToolName", applicationName);
    _xmlStreamWriter->writeAttribute("ExportVersion", _model->getExportVersion());
    _xmlStreamWriter->writeAttribute("Date", QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss"));
    _xmlStreamWriter->writeAttribute("ExportDescription", _model->getExportDescription());
    _xmlStreamWriter->writeAttribute("UserId", _model->getUser());
}

void XmiWriter::writeEndDocument()
{
    _xmlStreamWriter->writeEndDocument();
}

void XmiWriter::write(ElementType *elementType)
{
    QMap<QString, Element *> *elements = _model->_getElementMap(elementType);
    QString tagName(elementType->getName());
    for (auto it = elements->cbegin() ; it != elements->cend() ; ++it)
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
            addAttribute(name, "true");
        else
            addAttribute(name, "false");
    }
}

void XmiWriter::addAttribute(const QString &name, int value)
{
    if (!name.isEmpty())
        addAttribute(name, QString::number(value));
}

void XmiWriter::addAttribute(const QString &name, double value)
{
    if (!name.isEmpty())
        addAttribute(name, QString::number(value));
}

void XmiWriter::addAttribute(const QString &name, const QDateTime &value)
{
    addAttribute(name, value.toString("yyyy/MM/dd hh:mm:ss"));
}

void XmiWriter::addAttribute(const QString &name, Element *element)
{
    if (element && !name.isEmpty())
        addAttribute(name, element->getId());
}


void XmiWriter::addAttribute(const QString &name, const QList<Element *> &elements)
{
    if ( !(elements.isEmpty() || name.isEmpty()) )
    {
        QString value;
        ushort nb;
        for (Element *elem : elements)
        {
            if (nb++ != 0)
                value += " ";
            value += elem->getId();
        }
        addAttribute(name, value);
    }
}
