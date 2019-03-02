#ifndef PERSON_H
#define PERSON_H

#include "Model/MObject.h"
class Person : public MObject
{
public:
    Person();
    ~Person();

    // MObject TYPE
    static MObjectType *TYPE;
    MObjectType *getModelObjectType() const override { return TYPE; }
    static MObject *createModelObject() {return new Person();}


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
    QSet<MObject *> *getParents();
    MObjectMap *getChilds();
    MObjectMultiMap *getMeetings();


    // Setters
    void setAge(int value);
    void setSex(const QString &value);
    void setPartner(MObject *value);
    void setParents(const MObjectList &values);
    void setChilds(const MObjectList &values);


    // Others
    QString getInfo();
};

#endif // PERSON_H
