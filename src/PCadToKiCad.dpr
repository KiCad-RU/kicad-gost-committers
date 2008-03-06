{*
 * PCadToKiCad, PCad ASCII to KiCad conversion utility
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 *}

program PCadToKiCad;

{%File 'Readme'}

uses
  Forms,
  PCadToKiCadUnit in 'PCadToKiCadUnit.pas' {PCadToKiCadPCBForm},
  TextToXMLUnit in 'TextToXMLUnit.pas',
  PCBComponents in 'PCBComponents.pas',
  LoadInputFileUnit in 'LoadInputFileUnit.pas',
  ProcessXMLToPCBUnit in 'ProcessXMLToPCBUnit.pas',
  SCHComponents in 'SCHComponents.pas',
  PCBPolygonsUnit in 'PCBPolygonsUnit.pas',
  ProcessXMLToSCHUnit in 'ProcessXMLToSCHUnit.pas',
  XMLToObjectCommonProceduresUnit in 'XMLToObjectCommonProceduresUnit.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.Title := 'PCadToKiCad - PCad ASCII to KiCad conversion utility';
  Application.CreateForm(TPCadToKiCadPCBForm, PCadToKiCadPCBForm);
  Application.Run;
end.
