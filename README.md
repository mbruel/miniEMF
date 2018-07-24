# miniEMF

The Ecore Model is completely transposed in C++ using the mini EMF framework. Instead of using the standard
attributes of the Objects we’re using a Property management system that has been implemented. This way an
Element of the Model, will hold a Map of Properties with their corresponding values.

Here are the main advantages of using this framework:
    - Updating a reference property of an object automatically update the opposite objects if the property has a
reverse property (e-opposite).
    - Every property is can be updated using one single method which makes it easy to use an Undo
Framework
    - Loading and Dumping a model in XML is easy (xmi format)
    - Possibility to do shallow copy or deep copy of a subset of the Model

An ElementType class is used as a meta-object in order to keep the Elements sorted by by type of objects but
also to save the Ecore inheritance of the objects.

Each Ecore class must have its own C++ class inheriting from Element where all the Properties are defined as
statics.

// TO_FINISH
