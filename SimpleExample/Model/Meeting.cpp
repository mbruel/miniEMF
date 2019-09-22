#include "Meeting.h"
#include "Model/Property.h"
#include <QString>

// Type
MObjectType *Meeting::TYPE = Q_NULLPTR;

// Properties
QMap<QString, Property*> *Meeting::sClassPropertyMap     = Q_NULLPTR;
DateTimeProperty         *Meeting::PROPERTY_date         = Q_NULLPTR;
MapLinkProperty          *Meeting::PROPERTY_participants = Q_NULLPTR;

Meeting::Meeting(): MObject(sClassPropertyMap) {}

Meeting::~Meeting()
{
#ifdef __HIDE_ELEMENT_ON_DESTRUCTION__
    if (isInModel())
        hideFromLinkedModelObjects();
#endif
}

QDateTime   Meeting::getDate()         { return PROPERTY_date->getValue(this); }
MObjectMap *Meeting::getParticipants() { return PROPERTY_participants->getValues(this); }

void Meeting::setDate(QDateTime value)            { PROPERTY_date->setValue(this, value); }
void Meeting::setParticipants(MObjectList &values){ PROPERTY_participants->updateValue(this, values); }

QString Meeting::getInfo()
{
    QString info("Meeting "+ getName() + " (id=" + getId() + ")");
    info += " at " + getDate().toString("ddd MMMM d hh:mm:ss.zzz");
    info += " with: ";
    ushort i = 0;
    MObjectMap *participants = getParticipants();
    for (auto it = participants->cbegin(); it != participants->cend(); ++it)
    {
        if (++i !=0 )
            info += " and ";
        info += it.value()->getName();
    }
    return info;
}
