#!/usr/bin/python

from pcbnew import *

class TouchSliderWizard(FootprintWizardPlugin):
    def __init__(self):
        FootprintWizardPlugin.__init__(self)
        self.name = "Touch Slider"
        self.description = "Capacitive Touch Slider Wizard"
        self.parameters = {
             "Pads":
                {"*steps":4, # not internal (measurement) units preceded by "*"
                 "*bands":2, 
                 "width":           FromMM(10),
                 "length":          FromMM(50),
                 "clearance":       FromMM(1)
                }
                
        }
        self.ClearErrors()
      
    # build a rectangular pad
    def smdRectPad(self,module,size,pos,name):
            pad = D_PAD(module)
            pad.SetSize(size)
            pad.SetShape(PAD_RECT)
            pad.SetAttribute(PAD_SMD)
            pad.SetLayerMask(PAD_SMD_DEFAULT_LAYERS)
            pad.SetPos0(pos)
            pad.SetPosition(pos)
            pad.SetPadName(name)
            return pad
        
        
    def smdTrianglePad(self,module,size,pos,name,up_down=1,left_right=0):
            pad = D_PAD(module)
            pad.SetSize(wxSize(size[0],size[1]))
            pad.SetShape(PAD_TRAPEZOID)
            pad.SetAttribute(PAD_CONN)
            pad.SetLayerMask(PAD_CONN_DEFAULT_LAYERS)
            pad.SetPos0(pos)
            pad.SetPosition(pos)
            pad.SetPadName(name)
            pad.SetDelta(wxSize(left_right*size[1],up_down*size[0]))
            return pad


    # This method checks the parameters provided to wizard and set errors
    def CheckParameters(self):
        p = self.parameters        
        steps            = p["Pads"]["*steps"]        
        errors = ""
        if (steps<1):
            self.parameter_errors["Pads"]["*steps"]="Must be positive"
            errors +="Pads/*steps has wrong value"
        p["Pads"]["*steps"] = int(pads)  # make sure it stays as int (default is float)       
        
        bands            = p["Pads"]["*bands"]   
        
        if (bands<1):
            self.parameter_errors["Pads"]["*bands"]="Must be positive"
            errors +="Pads/*bands has wrong value"
        p["Pads"]["*bands"] = int(bands)  # make sure it stays as int (default is float)       
        
                    
        touch_width       = p["Pads"]["width"]
        touch_length      = p["Pads"]["length"]
        touch_clearance   = p["Pads"]["clearance"]
        
        
        return errors 

    # The start pad is made of a rectangular pad plus a couple of 
    # triangular pads facing tips on the middle/right of the first
    # rectangular pad
    def AddStartPad(self,position,touch_width,step_length,clearance,name):
        module = self.module
        step_length = step_length - clearance
        size_pad = wxSize(step_length/2.0+(step_length/3),touch_width)
        pad = self.smdRectPad(module,size_pad,position-wxPoint(step_length/6,0),name)
        module.Add(pad)
        
        size_pad = wxSize(step_length/2.0,touch_width)        
        
        tp = self.smdTrianglePad(module,wxSize(size_pad[0],size_pad[1]/2),
                                 position+wxPoint(size_pad[0]/2,size_pad[1]/4),
                                name)
        module.Add(tp)
        tp = self.smdTrianglePad(module,wxSize(size_pad[0],size_pad[1]/2),
                                 position+wxPoint(size_pad[0]/2,-size_pad[1]/4),
                                name
                                ,-1)
        module.Add(tp)
 
    # compound a "start pad" shape plus a triangle on the left, pointing to 
    # the previous touch-pad
    def AddMiddlePad(self,position,touch_width,step_length,clearance,name):
        module = self.module
        step_length = step_length - clearance
        size_pad = wxSize(step_length/2.0,touch_width)
        
        size_pad = wxSize(step_length/2.0,touch_width)
        pad = self.smdRectPad(module,size_pad,position,name)
        module.Add(pad)
        
        tp = self.smdTrianglePad(module,wxSize(size_pad[0],size_pad[1]/2),
                                 position+wxPoint(size_pad[0]/2,size_pad[1]/4),
                                name)
        module.Add(tp)
        tp = self.smdTrianglePad(module,wxSize(size_pad[0],size_pad[1]/2),
                                 position+wxPoint(size_pad[0]/2,-size_pad[1]/4),
                                name
                                ,-1)
        module.Add(tp)
        
        tp = self.smdTrianglePad(module,wxSize(size_pad[0],size_pad[1]/2),
                                        position+wxPoint(-size_pad[0],0),
                                        name,
                                        0,
                                        -1)
        module.Add(tp)
        
        
    def AddFinalPad(self,position,touch_width,step_length,clearance,name):
        module = self.module
        step_length = step_length - clearance
        size_pad = wxSize(step_length/2.0,touch_width)
        
        pad = self.smdRectPad(module,
                              wxSize(size_pad[0]+(step_length/3),size_pad[1]),
                              position+wxPoint(step_length/6,0),
                              name)
        module.Add(pad)
        
        tp = self.smdTrianglePad(module,wxSize(size_pad[0],size_pad[1]/2),
                                        position+wxPoint(-size_pad[0],0),
                                        name,
                                        0,
                                        -1)
        module.Add(tp)
    
    def AddStrip(self,pos,steps,touch_width,step_length,touch_clearance):
        self.AddStartPad(pos,touch_width,step_length,touch_clearance,"1")

        for n in range(2,steps):
            pos = pos + wxPoint(step_length,0)
            self.AddMiddlePad(pos,touch_width,step_length,touch_clearance,str(n))
            
        pos = pos + wxPoint(step_length,0)
        self.AddFinalPad(pos,touch_width,step_length,touch_clearance,str(steps))
        
    # build the footprint from parameters 
    def BuildFootprint(self):
        
        module = MODULE(None) # create a new module
        self.module = module
        
        p = self.parameters
        steps             = int(p["Pads"]["*steps"])        
        bands             = int(p["Pads"]["*bands"])   
        touch_width       = p["Pads"]["width"]
        touch_length      = p["Pads"]["length"]
        touch_clearance   = p["Pads"]["clearance"]
        
        step_length = float(touch_length) / float(steps)    
        
        module.SetReference("TS"+str(steps))   # give it a reference name
        module.m_Reference.SetPos0(wxPointMM(-1,-2))
        module.m_Reference.SetPosition(wxPointMM(-1,-2))
    
        # starting pad
    
        pos = wxPointMM(0,0)
        band_width = touch_width/bands
        
        for b in range(bands):

            self.AddStrip(pos,steps,band_width,step_length,touch_clearance)
            pos+=wxPoint(0,band_width)
            
        
        
        
        module.SetLibRef("S"+str(steps))        


# create our footprint wizard
touch_slider_wizard = TouchSliderWizard() 

# register it into pcbnew
touch_slider_wizard.register()
