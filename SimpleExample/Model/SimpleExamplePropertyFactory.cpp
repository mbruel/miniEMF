#include "SimpleExamplePropertyFactory.h"
#include "SimpleExampleTypeFactory.h"
#include "allModelObjectsInclude.h"
#include "Constant.h"
#include <QDateTime>


void SimpleExamplePropertyFactory::initStatics(){

    initModelObjectProperties();

    // STEP 1.: intialize properties of classes having children


    // STEP 2.: all the others
    Person::sClassPropertyMap = MObject::initPropertyMap();
    Person::PROPERTY_sex      = _create<EnumProperty>(Person::sClassPropertyMap, "sex", QT_TRANSLATE_NOOP("Property", "Sex"));
    Person::PROPERTY_age      = _create<int, AttributeProperty>(Person::sClassPropertyMap, "age", QT_TRANSLATE_NOOP("Property", "Age"), 0);
    Person::PROPERTY_partner  = _create<Link01Property>(Person::sClassPropertyMap, Person::TYPE, Person::TYPE, "partner", QT_TRANSLATE_NOOP("Property", "Partner"), false);
    Person::PROPERTY_parents  = _create<Link0NProperty>(Person::sClassPropertyMap, Person::TYPE, Person::TYPE, "parents", QT_TRANSLATE_NOOP("Property", "Parents"), false);
    Person::PROPERTY_childs   = _create<MapLinkProperty>(Person::sClassPropertyMap, Person::TYPE, Person::TYPE, "childs", QT_TRANSLATE_NOOP("Property", "Children"), false);
    Person::PROPERTY_meetings = _create<MultiMapLinkProperty>(Person::sClassPropertyMap, Person::TYPE, Meeting::TYPE, "meetings", QT_TRANSLATE_NOOP("Property", "Meetings"), false);

    Meeting::sClassPropertyMap = MObject::initPropertyMap();
    Meeting::PROPERTY_date         = _create<QDateTime, AttributeProperty>(Meeting::sClassPropertyMap, "date", QT_TRANSLATE_NOOP("Property", "Date and Time"), QDateTime::currentDateTime());
    Meeting::PROPERTY_participants = _create<MapLinkProperty>(Meeting::sClassPropertyMap, Meeting::TYPE, Person::TYPE, "participants", QT_TRANSLATE_NOOP("Property", "Participants"), false);
}


void SimpleExamplePropertyFactory::defineEcoreContainmentProperties()
{
    // Nothing yet (people don't own each other..."

}

void SimpleExamplePropertyFactory::defineModelObjectTypeContainerProperties()
{
    // Nothing yet

}

void SimpleExamplePropertyFactory::defineMapPropertiesKey()
{
    Person::PROPERTY_childs->setKey(Person::PROPERTY_age);
    Person::PROPERTY_meetings->setKey(Meeting::PROPERTY_date);
}

void SimpleExamplePropertyFactory::linkAllReverseProperties()
{
    linkReverseProperties(Person::PROPERTY_partner,  Person::PROPERTY_partner);
    linkReverseProperties(Person::PROPERTY_childs,   Person::PROPERTY_parents);
    linkReverseProperties(Person::PROPERTY_meetings, Meeting::PROPERTY_participants);
}

void SimpleExamplePropertyFactory::defineEnumPropertyValues()
{    
    QMap<int,QString> enums;

    // /miniExample/SEX
    enums.clear();
    enums.insert(static_cast<int>(Enum::SEX::Male),   Constant::C_Male);
    enums.insert(static_cast<int>(Enum::SEX::Female), Constant::C_Female);
    Person::PROPERTY_sex->setEnumValues(enums);
}
