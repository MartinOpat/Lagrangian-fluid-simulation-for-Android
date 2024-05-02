// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkClosestNPointsStrategy.h"

#include "vtkCell.h"
#include "vtkGenericCell.h"
#include "vtkIdList.h"
#include "vtkObjectFactory.h"
#include "vtkPointLocator.h"
#include "vtkPointSet.h"
#include "vtkStaticPointLocator.h"

#include <set>

//------------------------------------------------------------------------------
VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(vtkClosestNPointsStrategy);

//------------------------------------------------------------------------------
vtkClosestNPointsStrategy::vtkClosestNPointsStrategy()
{
  this->ClosestNPoints = 9;
}

//------------------------------------------------------------------------------
vtkClosestNPointsStrategy::~vtkClosestNPointsStrategy() = default;

//------------------------------------------------------------------------------
vtkIdType vtkClosestNPointsStrategy::FindCell(double x[3], vtkCell* cell, vtkGenericCell* gencell,
  vtkIdType cellId, double tol2, int& subId, double pcoords[3], double* weights)
{
  // First try standard strategy which is reasonably fast
  vtkIdType foundCell =
    this->Superclass::FindCell(x, cell, gencell, cellId, tol2, subId, pcoords, weights);
  if (foundCell >= 0)
  {
    return foundCell;
  }

  // Couldn't find anything so try more time consuming strategy.  It is
  // possible that the closest point is not part of a cell containing the
  // query point (i.e., a hanging node situation). In this case, look for the
  // N closest points (beyond any coincident points identified
  // previously). Typically N=9 (somewhat arbitrary, empirical, based on 2:1
  // subdivision of hexahedral cells). Using large N affects performance but
  // produces better results.
  vtkIdType numPts = this->NearPointIds->GetNumberOfIds();
  this->PointLocator->FindClosestNPoints(numPts + this->ClosestNPoints, x, this->NearPointIds);
  numPts = this->NearPointIds->GetNumberOfIds();

  vtkIdType i, j, ptId, numCells;
  int ret;
  double closest[3], dist2;
  for (i = 0; i < numPts; ++i)
  {
    ptId = this->NearPointIds->GetId(i);
    this->PointSet->GetPointCells(ptId, this->CellIds);
    numCells = this->CellIds->GetNumberOfIds();
    for (j = 0; j < numCells; j++)
    {
      cellId = this->CellIds->GetId(j);
      if (!this->VisitedCells[cellId])
      {
        cell = this->SelectCell(this->PointSet, cellId, nullptr, gencell);
        ret = cell->EvaluatePosition(x, closest, subId, pcoords, dist2, weights);
        if (ret != -1 && dist2 <= tol2)
        {
          return cellId;
        }
        this->VisitedCells[cellId] = true;
        this->VisitedCellIds->InsertNextId(cellId);
      }
    }
  }

  return -1;
}

//------------------------------------------------------------------------------
void vtkClosestNPointsStrategy::CopyParameters(vtkFindCellStrategy* from)
{

  this->Superclass::CopyParameters(from);

  vtkClosestNPointsStrategy* strategy = vtkClosestNPointsStrategy::SafeDownCast(from);
  if (strategy)
  {
    this->ClosestNPoints = strategy->ClosestNPoints;
  }
}

//------------------------------------------------------------------------------
void vtkClosestNPointsStrategy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
VTK_ABI_NAMESPACE_END
