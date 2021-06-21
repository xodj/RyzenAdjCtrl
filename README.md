# RyzenAdjCtrl
GUI for RyzenAdj.

- Runing as separeted service from gui (Windows only)
- RyzenAdj as library (faster then cli)
- Info widget (you can realtime control your states)
- Avaliable swithing presets by Effective Power Mode (Windows only)
- Avaliable swithing presets by AC Power Mode
- Agent with notifications
- For ASUS (ROG or TUF) users have "Armory Crate" profile switch (Windows only)

# Usage:
> Service started automaticaly after GUI started, or run it as task:
> 
> To run service use "RyzenAdjCtrl.exe startup" arguments.
> 
> To stop service use "RyzenAdjCtrl.exe exit" arguments.
> 
> SERVICE MUST RUN WITH ADMINISTRATOR PRIVILEGES

# Requirements  (Windows):
- Windows 10 Fall Creators Update - 1709 16299.64 or later
- [Microsoft Visual C++ 2015-2019 Redistributable (x64) - 14.28.29914](https://github.com/xodj/RyzenAdjCtrl/releases/download/0.1.0.41/VC_redist.x64.exe)

# Requirements (Linux):
- Qt 5.15 or later
- [ryzen_smu](https://github.com/leogx9r/ryzen_smu) sources
- [RyzenAdj](https://github.com/FlyGoat/RyzenAdj) sources
- Build tools (gcc, g++, gdb, make, cmake and kernel-headers)

# Contains:
[RyzenAdj](https://github.com/FlyGoat/RyzenAdj), [atrofac-cli](https://github.com/cronosun/atrofac) (not using after 0.3.4.492), [Qt](https://www.qt.io/download-open-source), [VS2019](https://visualstudio.microsoft.com/) (Windows only), [Inno Setup](https://github.com/jrsoftware/issrc) (Windows only), AMD Trade Mark (as program icon).

# Distributed by GPL-3 license.
