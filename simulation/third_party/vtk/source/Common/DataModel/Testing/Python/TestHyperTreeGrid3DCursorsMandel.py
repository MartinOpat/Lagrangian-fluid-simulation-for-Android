#!/usr/bin/env python
from vtkmodules.vtkCommonCore import (
    vtkBitArray,
    vtkDoubleArray,
    vtkUnsignedCharArray,
)
from vtkmodules.vtkCommonDataModel import (
    vtkHyperTreeGrid,
    vtkHyperTreeGridNonOrientedCursor,
    vtkHyperTreeGridNonOrientedGeometryCursor,
    vtkHyperTreeGridNonOrientedMooreSuperCursor,
    vtkHyperTreeGridNonOrientedUnlimitedMooreSuperCursor,
    vtkHyperTreeGridNonOrientedVonNeumannSuperCursor,
)

ROOT_SPLIT = 10
TARGET_LEVEL = 8
TARGET_LEVEL = 6
CUT_OFF = TARGET_LEVEL

# -----------------------------------------------------------------------------
# Helpers
# -----------------------------------------------------------------------------


def mandelbrotTest(x, y, timeStep=0):
    count = 0
    cReal = float(x)
    cImag = float(y)
    zReal = 0.0
    zImag = float(timeStep) / 10.0

    zReal2 = zReal * zReal
    zImag2 = zImag * zImag
    v1 = zReal2 + zImag2
    while v1 < 4.0 and count < 100:
        zImag = 2.0 * zReal * zImag + cImag
        zReal = zReal2 - zImag2 + cReal
        zReal2 = zReal * zReal
        zImag2 = zImag * zImag
        count += 1
        v1 = zReal2 + zImag2

    return count == 100


def mandelbrotSide(bounds):
    count = 1
    if mandelbrotTest(bounds[0], bounds[2]):
        count += 1
    if mandelbrotTest(bounds[1], bounds[2]):
        count += 1
    if mandelbrotTest(bounds[0], bounds[3]):
        count += 1
    if mandelbrotTest(bounds[1], bounds[3]):
        count += 1
    return count


def shouldRefine(level, bounds):
    if level >= TARGET_LEVEL:
        return False
    origin = mandelbrotTest(bounds[0], bounds[2])
    originX = mandelbrotTest(bounds[1], bounds[2])
    originY = mandelbrotTest(bounds[0], bounds[3])
    originXY = mandelbrotTest(bounds[1], bounds[3])
    canRefine = bounds[4] < 0.01

    if canRefine:
        if origin and originX and originY and originXY:
            return False
        if not origin and not originX and not originY and not originXY:
            return False
        return True
    return False


def handleNode(cursor, sideArray, levelArray):
    cellBounds = [0, -1, 0, -1, 0, -1]
    cursor.GetBounds(cellBounds)

    # Add field
    idx = cursor.GetGlobalNodeIndex()
    side = mandelbrotSide(cellBounds)
    sideArray.InsertTuple1(idx, side)
    mask.InsertTuple1(idx, side < CUT_OFF)

    if cursor.IsLeaf():
        if shouldRefine(cursor.GetLevel(), cellBounds):
            cursor.SubdivideLeaf()
            handleNode(cursor, sideArray, mask)

    else:
        for childIdx in range(cursor.GetNumberOfChildren()):
            cursor.ToChild(childIdx)
            handleNode(cursor, sideArray, mask)
            cursor.ToParent()


# -----------------------------------------------------------------------------
# Create Simple HTG
# -----------------------------------------------------------------------------

htg = vtkHyperTreeGrid()
htg.Initialize()
htg.SetDimensions(
    [ROOT_SPLIT + 1, ROOT_SPLIT + 1, 2]
)  # nb cells, not nb points : GridCell [ROOT_SPLIT, ROOT_SPLIT, 1]
htg.SetBranchFactor(2)

sideArray = vtkUnsignedCharArray()
sideArray.SetName("sideArray")
sideArray.SetNumberOfValues(0)
sideArray.SetNumberOfComponents(1)
htg.GetCellData().AddArray(sideArray)

mask = vtkBitArray()
mask.SetName("mask")

# X[-1.75, 0.75]
xValues = vtkDoubleArray()
xValues.SetNumberOfValues(ROOT_SPLIT + 1)
for i in range(ROOT_SPLIT + 1):
    xValues.SetValue(i, -1.75 + float(i) * 0.25)
htg.SetXCoordinates(xValues)

# Y[-1.25, 1.25]
yValues = vtkDoubleArray()
yValues.SetNumberOfValues(ROOT_SPLIT + 1)
for i in range(ROOT_SPLIT + 1):
    yValues.SetValue(i, -1.25 + float(i) * 0.25)
htg.SetYCoordinates(yValues)

# Z[0, 0]
zValues = vtkDoubleArray()
zValues.SetNumberOfValues(2)
zValues.SetValue(0, 0)
zValues.SetValue(1, 0.25)
htg.SetZCoordinates(zValues)

geoCursor = vtkHyperTreeGridNonOrientedGeometryCursor()

offsetIndex = 0
for treeId in range(htg.GetMaxNumberOfTrees()):
    htg.InitializeNonOrientedGeometryCursor(geoCursor, treeId, True)
    geoCursor.SetGlobalIndexStart(offsetIndex)
    handleNode(geoCursor, sideArray, mask)
    offsetIndex += geoCursor.GetTree().GetNumberOfVertices()

