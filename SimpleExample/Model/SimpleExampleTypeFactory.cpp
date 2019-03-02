#include "SimpleExampleTypeFactory.h"
#include "allModelObjectsInclude.h"

QList<MObjectType *> SimpleExampleTypeFactory::getRootModelObjectTypes()
{
    return {Person::TYPE, Meeting::TYPE};
}

void SimpleExampleTypeFactory::initStatics(){
    if (!MObject::TYPE)
        MObject::TYPE  = _createType(  0, "MObject", QT_TRANSLATE_NOOP("MObjectType", "MObject"), &MObject::createModelObject, false);

    Person::TYPE  = _createType( 1, "Person",  QT_TRANSLATE_NOOP("MObjectType", "Person"),  &Person::createModelObject);
    Meeting::TYPE = _createType( 2, "Meeting", QT_TRANSLATE_NOOP("MObjectType", "Meeting"), &Meeting::createModelObject);
}

void SimpleExampleTypeFactory::defineDerivedModelObjectTypesFromEcore()
{

}
