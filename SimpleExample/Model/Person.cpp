#include "Person.h"
#include "Model/Property.h"
#include <QString>

// Type
MObjectType *Person::TYPE = Q_NULLPTR;

// Properties
QMap<QString, Property*> *Person::sClassPropertyMap = Q_NULLPTR;
IntProperty              *Person::PROPERTY_age      = Q_NULLPTR;
EnumProperty             *Person::PROPERTY_sex      = Q_NULLPTR;
Link01Property           *Person::PROPERTY_partner  = Q_NULLPTR;
Link0NProperty           *Person::PROPERTY_parents  = Q_NULLPTR;
MapLinkProperty          *Person::PROPERTY_childs   = Q_NULLPTR;
MultiMapLinkProperty     *Person::PROPERTY_meetings = Q_NULLPTR;

Person::Person(): MObject(sClassPropertyMap) {}

Person::~Person()
{
#ifdef __HIDE_ELEMENT_ON_DESTRUCTION__
    if (isInModel())
        hideFromLinkedModelObjects();
#endif
}

QVariant Person::getPropertyMapKey(Property *mapProperty)
{
    if (mapProperty == Person::PROPERTY_childs)
        return getAge();
    else
        return getName();
}

int              Person::getAge()     { return PROPERTY_age->getValue(this); }
int              Person::getSex()     { return PROPERTY_sex->getValue(this); }
QString          Person::getSexName() { return PROPERTY_sex->getEnumValueByKey(getSex()); }
Person          *Person::getPartner() { return static_cast<Person*>(PROPERTY_partner->getValue(this)); }
QSet<MObject *> *Person::getParents() { return PROPERTY_parents->getValues(this); }
MObjectMap      *Person::getChilds()  { return PROPERTY_childs->getValues(this); }
MObjectMultiMap *Person::getMeetings(){ return PROPERTY_meetings->getValues(this); }

void Person::setAge(int value) { PROPERTY_age->setValue(this, value); }
void Person::setSex(const QString &value)
{
    int sex = PROPERTY_sex->getEnumKeyByValue(value);
    PROPERTY_sex->setValue(this, sex);
}

void Person::setPartner(MObject *value)            { PROPERTY_partner->updateValue(this, value->toVariant()); }
void Person::setParents(const MObjectList &values) { PROPERTY_parents->updateValue(this, values); }
void Person::setChilds(const MObjectList &values)  { PROPERTY_childs->updateValue(this,  values); }


QString Person::getInfo()
{
    QString info(getName());
    info += QObject::tr(" is a %1 year old %2").arg(QString::number(getAge())).arg(getSexName());

    Person *partner = getPartner();
    if (partner)
        info += ", his "+ Person::PROPERTY_partner->getLabel() +" is: " + partner->getName();
    else
        info += ", he's single";

    QSet<MObject *> *parents = getParents();
    if (parents->isEmpty())
        info += ", he has no parents...";
    else
    {
        info += ", he has " + QString::number(parents->size()) + " parents : ";
        ushort i = 0;
        for (MObject *parent : *parents)
        {
            if (i++ != 0)
                info += " and ";
            info += parent->getName();
        }
    }


    MObjectMap *children = getChilds();
    if (children->isEmpty())
        info += ", he has no children...";
    else
    {
        info += ", he has " + QString::number(children->size()) + " children : ";
        ushort i = 0;
        for (const QVariant &childAge : children->keys())
        {
            if (i++ != 0)
                info += " and ";
            info += children->value(childAge)->getName();
        }
    }

    MObjectMultiMap *meetings = getMeetings();
    if (!meetings->isEmpty()){
        info += ", he's involved in "+QString::number(meetings->size())+" meetings: ";
        ushort i = 0;

        for (auto it = meetings->cbegin() ; it != meetings->cend() ; ++it)
        {
            if (i++ != 0)
                info += " and ";
            info += it.value()->getName();
        }
    }

    info += " (id: " + getId() + ")";

    return info;
}



