// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#include "vtkPlot.h"

#include "vtkArrayDispatch.h"
#include "vtkAxis.h"
#include "vtkBrush.h"
#include "vtkContextMapper2D.h"
#include "vtkContextMouseEvent.h"
#include "vtkContextTransform.h"
#include "vtkDataArrayMeta.h"
#include "vtkDataArrayRange.h"
#include "vtkDataObject.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPen.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkTransform2D.h"
#include <sstream>

VTK_ABI_NAMESPACE_BEGIN
vtkCxxSetObjectMacro(vtkPlot, XAxis, vtkAxis);
vtkCxxSetObjectMacro(vtkPlot, YAxis, vtkAxis);

//------------------------------------------------------------------------------
vtkPlot::vtkPlot()
  : ShiftScale(0.0, 0.0, 1.0, 1.0)
{
  this->Pen = vtkSmartPointer<vtkPen>::New();
  this->Pen->SetWidth(2.0);
  this->Brush = vtkSmartPointer<vtkBrush>::New();

  this->SelectionPen = vtkSmartPointer<vtkPen>::New();
  this->SelectionPen->SetColor(255, 50, 0, 150);
  this->SelectionPen->SetWidth(4.0);
  this->SelectionBrush = vtkSmartPointer<vtkBrush>::New();
  this->SelectionBrush->SetColor(255, 50, 0, 150);

  this->Labels = nullptr;
  this->UseIndexForXSeries = false;
  this->Data = vtkSmartPointer<vtkContextMapper2D>::New();
  this->Selectable = true;
  this->Selection = nullptr;
  this->XAxis = nullptr;
  this->YAxis = nullptr;

  this->TooltipDefaultLabelFormat = "%l: %x,  %y";
  this->TooltipNotation = vtkAxis::STANDARD_NOTATION;
  this->TooltipPrecision = 6;

  this->LegendVisibility = true;
}

//------------------------------------------------------------------------------
vtkPlot::~vtkPlot()
{
  if (this->Selection)
  {
    this->Selection->Delete();
    this->Selection = nullptr;
  }
  this->SetLabels(nullptr);
  this->SetXAxis(nullptr);
  this->SetYAxis(nullptr);
}

//------------------------------------------------------------------------------
void vtkPlot::Update()
{
  if (!this->Visible)
  {
    return;
  }
  // Check if we have an input
  if (!this->Data->GetInput())
  {
    vtkDebugMacro(<< "Update event called with no input table data set.");
    return;
  }

  bool data_updated = false;
  if (this->Data->GetMTime() > this->BuildTime ||
    this->Data->GetInput()->GetMTime() > this->BuildTime)
  {
    this->Data->Update();
    data_updated = true;
  }

  if (data_updated || this->CacheRequiresUpdate())
  {
    vtkDebugMacro(<< "Updating internal cached values.");
    this->UpdateCache();
    this->BuildTime.Modified();
  }
}

//------------------------------------------------------------------------------
bool vtkPlot::CacheRequiresUpdate()
{
  return (this->MTime > this->BuildTime) ||
    (this->XAxis && this->XAxis->GetMTime() > this->BuildTime) ||
    (this->YAxis && this->YAxis->GetMTime() > this->BuildTime);
}

//------------------------------------------------------------------------------
bool vtkPlot::PaintLegend(vtkContext2D*, const vtkRectf&, int)
{
  return false;
}

//------------------------------------------------------------------------------
vtkIdType vtkPlot::GetNearestPoint(const vtkVector2f& vtkNotUsed(point),
  const vtkVector2f& vtkNotUsed(tolerance), vtkVector2f* vtkNotUsed(location),
  vtkIdType* vtkNotUsed(segmentId))
{
  return -1;
}

