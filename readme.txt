Steps to setup a new machine

* Install Python 2.7.*, put it on the path, test "python -V" should give something like Python 2.7.2
* Download scons, extract. execute setup.py with install param, "python setup.py install"
* Install git, Download mysysgit for windows.
* If you dont have keys use ssh-keygen -t rsa -C "email@email.com" to generate one.
* If key is not the defualt location, ssh-add key.pub to add it.
* git clone git@knowledgequest.unfuddle.com:knowledgequest/kraft.git kraft
* git config user.name "First Last"
* git config user.email "email@mymail.com" 
* To Create a tracking branch:  "git branch localname --track origin/remotebranch", local and remote can be the same names