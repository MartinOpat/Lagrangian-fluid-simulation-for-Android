#!/usr/bin/env python
# -*- coding: utf-8 -*-



import sys
from vtkmodules.vtkInteractionStyle import vtkInteractorStyleSwitch
from vtkmodules.vtkRenderingCore import (
    vtkRenderWindow,
    vtkRenderWindowInteractor,
    vtkRenderer,
)
import vtkmodules.vtkInteractionStyle
import vtkmodules.vtkRenderingFreeType
import vtkmodules.vtkRenderingOpenGL2
import vtkmodules.test.Testing
from vtkmodules.util.misc import vtkGetDataRoot
VTK_DATA_ROOT = vtkGetDataRoot()

'''
  Prevent .pyc files from being created.
  Stops the vtk source being polluted
  by .pyc files.
'''
sys.dont_write_bytecode = True

# Load base (spike and test)
import TestStyleBaseSpike
import TestStyleBase

class TestStyleJoystickCamera(vtkmodules.test.Testing.vtkTest):

    def testStyleJoystickCamera(self):

        ren = vtkRenderer()
        renWin = vtkRenderWindow()
        iRen = vtkRenderWindowInteractor()
        iRen.SetRenderWindow(renWin);

        testStyleBaseSpike = TestStyleBaseSpike.StyleBaseSpike(ren, renWin, iRen)

        # Set interactor style
        inStyle = vtkInteractorStyleSwitch()
        iRen.SetInteractorStyle(inStyle)

        # Switch to Joystick+Camera mode

        iRen.SetKeyEventInformation(0, 0, 'j', 0, '0')
        iRen.InvokeEvent("CharEvent")

        iRen.SetKeyEventInformation(0, 0, 'c', 0, '0')
        iRen.InvokeEvent("CharEvent")

        # Test style

        testStyleBase = TestStyleBase.TestStyleBase(ren)
        testStyleBase.test_style(inStyle.GetCurrentStyle())

        # render and interact with data

        img_file = "TestStyleJoystickCamera.png"
        vtkmodules.test.Testing.compareImage(iRen.GetRenderWindow(), vtkmodules.test.Testing.getAbsImagePath(img_file), threshold=25)
        vtkmodules.test.Testing.interact()

if __name__ == "__main__":
     vtkmodules.test.Testing.main([(TestStyleJoystickCamera, 'test')])