//------------------------------------------------------------------------------
vtkStdString vtkPlot::GetTooltipLabel(const vtkVector2d& plotPos, vtkIdType seriesIndex, vtkIdType)
{
  std::string tooltipLabel;
  std::string& format =
    this->TooltipLabelFormat.empty() ? this->TooltipDefaultLabelFormat : this->TooltipLabelFormat;
  // Parse TooltipLabelFormat and build tooltipLabel
  bool escapeNext = false;
  for (size_t i = 0; i < format.length(); ++i)
  {
    if (escapeNext)
    {
      switch (format[i])
      {
        case 'x':
          tooltipLabel += this->GetNumber(plotPos.GetX(), this->XAxis);
          break;
        case 'y':
          tooltipLabel += this->GetNumber(plotPos.GetY(), this->YAxis);
          break;
        case 'i':
          if (this->IndexedLabels && seriesIndex >= 0 &&
            seriesIndex < this->IndexedLabels->GetNumberOfTuples())
          {
            tooltipLabel += this->IndexedLabels->GetValue(seriesIndex);
          }
          break;
        case 'l':
          // GetLabel() is GetLabel(0) in this implementation
          tooltipLabel += this->GetLabel();
          break;
        default: // If no match, insert the entire format tag
          tooltipLabel += "%";
          tooltipLabel += format[i];
          break;
      }
      escapeNext = false;
    }
    else
    {
      if (format[i] == '%')
      {
        escapeNext = true;
      }
      else
      {
        tooltipLabel += format[i];
      }
    }
  }
  return tooltipLabel;
}

//------------------------------------------------------------------------------
vtkStdString vtkPlot::GetNumber(double position, vtkAxis* axis)
{
  // Determine and format the X and Y position in the chart
  std::ostringstream ostr;
  ostr.imbue(std::locale::classic());
  ostr.precision(this->GetTooltipPrecision());

  if (this->GetTooltipNotation() == vtkAxis::SCIENTIFIC_NOTATION)
  {
    ostr.setf(ios::scientific, ios::floatfield);
  }
  else if (this->GetTooltipNotation() == vtkAxis::FIXED_NOTATION)
  {
    ostr.setf(ios::fixed, ios::floatfield);
  }

  if (axis && axis->GetLogScaleActive())
  {
    // If axes are set to logarithmic scale we need to convert the
    // axis value using 10^(axis value)
    ostr << pow(double(10.0), double(position));
  }
  else
  {
    ostr << position;
  }
  return ostr.str();
}

//------------------------------------------------------------------------------
bool vtkPlot::SelectPoints(const vtkVector2f&, const vtkVector2f&)
{
  if (this->Selection)
  {
    this->Selection->SetNumberOfTuples(0);
  }
  return false;
}

//------------------------------------------------------------------------------
bool vtkPlot::SelectPointsInPolygon(const vtkContextPolygon&)
{
  if (this->Selection)
  {
    this->Selection->SetNumberOfTuples(0);
  }
  return false;
}

//------------------------------------------------------------------------------
void vtkPlot::SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  this->Pen->SetColor(r, g, b, a);
}

//------------------------------------------------------------------------------
void vtkPlot::SetColorF(double r, double g, double b, double a)
{
  this->Pen->SetColorF(r, g, b, a);
}

//------------------------------------------------------------------------------
void vtkPlot::SetColorF(double r, double g, double b)
{
  this->Pen->SetColorF(r, g, b);
}

//------------------------------------------------------------------------------
void vtkPlot::GetColor(unsigned char rgb[3])
{
  this->Pen->GetColor(rgb);
}

//------------------------------------------------------------------------------
void vtkPlot::GetColorF(double rgb[3])
{
  this->Pen->GetColorF(rgb);
}

//------------------------------------------------------------------------------
void vtkPlot::SetWidth(float width)
{
  this->Pen->SetWidth(width);
}

//------------------------------------------------------------------------------
float vtkPlot::GetWidth()
{
  return this->Pen->GetWidth();
}

//------------------------------------------------------------------------------
void vtkPlot::SetPen(vtkPen* pen)
{
  if (this->Pen != pen)
  {
    this->Pen = pen;
    this->Modified();
  }
}

//------------------------------------------------------------------------------
vtkPen* vtkPlot::GetPen()
{
  return this->Pen;
}

//------------------------------------------------------------------------------
void vtkPlot::SetBrush(vtkBrush* brush)
{
  if (this->Brush != brush)
  {
    this->Brush = brush;
    this->Modified();
  }
}

//------------------------------------------------------------------------------
vtkBrush* vtkPlot::GetBrush()
{
  return this->Brush;
}

//------------------------------------------------------------------------------
void vtkPlot::SetSelectionPen(vtkPen* pen)
{
  if (this->SelectionPen != pen)
  {
    this->SelectionPen = pen;
    this->Modified();
  }
}

//------------------------------------------------------------------------------
vtkPen* vtkPlot::GetSelectionPen()
{
  return this->SelectionPen;
}

