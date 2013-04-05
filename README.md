Pidgin XEP-0136 Message Archiving plugin
========================================

* **author**: Daniel Kraic
* **email**: danielkraic@gmail.com
* **date**: 2013-04-05
* **version**: v0.7


##Requirements

It is assumed that you use XMPP server with XEP-0136 extrension support. Ejabberd IM server should work with "mod\_archive\_odbc" enabled as well as Prosody IM server with "archive" module enabled. Plugin also supports Openfire XMPP server.


Requirements for compilation: Pidgin, GTK+, Glib, Git, gcc, make

##Install

####Linux

The full git repository is at: <https://github.com/danielkraic/Pidgin-XEP-0136-plugin>

Get it using the following command:

    $ git clone git://github.com/danielkraic/Pidgin-XEP-0136-plugin.git xep136


Run these commands:

    $ cd xep136
    $ make
    $ make install
    $ make clean

Warning: Do NOT run 'make install' command as root.

Restart Pidgin and activate XEP-0136 Plugin in the "Plugins" menu.

####Windows 

Download and run [setup.exe](https://github.com/downloads/danielkraic/Pidgin-XEP-0136-plugin/setup.exe)

Restart Pidgin and activate XEP-0136 Plugin in the "Plugins" menu.

Cross-compilation for Windows was done by following the instructions from <http://code.google.com/p/pidgin-privacy-please/wiki/HowToCrossCompileForWindows>.


##Usage

You can access history by clicking the "HISTORY" button located at the bottom of Pidgin conversation window. Then "XEP136" window should appears. Next click the "SHOW" button on the right of "XEP-136" window to see list of conversation dates on the left. Select conversation date to see its content. You can change default position of "HISTORY" button in "Configure plugin" window in "Plugins" menu. 

![history button1](http://i450.photobucket.com/albums/qq222/kzr_discofil/histbutton1.jpg)

![xep window](http://i450.photobucket.com/albums/qq222/kzr_discofil/docs_xep_main.jpg)

![history button2](http://i450.photobucket.com/albums/qq222/kzr_discofil/histbutton2.jpg)

##About


![xmpp](http://xmpp.org/images/xmpp-small.png)

[XMPP](http://xmpp.org) is the Extensible Messaging and Presence Protocol, a set of open technologies for instant messaging, presence, multi-party chat, voice and video calls, collaboration, lightweight middleware, content syndication, and generalized routing of XML data. 

[XEP-0136](http://xmpp.org/extensions/xep-0136.html) Message Archiving protocol defines mechanisms and preferences for the server-side archiving and retrieval of XMPP messages.



![pidgin](http://upload.wikimedia.org/wikipedia/commons/thumb/1/18/Pidgin.svg/48px-Pidgin.svg.png)

[Pidgin](http://pidgin.im) (formerly named Gaim) is an open-source multi-platform instant messaging client, based on a library named libpurple.



![ejabberd](http://www.process-one.net/images/ejabberd_logo.png)

[ejabberd](http://www.process-one.net/en/ejabberd/) is a distributed fault-tolerant instant messaging server. It is based on the XMPP / Jabber protocol. ejabberd is a full-featured XMPP server that implements numerous XMPP Extension Protocols.


###My bachelor thesis:

* Pidgin XEP-0136 Message Archiving plugin
* Institute of Applied Informatics, Automation and Mathematics 
* [Faculty of Materials Science and Technology](http://www.mtf.stuba.sk)
* [Slovak University of Technology in Bratislava](http://www.stuba.sk)


###Základné údaje

* **Typ práce:** Bakalárska práca
* **Názov témy:** Plugin pre podporu XEP-0136 do programu Pidgin IM
* **Fakulta:** Materiálovotechnologická fakulta so sídlom v Trnave
* **Zodpovedná katedra:** Ústav aplikovanej informatiky, automatizácie a matematiky (MTF)
* **Akademický rok:** 2011/2012
