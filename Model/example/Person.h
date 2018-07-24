#ifndef PERSON_H
#define PERSON_H

#include "Model/Element.h"
class Person : public Element
{
public:
    Person(): Element(sClassPropertyMap) {}
    ~Person();

    // ELEMENT TYPE
    static ElementType *TYPE;
    ElementType *getElementType() const override { return TYPE; }
    static Element *createElement() {return new Person();}


    // PROPERTIES
    static QMap<QString, Property*> *sClassPropertyMap;

    static EnumProperty             *PROPERTY_sex;
    static IntProperty              *PROPERTY_age;
    static Link01Property           *PROPERTY_partner;
    static Link0NProperty           *PROPERTY_parents;
    static MapLinkProperty          *PROPERTY_childs; //!< childs ordered by age
    static MultiMapLinkProperty     *PROPERTY_meetings; //!< ordered by date

    QVariant getPropertyMapKey(Property *mapProperty) override;

    // Getters
    int getAge();
    int getSex();
    QString getSexName();
    Person *getPartner();
    QSet<Element *> *getParents();
    ElemMap *getChilds();
    ElemMultiMap *getMeetings();


    // Setters
    void setAge(int value);
    void setSex(const QString &value);
    void setPartner(Element *value);
    void setParents(ElemSet *values);
    void setChilds(ElemMap *values);


    // Others
    QString getInfo();
};

#endif // PERSON_H