//------------------------------------------------------------------------------
void vtkPlot::SetSelectionBrush(vtkBrush* brush)
{
  if (this->SelectionBrush != brush)
  {
    this->SelectionBrush = brush;
    this->Modified();
  }
}

//------------------------------------------------------------------------------
vtkBrush* vtkPlot::GetSelectionBrush()
{
  return this->SelectionBrush;
}

//------------------------------------------------------------------------------
void vtkPlot::SetLabel(const vtkStdString& label)
{
  vtkNew<vtkStringArray> labels;
  labels->InsertNextValue(label);
  this->SetLabels(labels);
}

//------------------------------------------------------------------------------
vtkStdString vtkPlot::GetLabel()
{
  return this->GetLabel(0);
}

//------------------------------------------------------------------------------
void vtkPlot::SetLabels(vtkStringArray* labels)
{
  if (this->Labels == labels)
  {
    return;
  }

  this->Labels = labels;
  this->Modified();
}

//------------------------------------------------------------------------------
vtkStringArray* vtkPlot::GetLabels()
{
  // If the label string is empty, return the y column name
  if (this->Labels)
  {
    return this->Labels;
  }
  else if (this->AutoLabels)
  {
    return this->AutoLabels;
  }
  else if (this->Data->GetInput() &&
    this->Data->GetInformation()->Get(vtkAlgorithm::INPUT_ARRAYS_TO_PROCESS()) &&
    this->Data->GetInputArrayToProcess(1, this->Data->GetInput()))
  {
    this->AutoLabels = vtkSmartPointer<vtkStringArray>::New();
    this->AutoLabels->InsertNextValue(
      this->Data->GetInputArrayToProcess(1, this->Data->GetInput())->GetName());
    return this->AutoLabels;
  }
  else
  {
    return nullptr;
  }
}
//------------------------------------------------------------------------------
int vtkPlot::GetNumberOfLabels()
{
  vtkStringArray* labels = this->GetLabels();
  if (labels)
  {
    return labels->GetNumberOfValues();
  }
  else
  {
    return 0;
  }
}

//------------------------------------------------------------------------------
void vtkPlot::SetIndexedLabels(vtkStringArray* labels)
{
  if (this->IndexedLabels == labels)
  {
    return;
  }

  if (labels)
  {
    this->TooltipDefaultLabelFormat = "%i: %x,  %y";
  }
  else
  {
    this->TooltipDefaultLabelFormat = "%l: %x,  %y";
  }

  this->IndexedLabels = labels;
  this->Modified();
}

//------------------------------------------------------------------------------
vtkStringArray* vtkPlot::GetIndexedLabels()
{
  return this->IndexedLabels;
}

//------------------------------------------------------------------------------
vtkContextMapper2D* vtkPlot::GetData()
{
  return this->Data;
}

//------------------------------------------------------------------------------
void vtkPlot::SetTooltipLabelFormat(const vtkStdString& labelFormat)
{
  if (this->TooltipLabelFormat == labelFormat)
  {
    return;
  }

  this->TooltipLabelFormat = labelFormat;
  this->Modified();
}

//------------------------------------------------------------------------------
vtkStdString vtkPlot::GetTooltipLabelFormat()
{
  return this->TooltipLabelFormat;
}

//------------------------------------------------------------------------------
void vtkPlot::SetTooltipNotation(int notation)
{
  this->TooltipNotation = notation;
  this->Modified();
}

//------------------------------------------------------------------------------
int vtkPlot::GetTooltipNotation()
{
  return this->TooltipNotation;
}

//------------------------------------------------------------------------------
void vtkPlot::SetTooltipPrecision(int precision)
{
  this->TooltipPrecision = precision;
  this->Modified();
}

//------------------------------------------------------------------------------
int vtkPlot::GetTooltipPrecision()
{
  return this->TooltipPrecision;
}

//------------------------------------------------------------------------------
vtkStdString vtkPlot::GetLabel(vtkIdType index)
{
  vtkStringArray* labels = this->GetLabels();
  if (labels && index >= 0 && index < labels->GetNumberOfValues())
  {
    return labels->GetValue(index);
  }
  else
  {
    return {};
  }
}
//------------------------------------------------------------------------------
void vtkPlot::SetInputData(vtkTable* table)
{
  this->Data->SetInputData(table);
  this->AutoLabels = nullptr; // No longer valid
}

