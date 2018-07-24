#include <QCoreApplication>
#include <QTranslator>

#include "Model/Model.h"
#include "Model/example/ElementTypeFactoryMiniEMF.h"
#include "Model/example/PropertyFactoryMiniEMF.h"
#include "Model/example/allElementsInclude.h"
#include <QDebug>
#include <QtGlobal> // Q_ASSERT

#include <Service/XMIService.h>


void initStatics(){

    ElementTypeFactoryMiniEMF::getInstance()->initStatics();
    PropertyFactoryMiniEMF::getInstance()->initStatics();
}

void initMetaModel(){
    ElementTypeFactoryMiniEMF::getInstance()->initElementTypes();
    PropertyFactoryMiniEMF::getInstance()->initProperties();
}



Person *createPerson(Model *model, const char *name, uint age, const char *sex)
{
    Person *p = new Person();
    p->setName(name);
    p->setAge(age);
    p->setSex(sex);

    model->addNewElementWithIdCreation(p);
    return p;
}

Meeting *createMeeting(Model *model, const char *name)
{
    Meeting *m = new Meeting();
    m->setName(name);
    QDateTime time = QDateTime::currentDateTime();
    m->setDate(time);

    model->addNewElementWithIdCreation(m);
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

    QTranslator translator;
    QString lang("example_fr");
    bool langOk = translator.load(lang);
    bool langLoaded = app.installTranslator(&translator);
    qDebug () << "Loading " << lang << " return : " << langOk << " , loaded: " << langLoaded;



    initStatics();
    initMetaModel();


    QString xmiOutput = "/home/bruel/Downloads/miniEmf.xml";
    XMIService *xmiService = XMIService::getInstance();

    Model model(ElementTypeFactoryMiniEMF::getInstance(),
                "miniEmfExample", "v0.1", "Simple Example MiniEMF", "42", "");

    Person *mat   = createPerson(&model, "Matthieu", 35, "Male");
    Person *dad   = createPerson(&model, "Josian", 67, "Male");
    Person *mum   = createPerson(&model, "Annick", 65, "Female");
    Person *alice = createPerson(&model, "Alice", 34, "Female");

    mat->setPartner(alice);
    QSet<Element*> parents = {mum, dad};
    mat->setParents(&parents);
Q_ASSERT(parents.size() == 0); // Should be 0 as we do a swap for efficiency


    Person *bebe = createPerson(&model, "Bebe", 11*7, "Female");
    parents = {mum, mat};
    bebe->setParents(&parents);
Q_ASSERT(parents.size() == 0);

    Person *philippe = createPerson(&model, "Philippe", 62, "Male");
    Person *momo = createPerson(&model, "Monique", 62, "Female");
    Person *lucie = createPerson(&model, "Lucie", 32, "Female");
    momo->setPartner(philippe);    
    parents = {momo, philippe};    
    alice->setParents(&parents);
Q_ASSERT(parents.size() == 0);
    parents = {momo, philippe};
    lucie->setParents(&parents);
Q_ASSERT(parents.size() == 0);

    Person *juliou = createPerson(&model, "Juliette", 7, "Female");
    parents = {lucie};
    juliou->setParents(&parents);
Q_ASSERT(parents.size() == 0);

    Person *unknown = createPerson(&model, "unknown", 32, "Female");



    Meeting *meeting1 = createMeeting(&model, "Meeting Langlois");
    QList<Element*> participants = {mat, alice, lucie, juliou};
    meeting1->setParticipants(participants);
    qDebug() << meeting1->getInfo();
Q_ASSERT(meeting1->getParticipants()->size() == 4);
Q_ASSERT(mat->getMeetings()->size() == 1);

    Meeting *meeting2 = createMeeting(&model, "Meeting Mum");
    participants = {mat, mum, bebe};
    meeting2->setParticipants(participants);
    qDebug() << meeting2->getInfo();
Q_ASSERT(meeting2->getParticipants()->size() == 3);
Q_ASSERT(mat->getMeetings()->size() == 2);

    printAllPersons({mat, alice, dad, mum, bebe, lucie, philippe, momo, juliou});

    model.dumpElementTypeMap();




    // Test Write XMI
    xmiService->writeXMI(&model, xmiOutput, "miniEmf");
    Model model2(ElementTypeFactoryMiniEMF::getInstance(),
                 "miniEmfExample", "v0.1", "Simple Example MiniEMF", "42", "");


    // Test Load XMI
    qDebug() << "\n Reload model from xmi saved before";
    QTime loadingTime;
    loadingTime.start();
    if (xmiService->initImportXMI(xmiOutput))
        xmiService->loadXMI(&model2);
    qDebug("\n#### Loading xmi done in: %d ms\n", loadingTime.elapsed());
    model2.dumpElementTypeMap();
    xmiService->writeXMI(&model2, xmiOutput+".copy", "miniEmf");
Q_ASSERT(model2 == model);


    // Test export 1 Element
    // the "unknown" Person should not be in the export
    xmiService->exportXMI(mat, &model, xmiOutput+".mat", "miniEmf");
    xmiService->exportXMI(unknown, &model, xmiOutput+".unknown", "miniEmf");


    // Test cloning subset
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

    model.dumpElementTypeMap();


    qDebug() << "\n Clear model";
    model.clearModel();
    model.dumpElementTypeMap();

    model2.clearModel();


    return app.exec();
}
