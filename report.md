# CS205 C/ C++ Programming - Project 5: : A Class for Matrices implement by C++  

**Name:** Lv Yue  
**SID:** 11710420

***This project is hosted at <https://github.com/JustLittleFive/CppMatrix>  

**Project goal**: Implement a matrix class using C++

---------------------------------------------------

## Core code

There are only 2 files in my implementation.

```fastMul.hpp``` contains the fast multiplication of project 4 in C style. It still works efficiently in C++ projects. To support various data types, algorithms and support functions are implemented as function templates.
![捕获](捕获.PNG  "捕获")

```matrix.cpp``` is the main file of this project. In order to support multiple data types and avoid possible memory conflicts caused by union structures, templates are used to define classes and most functions.  

First, it implemented a class called ```Data``` which contains 2 elements: ```used``` and ```__T*```.

```used``` is the element that records how many times this object has been referenced, which is used when creating, copying, and deleting objects.

```__T*``` points to the memory where the actual data is stored.

The core class of the project is the ```Matrix``` class.

It contains 3 private element: ```row```, ```col``` and ```Data``` pointer ```dataptr```.

    Hint: In fact, the *Data* class should be a substructure of *Matrix*. However, considering that the class template will actually create independent and absolute classes while compiling, in order to avoid repeated creation of the same type of *Data* class, the implementation handles the two separately.

![捕获](捕获1.PNG  "捕获")  

The first public part of ```Matrix``` class are the function to access the private elements.

![捕获](捕获2.PNG  "捕获")  

Next is the constructor with various parameters.

![捕获](捕获3.PNG  "捕获")  

The destructor. The memory management of the ```Data``` class is implemented directly here.

![捕获](捕获4.PNG  "捕获")

Overloaded functions for binary operators, some are function templates for different types of coercion.

![捕获](捕获5.PNG  "捕获")

![捕获](捕获6.PNG  "捕获")

Since fast multiplication requires memory alignment, multiplication of different types of matrices is temporarily not supported.

![捕获](捕获7.PNG  "捕获")

The overloading of comparison operators is divided into two types: matrices of the same type compare elements one by one, matrices of different types directly return false.
The latter one is implemented through function templates to support multiple types.

![捕获](捕获8.PNG  "捕获")

The same is true for assignment operator overloading, which can be implemented in two ways.  
When assigning different types of matrices, take elements one by one and perform mandatory type conversion, because the memory sizes used by different data types are different.

![捕获](捕获9.PNG  "捕获")

![捕获](捕获10.PNG  "捕获")

For the same reason, matrix assignment functions do not allow assignments to matrices using arrays of different types (directly disallowed by compilation).

![捕获](捕获11.PNG  "捕获")

The rest are the matrix transpose function and the input and output friend functions.

![捕获](捕获12.PNG  "捕获")

A function template for printing matrices is defined outside the Matrix class template for testing. This function can also be a member function.

![捕获](捕获13.PNG  "捕获")

The main function is written at the end of the file for testing.

![捕获](捕获14.PNG  "捕获")

The performance of the program is as expected, and the debugger also shows that memory reuse (soft copy) has been successfully implemented.

---------------------------------------------------

About Region of Interest (ROI):  
I don't think sloppy implementation of the ```ROI``` function without more wrappers is a good idea.
Unless the output of the ```ROI``` function is set to ```void```, the user will gain the ability to directly operate of memory where data stored as a one-dimensional array. This may cause the storage structure broken when the user modifies this piece of memory, causing unexpected problems when the destructor function is called, and may even cause a ***memory leak***.

---------------------------------------------------

## Difficulties & Solutions

A member function of a class template cannot be defined as a function template outside the template. The compiler will complain ```too many template parameters in template redeclaration template```.  
My guess is that the compiler cannot know the template parameter information of the template class to which the externally defined function belongs.  
At the same time, the compiler does not allow me to directly use template parameters to assign or compare pointers through pre-judgment. The compiler will complain about```comparison of distinct pointer types```or```Distinct pointer types lacks a cast```, even though my tests have determined that no assignment or comparison will be performed on arguments of different types.  
Therefore, I have to define some member functions of class templates into two categories: member functions that only handle parameters of the same type and function templates that adapt to other types, as I mentioned above.

Still, I have a lot of unanswered questions, which are listed in the code comments.