//------------------------------------------------------------------------------
void vtkPlot::SetInputData(
  vtkTable* table, const vtkStdString& xColumn, const vtkStdString& yColumn)
{
  vtkDebugMacro(<< "Setting input, X column = \"" << xColumn << "\", "
                << "Y column = \"" << yColumn << "\"");

  this->Data->SetInputData(table);
  this->Data->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_ROWS, xColumn.c_str());
  this->Data->SetInputArrayToProcess(
    1, 0, 0, vtkDataObject::FIELD_ASSOCIATION_ROWS, yColumn.c_str());
  this->AutoLabels = nullptr; // No longer valid
}

//------------------------------------------------------------------------------
void vtkPlot::SetInputData(vtkTable* table, vtkIdType xColumn, vtkIdType yColumn)
{
  this->SetInputData(table, table->GetColumnName(xColumn), table->GetColumnName(yColumn));
}

//------------------------------------------------------------------------------
void vtkPlot::SetInputConnection(vtkAlgorithmOutput* input)
{
  this->Data->SetInputConnection(0, input);
}

//------------------------------------------------------------------------------
vtkTable* vtkPlot::GetInput()
{
  return this->Data->GetInput();
}

//------------------------------------------------------------------------------
vtkAlgorithmOutput* vtkPlot::GetInputConnection()
{
  return this->Data->GetInputConnection(0, 0);
}

//------------------------------------------------------------------------------
void vtkPlot::SetInputArray(int index, const vtkStdString& name)
{
  this->Data->SetInputArrayToProcess(
    index, 0, 0, vtkDataObject::FIELD_ASSOCIATION_ROWS, name.c_str());
  this->AutoLabels = nullptr; // No longer valid
}

//------------------------------------------------------------------------------
void vtkPlot::SetSelection(vtkIdTypeArray* id)
{
  if (!this->GetSelectable())
  {
    return;
  }
  vtkSetObjectBodyMacro(Selection, vtkIdTypeArray, id);
}

//------------------------------------------------------------------------------
void vtkPlot::SetShiftScale(const vtkRectd& shiftScale)
{
  if (shiftScale != this->ShiftScale)
  {
    this->Modified();
    this->ShiftScale = shiftScale;
  }
}

//------------------------------------------------------------------------------
vtkRectd vtkPlot::GetShiftScale()
{
  return this->ShiftScale;
}

//------------------------------------------------------------------------------
void vtkPlot::SetProperty(const vtkStdString&, const vtkVariant&) {}

//------------------------------------------------------------------------------
vtkVariant vtkPlot::GetProperty(const vtkStdString&)
{
  return vtkVariant();
}

//------------------------------------------------------------------------------
void vtkPlot::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "LegendVisibility: " << this->LegendVisibility << endl;
}

//------------------------------------------------------------------------------
void vtkPlot::TransformScreenToData(const vtkVector2f& in, vtkVector2f& out)
{
  double tmp[2] = { in.GetX(), in.GetY() };

  this->TransformScreenToData(tmp[0], tmp[1], tmp[0], tmp[1]);

  out.Set(static_cast<float>(tmp[0]), static_cast<float>(tmp[1]));
}

//------------------------------------------------------------------------------
void vtkPlot::TransformDataToScreen(const vtkVector2f& in, vtkVector2f& out)
{
  double tmp[2] = { in.GetX(), in.GetY() };

  this->TransformDataToScreen(tmp[0], tmp[1], tmp[0], tmp[1]);

  out.Set(static_cast<float>(tmp[0]), static_cast<float>(tmp[1]));
}

//------------------------------------------------------------------------------
void vtkPlot::TransformScreenToData(double inX, double inY, double& outX, double& outY)
{
  // inverse shift/scale from screen space.
  const vtkRectd& ss = this->ShiftScale;
  outX = (inX / ss[2]) - ss[0];
  outY = (inY / ss[3]) - ss[1];

  const bool logX = this->GetXAxis() && this->GetXAxis()->GetLogScaleActive();
  const bool logY = this->GetYAxis() && this->GetYAxis()->GetLogScaleActive();

  if (logX)
  {
    outX = std::pow(10., outX);
  }
  if (logY)
  {
    outY = std::pow(10., outY);
  }
}

