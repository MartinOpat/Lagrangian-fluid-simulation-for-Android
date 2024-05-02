#!/usr/bin/env python

import os
from vtkmodules.vtkCommonDataModel import vtkImageData
from vtkmodules.vtkFiltersCore import vtkContourFilter
from vtkmodules.vtkIOImage import vtkImageReader
from vtkmodules.vtkIOXML import (
    vtkXMLImageDataReader,
    vtkXMLImageDataWriter,
)
from vtkmodules.vtkImagingCore import vtkExtractVOI
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
from vtkmodules.util.misc import vtkGetTempDir

VTK_DATA_ROOT = vtkGetDataRoot()
VTK_TEMP_DIR = vtkGetTempDir()

file0 = VTK_TEMP_DIR + '/idFile0.vti'
file1 = VTK_TEMP_DIR + '/idFile1.vti'
file2 = VTK_TEMP_DIR + '/idFile2.vti'

# read in some image data
imageReader = vtkImageReader()
imageReader.SetDataByteOrderToLittleEndian()
imageReader.SetDataExtent(0, 63, 0, 63, 1, 93)
imageReader.SetDataSpacing(3.2, 3.2, 1.5)
imageReader.SetFilePrefix(VTK_DATA_ROOT + '/Data/headsq/quarter')
imageReader.Update()

# Add direction to the image here since it isn't
# yet supported in vtkImageReader
direction = [1, 0, 0, 0, -1, 0, 0, 0, -1]
image = imageReader.GetOutput()
image.SetDirectionMatrix(direction)

# extract to reduce extents of grid
extract = vtkExtractVOI()
extract.SetInputData(image)
extract.SetVOI(0, 63, 0, 63, 0, 45)
extract.Update()

# write just a piece (extracted piece) as well as the whole thing
idWriter = vtkXMLImageDataWriter()
idWriter.SetFileName(file0)
idWriter.SetDataModeToAscii()
idWriter.SetInputData(extract.GetOutput())
idWriter.Write()

idWriter.SetFileName(file1)
idWriter.SetDataModeToAppended()
idWriter.SetInputData(image)
idWriter.SetNumberOfPieces(2)
idWriter.Write()

idWriter.SetFileName(file2)
idWriter.SetDataModeToBinary()
idWriter.SetWriteExtent(1, 31, 4, 63, 12, 92)
idWriter.Write()

# read the extracted grid
reader = vtkXMLImageDataReader()
reader.SetFileName(file0)
reader.WholeSlicesOff()
reader.Update()

id0 = vtkImageData()
id0.DeepCopy(reader.GetOutput())
cF0 = vtkContourFilter()
cF0.SetInputData(id0)
cF0.SetValue(0, 500)

mapper0 = vtkPolyDataMapper()
mapper0.SetInputConnection(cF0.GetOutputPort())
mapper0.ScalarVisibilityOff()

actor0 = vtkActor()
actor0.SetMapper(mapper0)
actor0.SetPosition(180, -60, 0)


# read the whole image
reader.SetFileName(file1)
reader.WholeSlicesOn()
reader.Update()

readDirection = reader.GetOutput().GetDirectionMatrix()
assert readDirection.GetElement(0,0) == direction[0]
assert readDirection.GetElement(0,1) == direction[1]
assert readDirection.GetElement(0,2) == direction[2]
assert readDirection.GetElement(1,0) == direction[3]
assert readDirection.GetElement(1,1) == direction[4]
assert readDirection.GetElement(1,2) == direction[5]
assert readDirection.GetElement(2,0) == direction[6]
assert readDirection.GetElement(2,1) == direction[7]
assert readDirection.GetElement(2,2) == direction[8]

id1 = vtkImageData()
id1.DeepCopy(reader.GetOutput())
cF1 = vtkContourFilter()
cF1.SetInputData(id1)
cF1.SetValue(0, 500)

mapper1 = vtkPolyDataMapper()
mapper1.SetInputConnection(cF1.GetOutputPort())
mapper1.ScalarVisibilityOff()

actor1 = vtkActor()
actor1.SetMapper(mapper1)
actor1.SetOrientation(90, 0, 0)


# read the partially written image
reader.SetFileName(file2)
reader.Update()

cF2 = vtkContourFilter()
cF2.SetInputConnection(reader.GetOutputPort())
cF2.SetValue(0, 500)

mapper2 = vtkPolyDataMapper()
mapper2.SetInputConnection(cF2.GetOutputPort())
mapper2.ScalarVisibilityOff()

actor2 = vtkActor()
actor2.SetMapper(mapper2)
actor2.SetOrientation(0, -90, 0)
actor2.SetPosition(180, -30, 0)

# Create the RenderWindow, Renderer and both Actors
#
ren = vtkRenderer()
renWin = vtkRenderWindow()
renWin.AddRenderer(ren)
iren = vtkRenderWindowInteractor()
iren.SetRenderWindow(renWin)

# Add the actors to the renderer, set the background and size
#
ren.AddActor(actor0)
ren.AddActor(actor1)
ren.AddActor(actor2)

renWin.SetSize(300, 300)
renWin.Render()

os.remove(file0)
os.remove(file1)
os.remove(file2)
