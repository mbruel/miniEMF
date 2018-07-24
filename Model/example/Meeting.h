#ifndef MEETING_H
#define MEETING_H

#include "Model/Element.h"
#include <QDateTime>

template<typename TypeAttribute> class AttributeProperty;

class Meeting : public Element
{
public:
    Meeting(): Element(sClassPropertyMap) {}
    ~Meeting();

    // ELEMENT TYPE
    static ElementType *TYPE;
    ElementType *getElementType() const override { return TYPE; }
    static Element *createElement() {return new Meeting();}


    // PROPERTIES
    static QMap<QString, Property*> *sClassPropertyMap;
    static DateTimeProperty         *PROPERTY_date;
    static MapLinkProperty          *PROPERTY_participants;


    QVariant getPropertyMapKey(Property *mapProperty) override;


    // Getters
    QDateTime getDate();
    ElemMap *getParticipants();


    // Setters
    void setDate(QDateTime value);
    void setParticipants(ElemList &values);

    // Others
    QString getInfo();
};


#endif // MEETING_H
