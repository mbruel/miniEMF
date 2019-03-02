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

#ifndef XMIWRITER_H
#define XMIWRITER_H
#include <QString>
#include <QVariant>
#include <QMultiMap>
#include <QSet>
#include <QList>
#include <QDateTime>

class MObjectType;
class MObject;
class Model;
class QXmlStreamWriter;
class QFile;

class XmiWriter
{
public:
    enum class XMI_TYPE {FULL_DUMP, EXPORT};

    XmiWriter(Model *model, QFile *file);
    XmiWriter(Model *model, QXmlStreamWriter *xmlWriter);
    virtual ~XmiWriter();

    XmiWriter(const XmiWriter &other) = delete;
    XmiWriter(const XmiWriter &&other) = delete;
    XmiWriter & operator=(const XmiWriter &other) = delete;
    XmiWriter & operator=(const XmiWriter &&other) = delete;

    void init();
    void writeStartTag(const QString &applicationName, XMI_TYPE xmiType);
    void writeEndDocument();

    void write(MObjectType *mObjectType);

    void writeStartElement(const QString &tagName);
    void writeEndElement();

    void addAttribute(const QString &name, const QString &value);
    void addAttribute(const QString &name, bool value);
    void addAttribute(const QString &name, int value);
    void addAttribute(const QString &name, double value);
    void addAttribute(const QString &name, float value);
    void addAttribute(const QString &name, const QDateTime &value);
    void addAttribute(const QString &name, MObject *mObject);
    void addAttribute(const QString &name, const QList<MObject*> &mObjects);

private:
    Model            *_model;
    QXmlStreamWriter *_xmlStreamWriter;
    bool              _ownStreamWriter;

    static const QMap<XMI_TYPE, QString> sXmiStartTags;
};

#endif // XMIWRITER_H