//------------------------------------------------------------------------------
void vtkPlot::TransformDataToScreen(double inX, double inY, double& outX, double& outY)
{
  outX = inX;
  outY = inY;

  const bool logX = this->GetXAxis() && this->GetXAxis()->GetLogScaleActive();
  const bool logY = this->GetYAxis() && this->GetYAxis()->GetLogScaleActive();

  if (logX)
  {
    outX = std::log10(outX);
  }
  if (logY)
  {
    outY = std::log10(outY);
  }

  // now, shift/scale to screen space.
  const vtkRectd& ss = this->ShiftScale;
  outX = (outX + ss[0]) * ss[2];
  outY = (outY + ss[1]) * ss[3];
}

//------------------------------------------------------------------------------
bool vtkPlot::ClampPos(double pos[2], double bounds[4])
{
  if (bounds[1] < bounds[0] || bounds[3] < bounds[2])
  {
    // bounds are not valid. Don't clamp.
    return false;
  }
  bool clamped = false;
  if (pos[0] < bounds[0] || vtkMath::IsNan(pos[0]))
  {
    pos[0] = bounds[0];
    clamped = true;
  }
  if (pos[0] > bounds[1])
  {
    pos[0] = bounds[1];
    clamped = true;
  }
  if (pos[1] < 0. || vtkMath::IsNan(pos[0]))
  {
    pos[1] = 0.;
    clamped = true;
  }
  if (pos[1] > 1.)
  {
    pos[1] = 1.;
    clamped = true;
  }
  return clamped;
}

//------------------------------------------------------------------------------
bool vtkPlot::ClampPos(double pos[2])
{
  double bounds[4];
  this->GetBounds(bounds);
  return vtkPlot::ClampPos(pos, bounds);
}

//------------------------------------------------------------------------------
bool vtkPlot::Hit(const vtkContextMouseEvent& mouse)
{
  if (!this->Interactive || !this->Visible)
  {
    return false;
  }
  double screenBounds[4];
  this->GetBounds(screenBounds);
  vtkContextTransform* transform = vtkContextTransform::SafeDownCast(this->Parent);
  vtkVector2f tol;
  constexpr int picking_zone_size = 5;
  if (transform != nullptr)
  {
    tol.SetX(std::fabs(
      picking_zone_size * (1.0 / transform->GetTransform()->GetMatrix()->GetElement(0, 0))));
    tol.SetY(std::fabs(
      picking_zone_size * (1.0 / transform->GetTransform()->GetMatrix()->GetElement(1, 1))));
  }
  vtkVector2f loc;
  vtkIdType segmentId;
  return this->GetNearestPoint(mouse.GetPos(), tol, &loc, &segmentId) >= 0;
}

namespace
{
struct FilterSelectedPoints
{
  template <typename ArrayType1, typename ArrayType2>
  void operator()(ArrayType1* points, ArrayType2* selectedPoints, vtkIdTypeArray* selectedIds)
  {
    auto pointsRange = vtk::DataArrayTupleRange(points);
    auto selectedPointsRange = vtk::DataArrayTupleRange(selectedPoints);
    const vtkIdType nSelected = selectedIds->GetNumberOfTuples();
    const vtkIdType* ids = selectedIds->GetPointer(0);
    assert(pointsRange.GetTupleSize() > 1);
    assert(selectedPointsRange.GetTupleSize() > 1);
    for (vtkIdType i = 0; i < nSelected; ++i)
    {
      const vtkIdType& id = ids[i];
      std::copy(pointsRange[id].cbegin(), pointsRange[id].cend(), selectedPointsRange[i].begin());
    }
  }
};
}

//------------------------------------------------------------------------------
void vtkPlot::FilterSelectedPoints(
  vtkDataArray* points, vtkDataArray* selectedPoints, vtkIdTypeArray* selectedIds)
{
  // resize selectedPoints
  selectedPoints->SetNumberOfComponents(points->GetNumberOfComponents());
  selectedPoints->SetNumberOfTuples(selectedIds->GetNumberOfTuples());
  // filter using selectedIds
  using DispatchT = vtkArrayDispatch::Dispatch2BySameValueType<vtkArrayDispatch::Reals>;
  ::FilterSelectedPoints worker;
  if (!DispatchT::Execute(points, selectedPoints, worker, selectedIds))
  {
    worker(points, selectedPoints, selectedIds);
  }
  selectedPoints->Modified();
}
VTK_ABI_NAMESPACE_END
