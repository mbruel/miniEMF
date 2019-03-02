#include <QCoreApplication>
#include <QTranslator>

#include "Model/Model.h"
#include "Model/Constant.h"
#include "Model/SimpleExampleTypeFactory.h"
#include "Model/SimpleExamplePropertyFactory.h"
#include "Model/allModelObjectsInclude.h"
#include <QDebug>
#include <QtGlobal> // Q_ASSERT

#include <Service/XMIService.h>


void initStatics(){

    SimpleExampleTypeFactory::getInstance()->initStatics();
    SimpleExamplePropertyFactory::getInstance()->initStatics();
}

void initMetaModel(){
    SimpleExampleTypeFactory::getInstance()->initModelObjectTypes();
    SimpleExamplePropertyFactory::getInstance()->initProperties();
}



Person *createPerson(Model *model, const char *name, uint age, const char *sex)
{
    QMap<Property *, QVariant> properties = {
        {Person::PROPERTY_NAME, name},
        {Person::PROPERTY_age, age}
    };
    Person *p = static_cast<Person*>(Person::TYPE->createModelObject(model->getId(), properties));
    p->setSex(sex);

    model->add(p);
    return p;
}

Meeting *createMeeting(Model *model, const char *name)
{
    QMap<Property *, QVariant> properties = {
        {Meeting::PROPERTY_NAME, name},
        {Meeting::PROPERTY_date, QDateTime::currentDateTime()}
    };
    Meeting *m = static_cast<Meeting*>(Meeting::TYPE->createModelObject(model->getId(), properties));

    model->add(m);
    return m;
}

void printAllPersons(const QList<Person*> &persons)
{
    qDebug() << "Dump of all " << Person::TYPE->getLabel();
    for (Person *const p : persons)
        qDebug() << "\t-" << p->getInfo();
}


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // I.1.: if we wish to use translations
    QTranslator translator;
    QString lang(":/languages/example_fr");
    bool langOk = translator.load(lang);
    bool langLoaded = app.installTranslator(&translator);
    qDebug () << "Loading " << lang << " return : " << langOk << " , loaded: " << langLoaded;


    // I.2.: First thing to do is to initialize the Statics and the Links between objects
    initStatics();
    initMetaModel();
    // I.3.: create a Model object
    Model model(SimpleExampleTypeFactory::getInstance(),
                "miniEmfExample", "v1.0", "Simple Example MiniEMF", 42, "");


    // I.4.: create some objects (Persons and add them into the model)
    Person *mat   = createPerson(&model, "Matthieu", 35, Constant::C_Male);
    Person *dad   = createPerson(&model, "Josian",   67, Constant::C_Male);
    Person *mum   = createPerson(&model, "Annick",   65, Constant::C_Female);
    Person *alice = createPerson(&model, "Alice",    34, Constant::C_Female);
    Q_ASSERT(model.getModelObjects(Person::TYPE).size() == 4);

    // I.5.: test the setters (the e-opposite are done automatically)
    mat->setPartner(alice);
    Q_ASSERT(mat->getPartner() == alice);
    Q_ASSERT(alice->getPartner() == mat);

    mat->setParents({mum, dad});
    Q_ASSERT(mat->getParents()->size() == 2);
    Q_ASSERT(mum->getChilds()->size() == 1);
    Q_ASSERT(*mum->getChilds()->begin() == mat);
    Q_ASSERT(dad->getChilds()->size() == 1);
    Q_ASSERT(*dad->getChilds()->begin() == mat);

    Person *bebe = createPerson(&model, "Bebe", 11*7, Constant::C_Female);
    Q_ASSERT(model.getModelObjects(Person::TYPE).size() == 5);
    bebe->setParents({mum, mat});
    Q_ASSERT(mum->getChilds()->size() == 2);
    Q_ASSERT(mat->getChilds()->size() == 1);


    Person *philippe = createPerson(&model, "Philippe", 62, Constant::C_Male);
    Person *momo     = createPerson(&model, "Monique",  61, Constant::C_Female);
    Person *lucie    = createPerson(&model, "Lucie",    32, Constant::C_Female);
    Q_ASSERT(model.getModelObjects(Person::TYPE).size() == 8);

    momo->setPartner(philippe);
    Q_ASSERT(momo->getPartner() == philippe);
    Q_ASSERT(philippe->getPartner() == momo);

    alice->setParents({momo, philippe});
    lucie->setParents({momo, philippe});
    Q_ASSERT(momo->getChilds()->size() == 2);
    Q_ASSERT(philippe->getChilds()->size() == 2);


    Person *juliou = createPerson(&model, "Juliette", 7, Constant::C_Female);
    Q_ASSERT(model.getModelObjects(Person::TYPE).size() == 9);
    juliou->setParents({lucie});
    Q_ASSERT(lucie->getChilds()->size() == 1);


    Person *unknown = createPerson(&model, "unknown", 32, Constant::C_Female);
    Q_ASSERT(model.getModelObjects(Person::TYPE).size() == 10);



    // I.6.: Create a Meeting
    Meeting *meeting1 = createMeeting(&model, "Meeting Langlois");
    MObjectList participants = {mat, alice, lucie, juliou};
    meeting1->setParticipants(participants);
