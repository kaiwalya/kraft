Steps to setup a new machine

* Install Python 2.7.*, put it on the path, test "python -V" should give something like Python 2.7.2
* Download scons, extract. execute setup.py with install param, "python setup.py install"
* Install git, Download mysysgit for windows.
* If you dont have keys use ssh-keygen -t rsa -C "email@email.com" to generate one.
* If key is not the defualt location, ssh-add key.pub to add it.
* git clone git@knowledgequest.unfuddle.com:knowledgequest/kraft.git kraft
* git config user.name "First Last"
* git config user.email "email@mymail.com" 
* To Create a tracking branch:  "git branch localname --track origin/remotebranch", local and remote can be the same names========================================================================
    MAKEFILE PROJECT : kraft Project Overview
========================================================================

AppWizard has created this kraft project for you.  

This file contains a summary of what you will find in each of the files that
make up your kraft project.


kraft.vcxproj
    This is the main project file for VC++ projects generated using an Application Wizard. 
    It contains information about the version of Visual C++ that generated the file, and 
    information about the platforms, configurations, and project features selected with the
    Application Wizard.

kraft.vcxproj.filters
    This is the filters file for VC++ projects generated using an Application Wizard. 
    It contains information about the association between the files in your project 
    and the filters. This association is used in the IDE to show grouping of files with
    similar extensions under a specific node (for e.g. ".cpp" files are associated with the
    "Source Files" filter).

This project allows you to build/clean/rebuild from within Visual Studio by calling the commands you have input 
in the wizard. The build command can be nmake or any other tool you use.

This project does not contain any files, so there are none displayed in Solution Explorer.

/////////////////////////////////////////////////////////////////////////////
