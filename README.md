PoissonEditing
==============

See complete algorithm documentation in Documentation/PoissonEditing.pdf


Obtaining the Source Code
-------------------------
This repository depends on submodules, and sub-submodules. You must clone it with:

git clone --recursive

OR

If you have already cloned the repository with 'git clone', you must also:

git submodule update --init

Dependencies
------------
- ITK >= 4

NOTE: If you get errors like:
vxl/core/vnl/vnl_numeric_traits.h:366:29: error: ‘constexpr’ needed for in-class initialization of static data member ‘zero’ of non-integral type

It means that you have not built ITK with c++11 eneabled. To do this, you must add -std=gnu++11 to CMAKE_CXX_FLAGS when you configure ITK:

ccmake ~/src/ITK -DCMAKE_CXX_FLAGS=-std=gnu++11

NOTE: you cannot configure (ccmake) and THEN set CMAKE_CXX_FLAGS - you MUST include the gnu++11 in the ccmake command the very first time it is run.

You can tell this project's CMake to use a local ITK build with:
cmake . -DITK_DIR=/home/doriad/build/ITK

- Boost 1.51
You can tell this project's CMake to use a local boost build with:
cmake . -DBOOST_ROOT=/home/doriad/build/boost_1_51

- Eigen 3.2.1
Requires an Eigen version that contains SimplicialLDLT (known to work with Eigen 3.2.4).
- You can tell this project's CMake to use a local Eigen build with:
cmake . -DEIGEN3_INCLUDE_DIR=/home/doriad/src/eigen-3.2.1/

Building
--------
This repository does not depend on any external libraries. The only caveat is that it depends on c++0x/11 parts of the c++ language used in the Helpers submodule.
For Linux, this means it must be built with the flag gnu++0x. For Windows (Visual Studio 2010), nothing special must be done.


