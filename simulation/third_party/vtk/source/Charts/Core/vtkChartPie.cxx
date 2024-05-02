// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#include "vtkChartPie.h"

#include "vtkObjectFactory.h"

#include "vtkContext2D.h"
#include "vtkContextMouseEvent.h"
#include "vtkContextScene.h"
#include "vtkPoints2D.h"
#include "vtkTransform2D.h"

#include "vtkPlotPie.h"

#include "vtkChartLegend.h"
#include "vtkTooltipItem.h"

#include <sstream>

VTK_ABI_NAMESPACE_BEGIN
class vtkChartPiePrivate
{
public:
  vtkChartPiePrivate() = default;

  vtkSmartPointer<vtkPlotPie> Plot;
};

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkChartPie);

//------------------------------------------------------------------------------
vtkChartPie::vtkChartPie()
{
  this->Legend = vtkChartLegend::New();
  this->Legend->SetChart(this);
  this->Legend->SetVisible(false);
  this->AddItem(this->Legend);
  this->Legend->Delete();

  this->Tooltip = vtkTooltipItem::New();
  this->Tooltip->SetVisible(false);

  this->Private = new vtkChartPiePrivate();
}

//------------------------------------------------------------------------------
vtkChartPie::~vtkChartPie()
{
  this->Tooltip->Delete();
  delete this->Private;
}

//------------------------------------------------------------------------------
void vtkChartPie::Update()
{
  if (this->Private->Plot && this->Private->Plot->GetVisible())
  {
    this->Private->Plot->Update();
  }

  this->Legend->Update();
  this->Legend->SetVisible(this->ShowLegend);
}

//------------------------------------------------------------------------------
bool vtkChartPie::Paint(vtkContext2D* painter)
{
  // This is where everything should be drawn, or dispatched to other methods.
  vtkDebugMacro(<< "Paint event called.");

  int geometry[] = { this->GetScene()->GetSceneWidth(), this->GetScene()->GetSceneHeight() };
  if (geometry[0] == 0 || geometry[1] == 0 || !this->Visible)
  {
    // The geometry of the chart must be valid before anything can be drawn
    return false;
  }

  this->Update();

  if (this->LayoutStrategy == vtkChart::FILL_SCENE &&
    (geometry[0] != this->Geometry[0] || geometry[1] != this->Geometry[1]))
  {
    this->SetSize(vtkRectf(0.0, 0.0, geometry[0], geometry[1]));
  }

  vtkVector2i tileScale = this->Scene->GetLogicalTileScale();
  this->SetBorders(
    20 * tileScale.GetX(), 20 * tileScale.GetY(), 20 * tileScale.GetX(), 20 * tileScale.GetY());

  // Put the legend in the top corner of the chart
  vtkRectf legendRect = this->Legend->GetBoundingRect(painter);
  this->Legend->SetPoint(
    this->Point2[0] - legendRect.GetWidth(), this->Point2[1] - legendRect.GetHeight());

  // Set the dimensions of the Plot
  if (this->Private->Plot)
  {
    this->Private->Plot->SetDimensions(this->Size.GetX() + 20, this->Size.GetY() + 20,
      this->Geometry[0] - 40, this->Geometry[1] - 40);
  }

  this->PaintChildren(painter);

  vtkPoints2D* rect = vtkPoints2D::New();
  rect->InsertNextPoint(this->Point1[0], this->Point2[1]);
  rect->InsertNextPoint(this->Point2[0] - this->Point1[0], 10);
  painter->ApplyTextProp(this->TitleProperties);
  painter->DrawStringRect(rect, this->Title);
  rect->Delete();

  this->Tooltip->Paint(painter);

  return true;
}

//------------------------------------------------------------------------------
void vtkChartPie::SetScene(vtkContextScene* scene)
{
  this->vtkAbstractContextItem::SetScene(scene);
  this->Tooltip->SetScene(scene);
}

//------------------------------------------------------------------------------
vtkPlot* vtkChartPie::AddPlot(int /* type */)
{
  if (!this->Private->Plot)
  {
    this->Private->Plot = vtkSmartPointer<vtkPlotPie>::New();
    this->AddItem(this->Private->Plot);
    // Ensure legend remains on top
    this->Raise(this->GetItemIndex(this->Legend));
  }
  return this->Private->Plot;
}

