/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 NBEE Embedded Systems, Miguel Angel Ajo <miguelangel@nbee.es>
 * Copyright (C) 1992-2012 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/**
 * @file module.i
 * @brief Specific BOARD extensions and templates
 */


%extend MODULE
{ 
  %pythoncode
  {
     
    def GetPads(self):		      return self.m_Pads
    def GetDrawings(self):            return self.m_Drawings
    def GetReferenceObj(self):	      return self.m_Reference
    
    #def SaveToLibrary(self,filename):
    #  return SaveModuleToLibrary(filename,self)
    
    #
    # add function, clears the thisown to avoid python from deleting
    # the object in the garbage collector
    #
    
    def Add(self,item): 
    	
    	itemC = item.Cast()
    	
    	if type(itemC) is D_PAD:
    		item.thisown=0
    		self.m_Pads.PushBack(itemC)
    	elif type(itemC) in [ TEXTE_PCB, DIMENSION, TEXTE_MODULE, DRAWSEGMENT,EDGE_MODULE]:
    		item.thisown = 0
    		self.m_Drawings.PushBack(item)
  }
  
}

%pythoncode 
{
    
def GetPluginForPath(lpath):
        return IO_MGR.PluginFind(IO_MGR.LEGACY)
	
def FootprintEnumerate(lpath):
        plug = GetPluginForPath(lpath)
        return plug.FootprintEnumerate(lpath)
	
def FootprintLoad(lpath,name):
        plug = GetPluginForPath(lpath)
        return plug.FootprintLoad(lpath,name)
	
def FootprintSave(lpath,module):
	plug = GetPluginForPath(lpath)
        return plug.FootprintSave(lpath,module)
	
def FootprintDelete(lpath,name):
        plug = GetPluginForPath(lpath)
        plug.FootprintDelete(lpath,name)
	
def FootprintLibCreate(lpath):
        plug = GetPluginForPath(lpath)
        plug.FootprintLibCreate(lpath)
	
def FootprintLibDelete(lpath):
        plug = GetPluginForPath(lpath)
        plug.FootprintLibDelete(lpath)
	
def FootprintIsWritable(lpath):
        plug = GetPluginForPath(lpath)
        plug.FootprintLibIsWritable(lpath)
  
    
}
