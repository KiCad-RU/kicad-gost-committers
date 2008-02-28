{*
 * PCadToKiCad, PCad ASCII to KiCad conversion utility
 * Copyright (C) 2007, 2008 Alexander Lunev <alexanderlunev@mail.ru>
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

{ Fills a polygon }

unit PCBPolygonsUnit;

interface
uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, xmldom, XMLIntf, msxmldom, XMLDoc, ComCtrls, oxmldom,
  PCBComponents, Math;

Function triangulate_polygon(vertexes_qty:Integer;
         vertices: Pointer;
         triangles: Pointer):Integer; cdecl;
         external 'tri.dll' Index 1;

type
   TPoint = Record
          x, y: double;
          end;

   TPolyline = Array of TPoint;
   TvgrpArray = array of array [0..1] of integer;
   TLine = Record
          Xst, Yst, Xend, Yend: Real;
          end;

   TLineArray = Array of TLine;
   TTriangle = array[0..2,0..1] of double;
   TTriangleArray = array of TTriangle;
   TDoubleArray = array of double;

 
procedure FillZone(island: TList; cutouts: TList; var result_zone:TList; step: double; width:integer;
                    pourType:string;
                    PCADlayer:integer; KiCADlayer:integer; timestamp:integer);

implementation


Procedure Sort(var arr : TDoubleArray);
var
   i,j : Integer;
   temp : Real;
begin
   for i := 1 to High(arr) do
   begin
      for j := High(arr) downto i do
         if arr[j-1]>arr[j] then
         begin
            temp := arr[j-1];
            arr[j-1] := arr[j];
            arr[j] := temp;
         end;
   end;
end;

{Procedure TransposeTriangles(var arr : TTriangleArray);
var
   i, v : Integer;
   temp : double;
begin
   for i:=0 to High(arr) do
    for v:= 0 to 2 do
    begin
      temp := arr[i][v][0];
      arr[i][v][0] := arr[i][v][1];
      arr[i][v][1] := temp;
    end;
end;

Procedure TransposeLines(var arr : TList);
var
   i : Integer;
   temp : integer;
begin
   for i:=0 to arr.Count-1 do
   begin
      temp := THPCBLine(arr[i]).ToX;
      THPCBLine(arr[i]).ToX := THPCBLine(arr[i]).ToY;
      THPCBLine(arr[i]).ToY := temp;

      temp := THPCBLine(arr[i]).PositionX;
      THPCBLine(arr[i]).PositionX := THPCBLine(arr[i]).PositionY;
      THPCBLine(arr[i]).PositionY := temp;
   end;
end;}

Function PointInsideSegment(segment:TLine; point_x:double; point_y:double):Boolean;
var
  min_x, max_x, min_y, max_y: double;
begin
  min_x := min(segment.Xst, segment.Xend);
  max_x := max(segment.Xst, segment.Xend);
  min_y := min(segment.Yst, segment.Yend);
  max_y := max(segment.Yst, segment.Yend);

  if (point_x>=min_x) and (point_x<=max_x) and
      (point_y>=min_y) and (point_y<=max_y) then result:=true
  else result:=false;
end;

Function PointInsideTriangle(triangle:TTriangle;
                             point_x:double; point_y:double):Boolean;
var
  i: Integer;
  r, rr, prod: Real;
  segment:TLine;
begin
  for i:=0 to 2 do
  begin
    segment.Xst:= triangle[i][0];
    segment.Yst:= triangle[i][1];
    if i=2 then
    begin
      segment.Xend:= triangle[i-2][0];
      segment.Yend:= triangle[i-2][1];
    end
    else
    begin
      segment.Xend:= triangle[i+1][0];
      segment.Yend:= triangle[i+1][1];
    end;

    r:= (point_y-segment.Yst)*(segment.Xend-segment.Xst) -
         (point_x-segment.Xst)*(segment.Yend-segment.Yst);

    if r=0 then break;
    prod:=r*rr;
    if (i>0) and (prod<0) then break;
    rr:=r;
  end;

  result:=((r=0) and PointInsideSegment(segment, point_x, point_y)) or ((r<>0) and (prod>0));
end;

Function PointInsideTPoly(tpoly:array of TTriangle;
                             point_x:double; point_y:double):Boolean;
var
  i: Integer;
begin
  result := false;
  for i := 0 to High(tpoly) do
    if (PointInsideTriangle(tpoly[i], point_x, point_y)) then
    begin
      result:=true;
      break;
    end;
end;

Procedure InsertNewValue(var value_arr: TDoubleArray; value:double);
var i: integer;
    found: boolean;
begin
  found:=false;
  for i:=0 to High(value_arr) do
    if (value_arr[i]=value) then
    begin
      found:=true;
      break;
    end;

    if (found=false) then
    begin
      SetLength(value_arr, Length(value_arr) + 1);
      value_arr[High(value_arr)]:=value;
    end;
end;


Procedure Zone (triangles_to_fill: TTriangleArray; cutout_triangles: TTriangleArray;
                var result_zone: TList; transpose: boolean;
                step: double; width: integer; PCADlayer:integer; KiCADlayer:integer;
                timestamp:integer);
type
   TLineExtended = Record
          Xst, Yst, Xend, Yend, k, b: Real;
          end;

   TLineExtendedArray = Array of TLineExtended;

Var
   segments : TLineExtendedArray;
   i, v, end_v, tri_qty, offs : Integer;
   left_top_corner_x : Real;
   right_bot_corner_x : Real;
   vert_line_x : Real;
   intersections : TDoubleArray;
   intrsect_y, middle : Real;
   min_x, max_x, min_y, max_y : Real;
   vertices: Array of Array [0..1] of double;
   v_grps_qty: Integer;
   v_grps: Array of Array [0..1] of Integer;
   zoneline: THPCBLine;
begin
   SetLength(segments, Length(triangles_to_fill)*3);
   For i:=0 to High(triangles_to_fill) do
   begin
      for v:=0 to 2 do
      begin
        if (v<2) then end_v:= v + 1 else end_v:= 0;
        if (transpose) then
        begin
          segments[i*3 + v].Xst := triangles_to_fill[i][v][1];
          segments[i*3 + v].Yst := triangles_to_fill[i][v][0];
          segments[i*3 + v].Xend := triangles_to_fill[i][end_v][1];
          segments[i*3 + v].Yend := triangles_to_fill[i][end_v][0];
        end
        else
        begin
          segments[i*3 + v].Xst := triangles_to_fill[i][v][0];
          segments[i*3 + v].Yst := triangles_to_fill[i][v][1];
          segments[i*3 + v].Xend := triangles_to_fill[i][end_v][0];
          segments[i*3 + v].Yend := triangles_to_fill[i][end_v][1];
        end;
      end;
   end;

   {Find left top corner and right bottom corner}
   left_top_corner_x := 1.7E308;
   right_bot_corner_x := -1.7E308;
   For i:=0 to High(segments) do
   begin
      if (segments[i].Xst < left_top_corner_x) then
         left_top_corner_x := segments[i].Xst;
      if (segments[i].Xend < left_top_corner_x) then
         left_top_corner_x := segments[i].Xend;
      if (segments[i].Xst > right_bot_corner_x) then
         right_bot_corner_x := segments[i].Xst;
      if (segments[i].Xend > right_bot_corner_x) then
         right_bot_corner_x := segments[i].Xend;
   end;

   offs:=Length(segments);
   SetLength(segments, offs + Length(cutout_triangles)*3);

   For i:=0 to High(cutout_triangles) do
   begin
      for v:=0 to 2 do
      begin
        if (v<2) then end_v:= v + 1 else end_v:= 0;
        if (transpose) then
        begin
          segments[offs + i*3 + v].Xst := cutout_triangles[i][v][1];
          segments[offs + i*3 + v].Yst := cutout_triangles[i][v][0];
          segments[offs + i*3 + v].Xend := cutout_triangles[i][end_v][1];
          segments[offs + i*3 + v].Yend := cutout_triangles[i][end_v][0];
        end
        else
        begin
          segments[offs + i*3 + v].Xst := cutout_triangles[i][v][0];
          segments[offs + i*3 + v].Yst := cutout_triangles[i][v][1];
          segments[offs + i*3 + v].Xend := cutout_triangles[i][end_v][0];
          segments[offs + i*3 + v].Yend := cutout_triangles[i][end_v][1];
        end;
      end;
   end;

   {Compute all input lines}
   For i:=0 to High(segments) do
   begin
      if (segments[i].Xst = segments[i].Xend) then
      begin
         segments[i].k := 1.7E308;
         segments[i].b := 0;
      end
      else
      begin
         segments[i].k := (segments[i].Yst - segments[i].Yend) /
                          (segments[i].Xst - segments[i].Xend);
         segments[i].b := segments[i].Yst - segments[i].k * segments[i].Xst;
      end;
   end;

   {vertical lines}
   vert_line_x := left_top_corner_x;
   while (vert_line_x < right_bot_corner_x) do
   begin
      SetLength(intersections, 0);
      For i:=0 to High(segments) do
      begin
         min_x := min(segments[i].Xst, segments[i].Xend);
         max_x := max(segments[i].Xst, segments[i].Xend);
         if (segments[i].Xst<>segments[i].Xend) and
            (vert_line_x>=min_x) and
            (vert_line_x<=max_x) then
         begin
            intrsect_y := segments[i].k * vert_line_x + segments[i].b;
            min_y := min(segments[i].Yst, segments[i].Yend);
            max_y := max(segments[i].Yst, segments[i].Yend);
            if (intrsect_y>=min_y) and (intrsect_y<=max_y) then
            begin
               {add new intersection to array}
               InsertNewValue(intersections, intrsect_y);
            end;
         end;
      end;
      Sort(intersections);
      {add new line to result_zone array}
      if (Length(intersections)>1) then
      begin
         For i:=0 to High(intersections)-1 do
         begin
            middle:=(intersections[i] + intersections[i+1]) / 2;
            if (transpose and PointInsideTPoly(triangles_to_fill, middle, vert_line_x) and
                not PointInsideTPoly(cutout_triangles, middle, vert_line_x) or
                not transpose and PointInsideTPoly(triangles_to_fill, vert_line_x, middle) and
                not PointInsideTPoly(cutout_triangles, vert_line_x, middle)) then
            begin
              zoneline:=THPCBLine.Create;
              zoneline.PCadLayer:=PCadLayer;
              zoneline.KiCadLayer:=KiCADlayer;
              zoneline.Width:=width;
              zoneline.timestamp:=timestamp;
              if (transpose) then
              begin
                zoneline.PositionX:=round(intersections[i]);
                zoneline.PositionY:=round(vert_line_x);
                zoneline.ToX:=round(intersections[i+1]);
                zoneline.ToY:=round(vert_line_x);
              end
              else
              begin
                zoneline.PositionX:=round(vert_line_x);
                zoneline.PositionY:=round(intersections[i]);
                zoneline.ToX:=round(vert_line_x);
                zoneline.ToY:=round(intersections[i+1]);
              end;
              result_zone.Add(zoneline);
            end;
         end;
      end;
      vert_line_x := vert_line_x + step;
   end;
end;


procedure FillZone(island: TList; cutouts: TList; var result_zone:TList; step: double; width:integer;
                    pourType:string;
                    PCADlayer:integer; KiCADlayer:integer; timestamp:integer);
var
  triangles_to_fill: TTriangleArray;
  cutout_triangles, cutouts_triangles: TTriangleArray;
  vertices: TPolyLine;
  i, c, tri_qty, offs: Integer;
begin
  SetLength(vertices, island.Count);
  for i:=0 to island.Count-1 do
  begin
    vertices[i].x:=TVertex(island[i]).x;
    vertices[i].y:=TVertex(island[i]).y;
  end;

  SetLength(triangles_to_fill, 3*Length(vertices));

  tri_qty:=triangulate_polygon(Length(vertices), vertices, triangles_to_fill);
  SetLength(triangles_to_fill, tri_qty);

  //PCBCutouts(PCBComponents, i, polygon_netname, cutout_triangles);
  SetLength(cutouts_triangles, 0);
  for c:=0 to cutouts.Count-1 do
    with TList(cutouts[c]) do
    begin
      SetLength(vertices, Count);
      for i:=0 to Count-1 do
      begin
        vertices[i].x:=TVertex(TList(cutouts[c])[i]).x;
        vertices[i].y:=TVertex(TList(cutouts[c])[i]).y;
      end;

      SetLength(cutout_triangles, 3*Length(vertices));
      tri_qty:=triangulate_polygon(Length(vertices), vertices, cutout_triangles);
      SetLength(cutout_triangles, tri_qty);
      offs:= Length(cutouts_triangles);
      SetLength(cutouts_triangles, offs + tri_qty);
      for i:=0 to tri_qty-1 do
        cutouts_triangles[offs+i]:=cutout_triangles[i];
    end;

   if (pourType='SOLIDPOUR') then
   begin
      step:=25;
      pourType:='HATCH90POUR';
   end
   else if (pourType='HATCH45POUR') then
   begin
   end;

   if (pourType='HATCH90POUR') or (pourType='HORIZPOUR') then
   begin
     {Compute horizontal lines of polygon}
     Zone(triangles_to_fill, cutouts_triangles, result_zone, true, step, width,
          PCADlayer, KiCADlayer, timestamp);
   end;

   if (pourType='HATCH90POUR') or (pourType='VERTPOUR') then
     {Compute vertical lines of polygon}
     Zone(triangles_to_fill, cutouts_triangles, result_zone, false, step, width,
           PCADlayer, KiCADlayer, timestamp);


end;

{Procedure FillTriangle(triangle:TTriangle;offs_x:double; step:double;
                        var lines:TLineArray);
var
  i: Integer;
  vert_line_x, intersection_y,
  left_top_corner_x, right_bot_corner_x: double;
  segment:TLine;
  intersections : TDoubleArray;
begin
  //Find left top corner and right bottom corner
  left_top_corner_x := 1.7E308;
  right_bot_corner_x := -1.7E308;
  For i:=0 to 2 do
  begin
     if (triangle[i][0] < left_top_corner_x) then
        left_top_corner_x := triangle[i][0];
     if (triangle[i][0] > right_bot_corner_x) then
        right_bot_corner_x := triangle[i][0];
  end;

  vert_line_x := left_top_corner_x;
  while (vert_line_x <=right_bot_corner_x) do
  begin
    SetLength(intersections, 0);
    for i:=0 to 2 do
    begin
        segment.Xst:= triangle[i][0];
        segment.Yst:= triangle[i][1];
        if i=2 then
        begin
          segment.Xend:= triangle[i-2][0];
          segment.Yend:= triangle[i-2][1];
        end
        else
        begin
          segment.Xend:= triangle[i+1][0];
          segment.Yend:= triangle[i+1][1];
        end;

        if (segment.Xst<>segment.Xend) then
        begin
          intersection_y:= (segment.Yst - segment.Yend) * (vert_line_x - segment.Xst) /
                            (segment.Xst - segment.Xend) + segment.Yst;
          if (PointInsideSegment(segment, vert_line_x, intersection_y)) then
            InsertNewValue(intersections, intersection_y);
        end
        else if (segment.Xst=vert_line_x) and (segment.Xend=vert_line_x) then
        begin
            InsertNewValue(intersections, segment.Yst);
            InsertNewValue(intersections, segment.Yend);
        end;
    end;
    if (Length(intersections)>1) then
    begin
      SetLength(lines, Length(lines) + 1);
      lines[High(lines)].Xst := vert_line_x;
      lines[High(lines)].Xend := vert_line_x;
      lines[High(lines)].Yst := intersections[0];
      lines[High(lines)].Yend := intersections[1];
    end;

    vert_line_x := vert_line_x + step;
  end;
end;

procedure LineOutline(var polyline:TPolyline; gap:Real);
var x1, x2, y1, y2: Real;
    A_main, B_main, C_main: Real;
    A1, A2, A3, A4, B1, B2, B3, B4, C1, C2, C3, C4: Real;
    line_length: Real;
begin
  x1:=polyline[0].x; x2:=polyline[1].x; y1:=polyline[0].y; y2:=polyline[1].y;
  line_length:= sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
  SetLength(polyline, 4);
  B_main:= -(x1 - x2);
  A_main:= y1 - y2;
  C_main:= -(y1 - y2)*x1 + (x1 - x2)*y1;

  A1:= A_main;
  B1:= B_main;
  C1:= C_main + gap*line_length;
  A2:= A_main;
  B2:= B_main;
  C2:= C_main - gap*line_length;
  A3:= -B_main;
  B3:= A_main;
  C3:= -A3*x2 - B3*y2 + gap*line_length;
  A4:= A3;
  B4:= B3;
  C4:= -A4*x1 - B4*y1 + gap*line_length;

  polyline[0].x:= (B1*C3 - B3*C1) / (A1*B3 - A3*B1);
  polyline[0].y:= (C1*A3 - C3*A1) / (A1*B3 - A3*B1);

  polyline[1].x:= (B1*C4 - B4*C1) / (A1*B4 - A4*B1);
  polyline[1].y:= (C1*A4 - C4*A1) / (A1*B4 - A4*B1);

  polyline[2].x:= (B2*C4 - B4*C2) / (A2*B4 - A4*B2);
  polyline[2].y:= (C2*A4 - C4*A2) / (A2*B4 - A4*B2);

  polyline[3].x:= (B2*C3 - B3*C2) / (A2*B3 - A3*B2);
  polyline[3].y:= (C2*A3 - C3*A2) / (A2*B3 - A3*B2);
end;

procedure PCBCutouts(PCBComponents:TList; parent_i:integer; polygon_netname:string;
                      var cutouts:TTriangleArray);
var i, line_i, tri_qty, offs:integer;
    vertices, polyline: TPolyLine;
    v_grps: TvgrpArray;
    triangles: TTriangleArray;
begin
  SetLength(cutouts, 0);
  SetLength(v_grps, 1);
  v_grps[High(v_grps)][1]:=0;
  for i:=0 to PCBComponents.Count-1 do
  begin
    if (i=parent_i) then continue;

    SetLength(vertices, 0);
    if THPCBComponent(PCBComponents[i]).ObjType='C' then // Polygon object
    begin
      with THPCBCutout(PCBComponents[i]) do
      begin
        SetLength(vertices, outline.Count);
        for line_i := 0 to outline.Count - 1 do
        begin
          vertices[line_i].x:= THPCBLine(outline[line_i]).PositionX;
          vertices[line_i].y:= THPCBLine(outline[line_i]).PositionY;
        end;
      end;
    end
    else if (THPCBComponent(PCBComponents[i]).ObjType='Z') and
            (THPCBComponent(PCBComponents[i]).NetlistName<>polygon_netname) then // Polygon object
    begin
      with THPCBPolygon(PCBComponents[i]) do
      begin
        SetLength(vertices, outline.Count);
        for line_i := 0 to outline.Count - 1 do
        begin
          vertices[line_i].x:= THPCBLine(outline[line_i]).PositionX;
          vertices[line_i].y:= THPCBLine(outline[line_i]).PositionY;
        end;
      end;
    end
    //else if THPCBComponent(PCBComponents[i]).ObjType='M' then // Module object
    else if THPCBLine(PCBComponents[i]).ObjType='L' then // Line object
    begin
      with THPCBLine(PCBComponents[i]) do
      begin
          SetLength(polyline, 2);
          polyline[0].x:= PositionX;
          polyline[0].y:= PositionY;
          polyline[1].x:= ToX;
          polyline[1].y:= ToY;

          LineOutline(polyline, 200);

          for line_i := 0 to High(polyline) do
          begin
            SetLength(vertices, Length(vertices) + 1);
            vertices[High(vertices)].x:= polyline[line_i].x;
            vertices[High(vertices)].y:= polyline[line_i].y;
          end;
      end;
    end;

    if (Length(vertices)>0) then
    begin
      v_grps[High(v_grps)][0]:=Length(vertices);
      SetLength(triangles, 3*Length(vertices));

      tri_qty:=triangulate_polygon(Length(vertices), @vertices[0],
                                Length(v_grps), @v_grps[0,0],@triangles[0,0,0]);
      offs:=Length(cutouts);
      SetLength(cutouts, Length(cutouts)+tri_qty);
      for line_i := 0 to tri_qty-1 do
      begin
        cutouts[line_i+offs]:= triangles[line_i];
      end;
    end;
  end;
end;}

end.
