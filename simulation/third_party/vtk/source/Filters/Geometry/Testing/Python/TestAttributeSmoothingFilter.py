#!/usr/bin/env python
from vtkmodules.vtkCommonCore import (
    vtkFloatArray,
    vtkPoints,
    vtkUnsignedCharArray,
)
from vtkmodules.vtkCommonDataModel import (
    vtkCellArray,
    vtkPolyData,
)
from vtkmodules.vtkFiltersCore import vtkExecutionTimer
from vtkmodules.vtkFiltersGeometry import vtkAttributeSmoothingFilter
from vtkmodules.vtkFiltersSources import vtkPlaneSource
from vtkmodules.vtkRenderingCore import (
    vtkActor,
    vtkPolyDataMapper,
    vtkRenderWindow,
    vtkRenderWindowInteractor,
    vtkRenderer,
)
import vtkmodules.vtkInteractionStyle
import vtkmodules.vtkRenderingFreeType
import vtkmodules.vtkRenderingOpenGL2
from vtkmodules.util.misc import vtkGetDataRoot
VTK_DATA_ROOT = vtkGetDataRoot()

# Test the vtkAttributeSmoothingFilter

# Control test size
res = 25

# Generate synthetic attributes
#
ps = vtkPlaneSource()
ps.SetXResolution(res-1)
ps.SetYResolution(res-1)
ps.Update()

pd = vtkPolyData()
pts = vtkPoints()
polys = vtkCellArray()
pd.SetPoints(ps.GetOutput().GetPoints())
pd.SetPolys(ps.GetOutput().GetPolys())

# Create some synthetic attribute data
s = vtkFloatArray()
s.SetNumberOfTuples(res*res)
s.Fill(4) # Interior all the same value
pd.GetPointData().SetScalars(s)

def SetRow(row,value):
    for x in range(0,res):
        idx = x + row*res
        s.SetTuple1(idx,value)

def SetColumn(col,value):
    for y in range(0,res):
        idx = col + y*res
        s.SetTuple1(idx,value)

def SetPoint(col,row,value):
    idx = col + row*res
    s.SetTuple1(idx,value)

# Set values adjacent to boundary
SetRow(1,1)
SetRow(res-2,1)
SetColumn(1,1)
SetColumn(res-2,1)

# Set boundary values
SetRow(0,0)
SetRow(res-1,0)
SetColumn(0,0)
SetColumn(res-1,0)

# Set center point
SetPoint(int(res/2),int(res/2),0)

# Display what we've got
pdm0 = vtkPolyDataMapper()
pdm0.SetInputData(pd)
pdm0.SetScalarRange(0,4)

a0 = vtkActor()
a0.SetMapper(pdm0)

# Different ways smooth the attribute data
att1 = vtkAttributeSmoothingFilter()
att1.SetInputData(pd)
att1.SetSmoothingStrategyToAllPoints()
att1.SetNumberOfIterations(50)
att1.SetRelaxationFactor(0.1)

timer = vtkExecutionTimer()
timer.SetFilter(att1)
att1.Update()
t1 = timer.GetElapsedWallClockTime()
print ("Smooth Attributes: ", t1)

pdm1 = vtkPolyDataMapper()
pdm1.SetInputConnection(att1.GetOutputPort())
pdm1.SetScalarRange(0,4)

a1 = vtkActor()
a1.SetMapper(pdm1)

# Smooth everything but the boundary
att2 = vtkAttributeSmoothingFilter()
att2.SetInputData(pd)
att2.SetSmoothingStrategyToAllButBoundary()
att2.SetNumberOfIterations(2)
att2.SetRelaxationFactor(0.5)
att2.Update()

pdm2 = vtkPolyDataMapper()
pdm2.SetInputConnection(att2.GetOutputPort())
pdm2.SetScalarRange(0,4)

a2 = vtkActor()
a2.SetMapper(pdm2)

# Only smooth points near boundary
att3 = vtkAttributeSmoothingFilter()
att3.SetInputData(pd)
att3.SetSmoothingStrategyToAdjacentToBoundary()
att3.SetNumberOfIterations(3)
att3.SetRelaxationFactor(0.333)
att3.Update()

pdm3 = vtkPolyDataMapper()
pdm3.SetInputConnection(att3.GetOutputPort())
pdm3.SetScalarRange(0,4)

a3 = vtkActor()
a3.SetMapper(pdm3)

# Using smoothing mask, smooth all points except center point
smoothArray = vtkUnsignedCharArray()
smoothArray.SetNumberOfTuples(res*res)
smoothArray.Fill(1) # Smooth all points
idx = int(res/2) + int(res/2)*res
smoothArray.SetTuple1(idx,0) # Except the center point

att4 = vtkAttributeSmoothingFilter()
att4.SetInputData(pd)
att4.SetSmoothingStrategyToSmoothingMask()
att4.SetNumberOfIterations(50)
att4.SetRelaxationFactor(0.10)
att4.SetSmoothingMask(smoothArray)
att4.Update()

pdm4 = vtkPolyDataMapper()
pdm4.SetInputConnection(att4.GetOutputPort())
pdm4.SetScalarRange(0,4)

a4 = vtkActor()
a4.SetMapper(pdm4)

# Create the RenderWindow, Renderer and interactive renderer
#
renWin = vtkRenderWindow()
ren0 = vtkRenderer()
ren0.SetViewport(0,0,0.20,1)
ren1 = vtkRenderer()
ren1.SetViewport(0.20,0,0.40,1)
ren2 = vtkRenderer()
ren2.SetViewport(0.40,0,0.60,1)
ren3 = vtkRenderer()
ren3.SetViewport(0.60,0,0.80,1)
ren4 = vtkRenderer()
ren4.SetViewport(0.80,0,1,1)
renWin.AddRenderer(ren0)
renWin.AddRenderer(ren1)
renWin.AddRenderer(ren2)
renWin.AddRenderer(ren3)
renWin.AddRenderer(ren4)

# make sure to have the same regression image on all platforms.
renWin.SetMultiSamples(0)
iren = vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)

ren0.AddActor(a0)
ren0.SetBackground(0, 0, 0)
ren1.AddActor(a1)
ren1.SetBackground(0, 0, 0)
ren2.AddActor(a2)
ren2.SetBackground(0, 0, 0)
ren3.AddActor(a3)
ren3.SetBackground(0, 0, 0)
ren4.AddActor(a4)
ren4.SetBackground(0, 0, 0)

renWin.SetSize(1000, 200)

renWin.Render()
iren.Start()
