#
# This program source code file is part of KiCad, a free EDA CAD application.
#
# Copyright (C) 2013-2014 KiCad Developers, see change_log.txt for contributors.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, you may find one here:
# http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
# or you may search the http://www.gnu.org website for the version 2 license,
# or you may write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
#

# This python script wizard creates a FPC connector
# for Surface Mounted Technology


from pcbnew import *

class FPCFootprintWizard(FootprintWizardPlugin):
    def __init__(self):
        FootprintWizardPlugin.__init__(self)
        self.name = "FPC"
        self.description = "FPC (SMT connector) Footprint Wizard"
        self.parameters = {
             "Pads":
                {"*n":40,           # not internal units preceded by "*"
                 "pitch":           FromMM(0.5),
                 "width":           FromMM(0.25),
                 "height":          FromMM(1.6)},
             "Shield":
                {"shield_to_pad":   FromMM(1.6),
                 "from_top":        FromMM(1.3),
                 "width":           FromMM(1.5),
                 "height":          FromMM(2)}
        }

        self.ClearErrors()

    # build a rectangular pad
    def smdRectPad(self,module,size,pos,name):
            pad = D_PAD(module)
            pad.SetSize(size)
            pad.SetShape(PAD_SHAPE_RECT)
            pad.SetAttribute(PAD_ATTRIB_SMD)
            pad.SetLayerSet( pad.SMDMask() )
            pad.SetPos0(pos)
            pad.SetPosition(pos)
            pad.SetPadName(name)
            return pad

    # This method checks the parameters provided to wizard and set errors
    def CheckParameters(self):
        p = self.parameters
        pad_count       = p["Pads"]["*n"]
        errors = ""
        if( pad_count < 1 ):
            self.parameter_errors["Pads"]["n"]="Must be positive"
            errors +="Pads/n has wrong value, "
        p["Pads"]["n"] = int( pad_count )  # make sure it stays as int (default is float)

        pad_width       = p["Pads"]["width"]
        pad_height      = p["Pads"]["height"]
        pad_pitch       = p["Pads"]["pitch"]
        shl_width       = p["Shield"]["width"]
        shl_height      = p["Shield"]["height"]
        shl_to_pad      = p["Shield"]["shield_to_pad"]
        shl_from_top    = p["Shield"]["from_top"]

        return errors


    # build the footprint from parameters
    def BuildFootprint(self):
        self.ClearErrors()
        self.CheckParameters()

        module = MODULE(None) # create a new module
        self.module = module

        p = self.parameters
        pad_count       = int(p["Pads"]["*n"])
        pad_width       = p["Pads"]["width"]
        pad_height      = p["Pads"]["height"]
        pad_pitch       = p["Pads"]["pitch"]
        shl_width       = p["Shield"]["width"]
        shl_height      = p["Shield"]["height"]
        shl_to_pad      = p["Shield"]["shield_to_pad"]
        shl_from_top    = p["Shield"]["from_top"]

        offsetX         = pad_pitch * ( pad_count-1 ) / 2
        size_pad = wxSize(pad_width,pad_height)
        size_shld = wxSize(shl_width,shl_height)
        size_text = wxSize( FromMM( 0.8), FromMM( 0.7) )
        textposy = pad_height/2 + FromMM(1)

        module.SetReference( "FPC"+str( pad_count ) )   # give it a reference name
        module.Reference().SetPos0(wxPoint(0, textposy))
        module.Reference().SetTextPosition(module.Reference().GetPos0())
        module.Reference().SetSize( size_text )

        textposy = textposy + FromMM(1)
        module.SetValue("Val***")           # give it a default value
        module.Value().SetPos0( wxPoint(0, textposy) )
        module.Value().SetTextPosition(module.Value().GetPos0())
        module.Value().SetSize( size_text )

        fpid = FPID(self.module.GetReference())   #the name in library
        module.SetFPID( fpid )

        # create a pad array and add it to the module
        for n in range ( 0, pad_count ):
            xpos = pad_pitch*n - offsetX
            pad = self.smdRectPad(module,size_pad,wxPoint(xpos,0),str(n+1))
            module.Add(pad)


        # Mechanical shield pads: left pad and right pad
        xpos = -shl_to_pad-offsetX
        pad_s0_pos = wxPoint(xpos,shl_from_top)
        pad_s0 = self.smdRectPad(module, size_shld, pad_s0_pos, "0")
        xpos = (pad_count-1) * pad_pitch+shl_to_pad - offsetX
        pad_s1_pos = wxPoint(xpos,shl_from_top)
        pad_s1 = self.smdRectPad(module, size_shld, pad_s1_pos, "0")

        module.Add(pad_s0)
        module.Add(pad_s1)

        #add outline
        outline = EDGE_MODULE(module)
        linewidth = FromMM(0.2)
        outline.SetWidth(linewidth)
        margin = FromMM(0.2)

        # upper line
        posy = -pad_height/2 - linewidth/2 - margin
        xstart = - pad_pitch*0.5-offsetX
        xend = pad_pitch * pad_count + xstart;
        outline.SetStartEnd( wxPoint(xstart, posy), wxPoint( xend, posy) )
        outline.SetLayer(F_SilkS)               #default: not needed
        outline.SetShape(S_SEGMENT)
        module.Add(outline)

        # lower line
        outline1 = outline.Duplicate()      #copy all settings from outline
        posy = pad_height/2 + linewidth/2 + margin
        outline1.SetStartEnd(wxPoint(xstart, posy), wxPoint( xend, posy))
        module.Add(outline1)

        # around left mechanical pad (the outline around right pad is mirrored/y axix)
        outline2 = outline.Duplicate()  # vertical segment
        yend = pad_s0_pos.y + shl_height/2 + margin
        outline2.SetStartEnd(wxPoint(xstart, posy), wxPoint( xstart, yend))
        module.Add(outline2)
        outline2_d = EDGE_MODULE(module)  # right pad side
        outline2_d.Copy(outline2)
        outline2_d.SetStartEnd(wxPoint(-xstart, posy), wxPoint( -xstart, yend))
        module.Add(outline2_d)

        outline3 = outline.Duplicate()  # horizontal segment below the pad
        posy = yend
        xend = pad_s0_pos.x - (shl_width/2 + linewidth + margin*2)
        outline3.SetStartEnd(wxPoint(xstart, posy), wxPoint( xend, posy))
        module.Add(outline3)
        outline3_d = EDGE_MODULE(module)  # right pad side
        outline3_d.Copy(outline3)
        outline3_d.SetStartEnd(wxPoint(-xstart, posy), wxPoint( -xend, yend))
        module.Add(outline3_d)

        outline4 = outline.Duplicate()  # vertical segment at left of the pad
        xstart = xend
        yend = posy - (shl_height + linewidth + margin*2)
        outline4.SetStartEnd(wxPoint(xstart, posy), wxPoint( xend, yend))
        module.Add(outline4)
        outline4_d = outline.Duplicate()  # right pad side
        outline4_d.SetStartEnd(wxPoint(-xstart, posy), wxPoint( -xend, yend))
        module.Add(outline4_d)

        outline5 = outline.Duplicate()  # horizontal segment above the pad
        xstart = xend
        xend = - pad_pitch*0.5-offsetX
        posy = yend
        outline5.SetStartEnd(wxPoint(xstart, posy), wxPoint( xend, yend))
        module.Add(outline5)
        outline5_d = outline.Duplicate()  # right pad side
        outline5_d.SetStartEnd(wxPoint(-xstart, posy), wxPoint( -xend, yend))
        module.Add(outline5_d)

        outline6 = outline.Duplicate()  # vertical segment above the pad
        xstart = xend
        yend = -pad_height/2 - linewidth/2 - margin
        outline6.SetStartEnd(wxPoint(xstart, posy), wxPoint( xend, yend))
        module.Add(outline6)
        outline6_d = outline.Duplicate()  # right pad side
        outline6_d.SetStartEnd(wxPoint(-xstart, posy), wxPoint( -xend, yend))
        module.Add(outline6_d)


# create our footprint wizard
fpc_wizard = FPCFootprintWizard()

# register it into pcbnew
fpc_wizard.register()