//------------------------------------------------------------------------------
void vtkChartPie::SetPlot(vtkPlotPie* plot)
{
  if (this->Private->Plot)
  {
    this->RemoveItem(this->Private->Plot);
  }
  this->Private->Plot = plot;
  this->AddItem(this->Private->Plot);
  // Ensure legend remains on top
  this->Raise(this->GetItemIndex(this->Legend));
  this->Modified();
}

//------------------------------------------------------------------------------
vtkPlot* vtkChartPie::GetPlot(vtkIdType index)
{
  if (index == 0)
  {
    return this->Private->Plot;
  }

  return nullptr;
}

//------------------------------------------------------------------------------
vtkIdType vtkChartPie::GetNumberOfPlots()
{
  if (this->Private->Plot)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

//------------------------------------------------------------------------------
void vtkChartPie::SetShowLegend(bool visible)
{
  this->vtkChart::SetShowLegend(visible);
  this->Legend->SetVisible(visible);
}

//------------------------------------------------------------------------------
vtkChartLegend* vtkChartPie::GetLegend()
{
  return this->Legend;
}

//------------------------------------------------------------------------------
bool vtkChartPie::Hit(const vtkContextMouseEvent& mouse)
{
  vtkVector2f pos(mouse.GetScenePos());
  return pos[0] > this->Point1[0] && pos[0] < this->Point2[0] && pos[1] > this->Point1[1] &&
    pos[1] < this->Point2[1];
}

//------------------------------------------------------------------------------
bool vtkChartPie::MouseEnterEvent(const vtkContextMouseEvent&)
{
  return true;
}

//------------------------------------------------------------------------------
bool vtkChartPie::MouseMoveEvent(const vtkContextMouseEvent& mouse)
{
  if (mouse.GetButton() == vtkContextMouseEvent::NO_BUTTON)
  {
    this->Scene->SetDirty(true);
    this->Tooltip->SetVisible(this->LocatePointInPlots(mouse));
  }

  return true;
}

//------------------------------------------------------------------------------
bool vtkChartPie::MouseLeaveEvent(const vtkContextMouseEvent&)
{
  return true;
}

//------------------------------------------------------------------------------
bool vtkChartPie::MouseButtonPressEvent(const vtkContextMouseEvent& /*mouse*/)
{
  return true;
}

//------------------------------------------------------------------------------
bool vtkChartPie::MouseButtonReleaseEvent(const vtkContextMouseEvent& /*mouse*/)
{
  return true;
}

//------------------------------------------------------------------------------
bool vtkChartPie::MouseWheelEvent(const vtkContextMouseEvent&, int /*delta*/)
{
  return true;
}

bool vtkChartPie::LocatePointInPlots(const vtkContextMouseEvent& mouse)
{
  if (!this->Private->Plot || !this->Private->Plot->GetVisible())
  {
    return false;
  }
  else
  {
    int dimensions[4];
    vtkVector2f position(mouse.GetScenePos());
    vtkVector2f tolerance(5, 5);
    vtkVector2f plotPos(0, 0);
    this->Private->Plot->GetDimensions(dimensions);

    vtkVector2f pos(mouse.GetScenePos());
    if (pos[0] >= dimensions[0] && pos[0] <= dimensions[0] + dimensions[2] &&
      pos[1] >= dimensions[1] && pos[1] <= dimensions[1] + dimensions[3])
    {
      vtkIdType segmentId;
      int labelIndex =
        this->Private->Plot->GetNearestPoint(position, tolerance, &plotPos, &segmentId);
      if (labelIndex >= 0)
      {
        auto label = this->Private->Plot->GetLabel(labelIndex);
        std::ostringstream ostr;
        ostr << label << ": " << plotPos.GetY();
        this->Tooltip->SetText(ostr.str().c_str());
        this->Tooltip->SetPosition(mouse.GetScenePos()[0] + 2, mouse.GetScenePos()[1] + 2);
        return true;
      }
    }
  }
  return false;
}

//------------------------------------------------------------------------------
void vtkChartPie::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  if (this->Private->Plot)
  {
    os << indent << "Plot: " << endl;
    this->Private->Plot->PrintSelf(os, indent.GetNextIndent());
  }
}
VTK_ABI_NAMESPACE_END
