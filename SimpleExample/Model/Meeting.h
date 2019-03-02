#ifndef MEETING_H
#define MEETING_H

#include "Model/MObject.h"
#include <QDateTime>

class Meeting : public MObject
{
public:
    Meeting();
    ~Meeting();

    // MObject TYPE
    static MObjectType *TYPE;
    MObjectType *getModelObjectType() const override { return TYPE; }
    static MObject *createModelObject() {return new Meeting();}


    // PROPERTIES
    static QMap<QString, Property*> *sClassPropertyMap;
    static DateTimeProperty         *PROPERTY_date;
    static MapLinkProperty          *PROPERTY_participants;


    QVariant getPropertyMapKey(Property *mapProperty) override;


    // Getters
    QDateTime getDate();
    MObjectMap *getParticipants();


    // Setters
    void setDate(QDateTime value);
    void setParticipants(const MObjectList &values);

    // Others
    QString getInfo();
};


#endif // MEETING_H
