#include "ElementTypeFactoryMiniEMF.h"
#include "allElementsInclude.h"

QList<ElementType *> ElementTypeFactoryMiniEMF::getRootElementTypes()
{
    return {Person::TYPE, Meeting::TYPE};
}

void ElementTypeFactoryMiniEMF::initStatics(){
    if (!Element::TYPE)
        Element::TYPE = _createType(  0, "Element", QT_TRANSLATE_NOOP("ElementType", "Element"), &Element::createElement, false);

    Person::TYPE  = _createType(  1, "Person",  QT_TRANSLATE_NOOP("ElementType", "Person"),  &Person::createElement);
    Meeting::TYPE = _createType(  2, "Meeting", QT_TRANSLATE_NOOP("ElementType", "Meeting"), &Meeting::createElement);
}

void ElementTypeFactoryMiniEMF::defineDerivedElementTypesFromEcore()
{

}

