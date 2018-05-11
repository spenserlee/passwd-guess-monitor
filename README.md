# passwd-guess-monitor
Password guess monitor is a Linux desktop program to monitor SSH attempts into your machine. The project was broken up into two separate programs: a log file watcher daemon, and a GUI application which communicates with the daemon to update the user interface. The daemon program can be run independently from the GUI application and it will add `iptables` block rules when an attempt threshold has been reached. Both programs were written using Qt 5 and have only been compiled and tested against Qt 5.8 and 5.10 on Ubuntu 16 and Fedora 25. 

The daemon and the GUI application communicate via JSON files which hold information about the remote hosts and their attempt history.

![UI](https://github.com/spenserlee/passwd-guess-monitor/blob/master/documentation/pics/ui-screenshot.png)

![iptables rule](https://github.com/spenserlee/passwd-guess-monitor/blob/master/documentation/pics/iptables-block.png)

## Directory Contents
	├── documentation
	│   ├── pics/
	│   ├── design.pdf
	│   ├── testing.pdf
	│   └── user-manual.pdf
	├── log-monitor ---------------------- The log monitor daemon
	│   ├── .gitignore
	│   ├── ipblockmonitor.cpp
	│   ├── ipblockmonitor.h
	│   ├── logmonitor.cpp
	│   ├── logmonitor.h
	│   ├── log-monitor.pro
	│   └── main.cpp
	├── passwd-guess-monitor ------------ The GUI application
	│   ├── activitylogmonitor.cpp
	│   ├── activitylogmonitor.h
	│   ├── main.cpp
	│   ├── mainwindow.cpp
	│   ├── mainwindow.h
	│   ├── mainwindow.ui
	│   ├── passwd-guess-monitor.pro
	└── .gitignore

## Usage
To run the GUI application you must run it as a superuser. You can configure the startup log-monitor daemon settings. If the daemon is already running, it will show up in the GUI and you can stop the daemon from there. Note that currently it assumes that the daemon and GUI will be run from the same directory.

![iptables rule](https://github.com/spenserlee/passwd-guess-monitor/blob/master/documentation/pics/gui-settings.png)

To run the log-monitor daemon:
`sudo ./log-monitor <path-to-logfile> <permitted-attempts> <reset-hours> <reset-min> <block-hours> <block-min>`

For example:
`sudo ./log-monitor /var/log/auth.log 3 0 1 0 5`

Will monitor the auth.log file and allow 3 invalid login attempts before blocking that host for 5 minutes. It will reset the attempt counter after 1 minute between invalid attempts.

## Todo
 - Allow for independent running of the GUI application and daemon
 - Add checks for programs other than SSH
 - Use a real PID file in /var/run/
 - Combine applications into one Qt project or even a single program with a GUI flag
 - Package executables