//    qDebug() << meeting1->getInfo();
    Q_ASSERT(meeting1->getParticipants()->size() == 4);
    for (MObject *p : participants)
        Q_ASSERT(static_cast<Person*>(p)->getMeetings()->size() == 1);

    Meeting *meeting2 = createMeeting(&model, "Meeting Mum");
    participants = {mat, mum, bebe};
    meeting2->setParticipants(participants);
//    qDebug() << meeting2->getInfo();
    Q_ASSERT(meeting2->getParticipants()->size() == 3);
    Q_ASSERT(mat->getMeetings()->size() == 2);

    printAllPersons({mat, alice, dad, mum, bebe, lucie, philippe, momo, juliou});

    model.dumpModelObjectTypeMap();






    // II.1: Test Write XMI
    QString xmiOutput = "/tmp/miniEMF_SimpleExample.xml";
    XMIService *xmiService = XMIService::getInstance();
    xmiService->writeXMI(&model, xmiOutput, "miniEmf");



    // II.2: Test Load XMI
    qDebug() << "\n Reload model from xmi saved before";
    Model model2(SimpleExampleTypeFactory::getInstance(),
                 "miniEmfExample", "v1.0", "Simple Example MiniEMF", 43, "");
    QTime loadingTime;
    loadingTime.start();
    if (xmiService->initImportXMI(xmiOutput))
        xmiService->loadXMI(&model2);
    qDebug("\n#### Loading xmi done in: %d ms\n", loadingTime.elapsed());
    model2.dumpModelObjectTypeMap();
    xmiService->writeXMI(&model2, xmiOutput+".copy", "miniEmf");
    Q_ASSERT(model2 == model);


    // II.3: Test export 1 Element
    // the "unknown" Person should not be in the export
    xmiService->exportXMI(mat, &model, xmiOutput+".mat", "miniEmf");
    xmiService->exportXMI(unknown, &model, xmiOutput+".unknown", "miniEmf");


    // II.4: Test cloning subset
    Model *model3 = model2.cloneSubset({mat});
    xmiService->writeXMI(model3, xmiOutput+".mat.clone", "miniEmf");
    delete model3;



    model.remove(meeting2);
    qDebug() << "\n Meeting2 has been removed from the model (kind of deleted except we could Undo ;))";
    Q_ASSERT(mat->getMeetings()->size() == 1);
    Q_ASSERT(mum->getMeetings()->size() == 0);
    Q_ASSERT(bebe->getMeetings()->size() == 0);
    printAllPersons({mat, mum, bebe});

    qDebug() << "\n Juliette has been removed from the model (kind of deleted except we could Undo ;))";
    model.remove(juliou);
    Q_ASSERT(meeting1->getParticipants()->size() == 3);
    qDebug() << meeting1->getInfo();

    model.dumpModelObjectTypeMap();


    qDebug() << "\n Clear model";
    model.clearModel();
    model.dumpModelObjectTypeMap();

    model2.clearModel();

    return 0;
//    return app.exec();
}
