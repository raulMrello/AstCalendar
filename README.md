# AstCalendar

AstCalendar is an Active Object, ```arm mbed``` compatible that includes an Astronomic Calendar manager and uses a [RealTimeClock](https://github.com/raulMrello/RealTimeClock) device to backup current date/time. 

It also, uses [MQLib](https://github.com/raulMrello/MQLib) as pub-sub infrastructure to communicate with other components in a decoupled way.

It can be accessed through different topic updates, using binary data structures (blob). These struct definitions are included in file ```AstCalendarBlob.h```, so other componentes can include this file, to communicate with it.


  
## Changelog

---
### **17.01.2019**
- [x] Initial commit