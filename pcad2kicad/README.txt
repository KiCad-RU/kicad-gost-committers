PCadToKiCad, PCad ASCII to KiCad conversion utility

Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
Copyright (C) 2007, 2008 Alexander Lunev <alexanderlunev@mail.ru>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
USA

PCadToKiCad software uses triangulator of the Triangle project, version 1.6.
Copyright 1993, 1995, 1997, 1998, 2002, 2005 Jonathan Richard Shewchuk <jrs@cs.berkeley.edu>
http://www.cs.cmu.edu/~quake/triangle.html

***********************************************************************************************************************
*PCad ASCII PCB to KiCad BRD(pcb)
*PCad ASCII PCBLibrary to KiCad MOD(pcblib)
*PCad ASCII SCH to KiCad SCH (schematic)
*PCad ASCII SCHLibrary to KiCad LIB and DCM (schlib files)

This utility converts ASCII data file from PCad 2000 format to KiCad format.
Actual suported files are
  PCad 2000 ASCII
       PCB Libraries  ( *.lia ) --->  converted to KiCad *.mod file
       PCB Board      ( *.pcb ) --->  converted to KiCad *.brd file
       SCH Libraries  ( *.lia ) --->  converted to KiCad *.lib and *.dcm files
       SCH Schematics ( *.sch ) --->  converted to KiCad *.KiCad.sch *.KiCad.lib and *.KiCad.dcm files

***********************************************************************************************************************

Some comments to current version :

*There is no restriction for sizes, but also not checking, so for really big files it can raise out of memory error,
or somethink like this, i hope, your file is small enough to be processed.....

*Also, there is only standard double sided board supported .....

*There is not full support for some specials, which you can do in source cad, but is not easy to transform....,
        but standard boards are converted well.

*There is issue with text size and positioning. The texts are placed, but the position is litlebit
       different from original in some cases....

*When you are converting SCHEMATICS FILE, be shure you have placed new converted libraries *.KiCad.lib and
       *.KiCad.dcm files into KiCad library directory.
       You have to add those new libraries into your KiCad library list before you open new converted
       KiCad schematic file . This is critically.

*There is no support for hiererchical schematic sheets

*There is no support for HATCH45POUR type of polygon filling.
*PCAD cutouts influence zone filling only and are not converted to corresponding
        cutouts objects of kicad because kicad does not support such objects.
*Resulted kicad *.brd files are too big because kicad file format contains not only polygons
        outlines but also all filling lines in comparison with PCAD that contains not filling
        lines but islands outlines composing whole filled polygon.


******************************************************************************************************************
Version list
0.9.05 released from 24.02.2008
 - supported new pcbnew file format namely zone outline description
 - pad net names of modules are fixed

0.9.04
 - Added support for RndRect PCAD pad shape type, this shape is converting to Rect KiCad pad type
 - Some RTM errors, rised by different schlib input file format are fixed....
 - Corrected part names with '(' or ')' chars
 - Name of PCB LIB module components is now with "Type" named font, it meens, size is not zero, if this font has assign size.

0.9.03
 - Added SCHEMATICS LIBRARY CONVERSION
 - Added SCHEMATICS SIMPLE FILE CONVERSION
 - Worksheet size is adujusted
 - PCB Polygons are implemented

0.9.02
 - stack size is enhanced and biger size files are converted now
 - multiple component outlines in pcblib used in design - select correct implementation
 - copper layer components added
 - 45 deg. components rotation fixed
 - polygon pads are converted to rect pads
 - fixed SMD pin layers mapping
 - added flexible - by input file - layers matching
 - components ref texts - added mirror option
 - added support for circles drawings in newer file format representation
 - improved text positioning
 - fixed bug with mixing mm and mil units - now there is default units readed from input file and if there is not exact information, this default is used
 - fixed bug with Edge layer - in documentation there si bug and edge layer is numbered 27... not 28. I found it and fixed.

0.9.01
*The first version. Support only PCad2000 file format


******************************************************************************************************************
TWO TIPS AT THE END :

1, If you are using other CAD system,
       which support EXPORT to file format PCad, You can use this utility and to convert to KiCAd !!!!!
       Protel for example.....

2, Check XML file produced by this software as semiproduct !!!!!

********************************************************************************************************************

Please e-mail to authors and share  your experience with this convertor utility, bugs, or any comments.

Please let us know if you use this utility,
     - We can inform you by e-mail later in case of utility update.
     - This is really good feedback for us if someone is using this utility....

                 Developers.




