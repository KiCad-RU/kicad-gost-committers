pcad2kicadsch - PCad 200x ASCII to KiCad schematic conversion utility

This utility converts PCad 200x ASCII schematic file to KiCad format.
The supported files are:
 - SCH Libraries  ( *.lia ) converted to KiCad *.lib and *.dcm files
 - SCH Schematics ( *.sch ) converted to KiCad *.KiCad.sch *.KiCad.lib and *.KiCad.dcm files

PCB conversion can be done by means of pcad2kicad Pcbnew plugin which is built in KiCad itself.

Limitations
===========

 - Some special cases are not supported such as HATCH45POUR type of polygon filling etc. They can not be
   translated because each CAD represents objects in different ways
 - There are issues with text size and positioning. The positioning and size may be little bit
       different from original ones in some cases

Tips
====

While loading converted schematic files to KiCad make sure that you have firstly attached the converted libraries
 (*.KiCad.lib and *.KiCad.dcm files) to KiCad.


Changelog
=========

* Since 08-Aug-2012 pcad2kicad utility has been built into KiCad itself, see bzr log here:
https://code.launchpad.net/~pcad2kicad-committers/kicad/pcad2kicad

* 0.9.05 released on 24-Feb-2008
 - supported new pcbnew file format namely zone outline description
 - pad net names of modules are fixed

* 0.9.04
 - Added support for RndRect PCad pad shape type, this shape is converted to Rect KiCad pad type
 - Some RTM errors, raised by different schlib input file format are fixed....
 - Corrected part names with '(' or ')' characters
 - Name of PCB LIB module components is now with "Type" named font, it means, size is not zero, if this font has assigned size.

* 0.9.03
 - Added SCHEMATICS LIBRARY CONVERSION
 - Added SCHEMATICS SIMPLE FILE CONVERSION
 - Worksheet size is adjusted
 - PCB Polygons are implemented

* 0.9.02
 - stack size is enhanced and bigger size files are converted now
 - multiple component outlines in pcblib used in design - select correct implementation
 - copper layer components added
 - 45 deg. components rotation fixed
 - polygon pads are converted to rect pads
 - fixed SMD pin layers mapping
 - added flexible - by input file - layers matching
 - components ref texts - added mirror option
 - added support for circle drawings in newer file format representation
 - improved text positioning
 - fixed bug with mixing mm and mil units - now there are default units read from input file and if there is not exact information, this default is used
 - fixed bug with Edge layer - in documentation there is a bug and edge layer is numbered 27... not 28. I found it and fixed.

* 0.9.01
 - The first version. Supported only PCad 2000 file format
