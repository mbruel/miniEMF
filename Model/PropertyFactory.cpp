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

#include "PropertyFactory.h"

void PropertyFactory::initModelObjectProperties()
{
    MObject::PROPERTY_NAME        = new StringProperty("name", "Name", "");
//    MObject::PROPERTY_DESCRIPTION = new StringProperty("description", "Description", "");
}

void PropertyFactory::initProperties()
{
    // define the EcoreContainment properties (Ecore Model)
    defineEcoreContainmentProperties();

    // make the links between properties (e-opposite)
    linkAllReverseProperties();

    // set the key of MapLinkProperties
    defineMapPropertiesKey();

    // define the Enums
    defineEnumPropertyValues();


    defineModelObjectTypeContainerProperties();
}

void PropertyFactory::linkReverseProperties(LinkProperty * const linkProperty, LinkProperty * const reverseLinkProperty)
{
    linkProperty->setReverseLinkProperty(reverseLinkProperty);
    reverseLinkProperty->setReverseLinkProperty(linkProperty);
}
