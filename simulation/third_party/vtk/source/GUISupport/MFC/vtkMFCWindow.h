// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#ifndef vtkMFCWindow_h
#define vtkMFCWindow_h

#include "afxwin.h"
#include "vtkGUISupportMFCModule.h" // For export macro

VTK_ABI_NAMESPACE_BEGIN
class vtkWin32OpenGLRenderWindow;
class vtkRenderWindowInteractor;

//! class to display a VTK window in an MFC window
class VTKGUISUPPORTMFC_EXPORT vtkMFCWindow : public CWnd
{
public:
  //! constructor requires a parent
  vtkMFCWindow(CWnd* pcWnd);
  //! destructor
  virtual ~vtkMFCWindow();

#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

  //! draw to a device context
  void DrawDC(CDC* pDC);

  //! give an instance of a vtk render window to the mfc window
  virtual void SetRenderWindow(vtkWin32OpenGLRenderWindow*);
  //! get the render window
  virtual vtkWin32OpenGLRenderWindow* GetRenderWindow();
  //! get the interactor
  virtual vtkRenderWindowInteractor* GetInteractor();

protected:
  //! handle size events
  afx_msg void OnSize(UINT nType, int cx, int cy);
  //! handle paint events
  afx_msg void OnPaint();
  //! handle destroy events
  afx_msg void OnDestroy();
  //! don't clear background
  BOOL OnEraseBkgnd(CDC* pDC);

  afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
  afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  afx_msg void OnTimer(UINT_PTR nIDEvent);

  //! the vtk window
  vtkWin32OpenGLRenderWindow* pvtkWin32OpenGLRW;

  DECLARE_MESSAGE_MAP()
};

VTK_ABI_NAMESPACE_END
#endif

// VTK-HeaderTest-Exclude: vtkMFCWindow.h
