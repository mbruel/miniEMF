# miniEMF

The mini EMF framework is a C++11/QT transposition of an Ecore Model.<br />
Instead of using the standard Class attributes we’re using a Property management system.<br />
The idea doing this is to automatically update the e-opposites.<br />
It's kind of replacing the observer pattern.<br />


Here are the main advantages of using this framework:
- Updating a link property of an MObject (ModelObject) automatically updates the opposite MObjects (if the property has a reverse property (e-opposite)).
- Removing an MObject from the Model will automatically remove it from all the other MObjects that use it
- Every property is updated using one single method (MObject::updateValue) which makes it pretty easy to integrate a Model within an Undo Framework
- Loading and Dumping a model in XML is easy (xmi format)
- Possibility to do shallow copy or deep copy of a subset of the Model



### How to use it
- Create all your business Objects (ex: Person, Meeting...) that derives from MObject
- Declare all their Properties that derives from Property (DoubleProperty, Link11Property, MapLinkProperty...)
- in a TypeFactory deriving from MObjectTypeFactory, istanciate an MObjectType for each of your business Objects and define their Ecore inheritance.
- in a PropertyFactory deriving from PropertyFactory, instanciate all the Properties of your objects, linking them with their e-opposites


To be continued...<br />
In the meantime you can check the SimpleExample