print("offsetIndex: ", offsetIndex)

# Squeeze
htg.Squeeze()

# Select an active scalar field
htg.GetCellData().SetActiveScalars("sideArray")

# DataRange sideArray on PointData HTG
dataRange = htg.GetCellData().GetArray("sideArray").GetRange()
print("sideArray on PointData HTG:", dataRange)
print("HTG:", htg.GetNumberOfCells())


# Tests cursors... in Python
# maxDepth allows for testing even unlimited cursors
def recursive(cursor, maxDepth=-1):
    if cursor.IsLeaf() or maxDepth == 0:
        return 1
    nb = 0
    for ichild in range(cursor.GetNumberOfChildren()):
        cursor.ToChild(ichild)
        nb += recursive(cursor, maxDepth - 1)
        cursor.ToParent()
    return nb


def nonOrientedCursor(htg, expected):
    nb = 0
    cursor = vtkHyperTreeGridNonOrientedCursor()
    for treeId in range(htg.GetMaxNumberOfTrees()):
        htg.InitializeNonOrientedCursor(cursor, treeId)
        if not cursor.IsMasked():
            nb += recursive(cursor)
    print("nb: ", nb)
    if expected and nb != expected:
        print("ERROR Not corresponding expected value")
    return nb


expected = nonOrientedCursor(htg, None)


def nonOrientedGeometryCursor(htg, expected):
    nb = 0
    cursor = vtkHyperTreeGridNonOrientedGeometryCursor()
    for treeId in range(htg.GetMaxNumberOfTrees()):
        htg.InitializeNonOrientedGeometryCursor(cursor, treeId)
        if not cursor.IsMasked():
            nb += recursive(cursor)
    print("nb: ", nb)
    if expected and nb != expected:
        print("ERROR Not corresponding expected value")
    return nb


nonOrientedGeometryCursor(htg, expected)


def nonOrientedVonNeumannSuperCursor(htg, expected):
    nb = 0
    cursor = vtkHyperTreeGridNonOrientedVonNeumannSuperCursor()
    for treeId in range(htg.GetMaxNumberOfTrees()):
        htg.InitializeNonOrientedVonNeumannSuperCursor(cursor, treeId)
        if not cursor.IsMasked():
            nb += recursive(cursor)
    print("nb: ", nb)
    if expected and nb != expected:
        print("ERROR Not corresponding expected value")
    return nb


nonOrientedVonNeumannSuperCursor(htg, expected)


def nonOrientedMooreSuperCursor(htg, expected):
    nb = 0
    cursor = vtkHyperTreeGridNonOrientedMooreSuperCursor()
    for treeId in range(htg.GetMaxNumberOfTrees()):
        htg.InitializeNonOrientedMooreSuperCursor(cursor, treeId)
        if not cursor.IsMasked():
            nb += recursive(cursor)
    print("nb: ", nb)
    if expected and nb != expected:
        print("ERROR Not corresponding expected value")
    return nb


nonOrientedMooreSuperCursor(htg, expected)


def nonOrientedUnlimitedMooreSuperCursor(htg):
    nb = 0
    cursor = vtkHyperTreeGridNonOrientedUnlimitedMooreSuperCursor()
    for treeId in range(3):
        htg.InitializeNonOrientedUnlimitedMooreSuperCursor(cursor, treeId)
        nb += recursive(cursor, 3)
    if nb != (3 * 512):
        print("ERROR Unexpected value for unlimited cursor")
    return nb


nonOrientedUnlimitedMooreSuperCursor(htg)


# Test Find
findx = [
    -1.75 + float(ROOT_SPLIT) * 0.5 * 0.25,
    -1.75 + float(ROOT_SPLIT) * 0.5 * 0.25,
    0.11,
]
print("--------- FindNonOrientedGeometryCursor ---------")
cursor = htg.FindNonOrientedGeometryCursor(findx)
cursor.UnRegister(htg)

bbox = [
    -1.0,
    -1.0,
    -1.0,
    -1.0,
    -1.0,
    -1.0,
]
cursor.GetBounds(bbox)
print("bbox: ", bbox)

point = [
    -1.0,
    -1.0,
    -1.0,
]
cursor.GetPoint(point)
print("point: ", point[0], point[1], point[2])

print("IsLeaf: ", cursor.IsLeaf())
assert cursor.IsLeaf()

print(bbox[0], " <= ", findx[0], " and ", findx[0], " <= ", bbox[1])

if bbox[0] <= findx[0] and findx[0] <= bbox[1]:
    pass
else:
    assert bbox[0] <= findx[0] and findx[0] <= bbox[1]

print(bbox[2], " <= ", findx[1], " and ", findx[1], " <= ", bbox[3])

if bbox[2] <= findx[1] and findx[1] <= bbox[3]:
    pass
else:
    assert bbox[2] <= findx[1] and findx[1] <= bbox[3]

print(bbox[4], " <= ", findx[2], " and ", findx[2], " <= ", bbox[5])

if bbox[4] <= findx[2] and findx[2] <= bbox[5]:
    pass
else:
    assert bbox[4] <= findx[2] and findx[2] <= bbox[5]

# --- end of script --
