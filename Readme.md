# nBodySim v1.3 October 31st 2004
## By Lewis Sellers (aka Min)
## Intrafoundation Software
### http://www.intrafoundation.com/products/nBodySim

nBodySim is a port of software I wrote for my TI-85/86 graphing calculator that models a set of N spheres under Newtonian gravitation with collision responses.

![nBodySim](https://github.com/lasellers/nBodySim/blob/master/screenshot1.png)

This software was written in C++ using MSVC++ 6 Professional SP5 + MS Platform SDK.

There is very little in the way of documentation for this software at the moment. Maybe latter one snowy day I’ll complete the collision functions and properly document how the program works and the mathematics behind it.

If YOU are a Physics Major/Grad and DO understand the mathematics behind rigid body collisions, feel free to email me. It would be nice to have a complete, fully documented, functional software package suitable for high school/college level gravitational demonstrations. (See Version history for further details.)

Once this is working properly I’d like to move on to a version that handles elastic collisions of more complex objects (liquids for example) with friction, etc.

Thanks,
Lewis Sellers
--min


# Version History
v1.3 Feb 2017. Refresh for Visual Studio 2015 and GitHub.

v1.2 October 31, 2004.
*Added help file and recompiled.
*The collision functions are NOT working correctly at the moment. I suspect it’s because of catastrophic interpenetration issues that occur when the velocities of objects take them too deep within an other object, causing the math to produce a very high false acceleration in the inverse direction. A more advanced system that can do fractional or partial time computations needs to be added to deal with this. In other words, the problems go away if the time frame is set low enough – though this means the interactions happen at a somewhat sluggish rate.
*Added new graphics for the about dialog. My cat Sarah is very prickly (she bites a lot) – reminding me of a cactus, so I made a little garden area that hints of her.
*Added collision sound effects.
*Aside from someday fixing all collision response math, I plan on creating a scenario load/save function which will allow you to load complex nbody scenarios from a text file as well as save the current view and nbody position/momentum data to a text file.

v1.1 April 2003. (undocumented) Massive rewrite to program. Collisions are better, but still don’t work.

v1.0 Early 2003. (undocumented)

v0 ? (undocumented) This software was originally written sometime in early 2003 (I don't remember exactly when) in C++ using MSVC++ 6 Professional SP5 + MS Platform SDK.

