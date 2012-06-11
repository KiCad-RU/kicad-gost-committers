/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2012 Alexander Lunev <al.lunev@yahoo.com>
 * Copyright (C) 2012 KiCad Developers, see CHANGELOG.TXT for contributors.
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
 * @file TextToXMLUnit.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <wx/xml/xml.h>

/*
unit TextToXMLUnit;

interface
uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, xmldom, XMLIntf, msxmldom, XMLDoc, ComCtrls, oxmldom;

procedure TextToXML(StatusBar:TStatusBar;XMLFileName:String;tLines:TMemo);

implementation
procedure TextToXML(StatusBar:TStatusBar;XMLFileName:String;tLines:TMemo);
var
    xmlDoc : TXMLDocument;
    iNode : IXMLNode;
    i:integer;
//    TotalLines:integer;

 procedure ProcessLine(iNode:IXMLNode);
 var ls:string;
    cNode : IXMLNode;
    i:integer;
    Go:boolean;
 begin
  Application.ProcessMessages;
  if tLines.Lines.Count=0 then exit;
  StatusBar.SimpleText:='Creating XML file : '+IntToStr(tLines.Lines.Count);
  ls:=tLines.Lines.Strings[tLines.Lines.Count-1];
  tLines.Lines.Delete(tLines.Lines.Count-1);
  if (ls='GoUP') then
     ProcessLine(iNode.GetPArentNode)
  else
  if (ls='GoDOWN') then
   begin
      ls:=tLines.Lines.Strings[tLines.Lines.Count-1];
      tLines.Lines.Delete(tLines.Lines.Count-1);
      cNode := iNode.AddChild(ls);
      cNode.Text :='';
      while tLines.Lines.Count>0 do
      begin
        ls:=tLines.Lines.Strings[tLines.Lines.Count-1];
        if ((ls='GoUP') or (ls='GoDOWN')) then ProcessLine(cNode)
        else
        begin
          if ((length(ls)>0) and (ls[1]='"')) then
           begin
              ls:=copy(ls,2,length(ls)-2);
              if (cNode.Attributes['Name']<>null) then
                   cNode.Attributes['Name'] := cNode.Attributes['Name']+' '+ls
              else cNode.Attributes['Name']:=ls;
           end
           else
             begin
                cNode.Text :=cNode.Text + ' ' + ls;
             end;
          tLines.Lines.Delete(tLines.Lines.Count-1);
        end;
      end;
   end
   else
   begin
      cNode := iNode.AddChild('value');
      if ((length(ls)>0) and (ls[1]='"')) then
           begin
              ls:=copy(ls,2,length(ls)-2);
           end;
      cNode.Text :=ls;
      ProcessLine(iNode);
   end;
 end;
begin
   try
     xmlDoc := TXMLDocument.Create(nil) ;
     xmlDoc.Active := true;
     iNode := XMLDoc.AddChild('www.lura.sk');
     while tLines.Lines.Count>0 do
       begin
          ProcessLine(iNode);
      end;
     StatusBar.SimpleText:='Saving XML file : '+XMLFileName;
     XMLDoc.SaveToFile(XMLFileName);
   finally
     xmlDoc.Active := false;
   end;
end;
end.
*/

void ProcessLine(wxStatusBar* statusBar, wxXmlNode *iNode, wxArrayString *tLines) {
    wxString ls, propValue, content;
    wxXmlNode *cNode;

    if (tLines->GetCount() == 0) return;

    statusBar->SetStatusText(wxT("Creating XML file : ") + wxString::Format("%d", tLines->GetCount()));
    ls = (*tLines)[tLines->GetCount() - 1];
    tLines->RemoveAt(tLines->GetCount() - 1);
    if (ls == wxT("GoUP")) ProcessLine(statusBar, iNode->GetParent(), tLines);
    else if (ls == wxT("GoDOWN")) {
        ls = (*tLines)[tLines->GetCount() - 1];
        tLines->RemoveAt(tLines->GetCount() - 1);
        cNode = new wxXmlNode(wxXML_ELEMENT_NODE, ls);
        iNode->AddChild(cNode);
        while (tLines->GetCount() > 0) {
            ls = (*tLines)[tLines->GetCount() - 1];
            if (ls == wxT("GoUP") || ls == wxT("GoDOWN")) ProcessLine(statusBar, cNode, tLines);
            else {
                if ((ls.Len() > 0) && (ls[0] == '"')) {
                    ls = ls.Mid(1, ls.Len() - 2);
                    if (cNode->GetPropVal(wxT("Name"), &propValue)) {
                        cNode->DeleteProperty(wxT("Name"));
                        cNode->AddProperty(wxT("Name"), propValue + ' ' + ls);
                    }
                    else {
                        cNode->DeleteProperty(wxT("Name"));
                        cNode->AddProperty(wxT("Name"), ls);
                    }
                }
                else {
                    // update node content
                    content = cNode->GetNodeContent() + ' ' + ls;
                    cNode->RemoveChild(cNode->GetChildren());
                    cNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxEmptyString, content));
                }

                tLines->RemoveAt(tLines->GetCount() - 1);
            }
        }
    }
    else {
        if (ls.Len() > 0 && ls[0] == '"') {
            ls = ls.Mid(1, ls.Len() - 2);
        }

        cNode = new wxXmlNode(wxXML_ELEMENT_NODE, wxT("value"));
        iNode->AddChild(cNode);

        if (ls.Len() > 0 && ls[0] == '"') {
            ls = ls.Mid(1, ls.Len() - 2);
        }

        cNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, wxEmptyString, ls));

        ProcessLine(statusBar, iNode, tLines);
    }
}

void TextToXML(wxStatusBar* statusBar, wxString XMLFileName, wxArrayString *tLines) {
    wxXmlDocument xmlDoc;
    wxString ls;

    wxXmlNode *root = new wxXmlNode(wxXML_ELEMENT_NODE, wxT("www.lura.sk"));
    xmlDoc.SetRoot(root);

    while (tLines->GetCount() > 0) {
        ProcessLine(statusBar, root, tLines); // here is the recursion that should be eliminated!
    }

    statusBar->SetStatusText(wxT("Saving XML file : ") + XMLFileName);
    xmlDoc.Save(XMLFileName);
}
