#include "Meeting.h"
#include "Model/Property.h"
#include <QString>

// Type
ElementType *Meeting::TYPE = Q_NULLPTR;

// Properties
QMap<QString, Property*> *Meeting::sClassPropertyMap = Q_NULLPTR;
DateTimeProperty         *Meeting::PROPERTY_date         = Q_NULLPTR;
MapLinkProperty          *Meeting::PROPERTY_participants = Q_NULLPTR;

Meeting::~Meeting()
{
    if (isInModel())
        hideFromLinkedElements();
}

QVariant Meeting::getPropertyMapKey(Property *mapProperty)
{
    Q_UNUSED(mapProperty)
    return getDate();
}

QDateTime Meeting::getDate() { return PROPERTY_date->getValue(this); }
ElemMap *Meeting::getParticipants() { return PROPERTY_participants->getValues(this); }

void Meeting::setDate(QDateTime value) { PROPERTY_date->setValue(this, value); }
void Meeting::setParticipants(ElemList &values){ PROPERTY_participants->updateValue(this, values); }

QString Meeting::getInfo()
{
    QString info("Meeting "+ getName() + " (id=" + getId() + ")");
    info += " at " + getDate().toString("ddd MMMM d hh:mm:ss.zzz");
    info += " with: ";
    ushort i = 0;
    ElemMap *participants = getParticipants();
    for (auto it = participants->cbegin(); it != participants->cend(); ++it)
    {
        if (++i !=0 )
            info += " and ";
        info += it.value()->getName();
    }
    return info;
}
