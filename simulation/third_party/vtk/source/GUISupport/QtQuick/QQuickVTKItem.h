// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-FileCopyrightText: Copyright (c) John Stone
// SPDX-License-Identifier: BSD-3-Clause
/**
 * @class QQuickVTKItem
 * @brief QQuickItem that manages a VTK rendering in the QML scenegraph
 */
#ifndef QQuickVTKItem_h
#define QQuickVTKItem_h

#include <QtQuick/QQuickItem>

#include <QtCore/QScopedPointer>

#include <functional>

#include "vtkGUISupportQtQuickModule.h" // for export macro
#include "vtkSmartPointer.h"

VTK_ABI_NAMESPACE_BEGIN

class vtkRenderWindow;
class vtkObject;

class QQuickVTKItemPrivate;
class VTKGUISUPPORTQTQUICK_EXPORT QQuickVTKItem : public QQuickItem
{
  Q_OBJECT

public:
  explicit QQuickVTKItem(QQuickItem* parent = nullptr);
  ~QQuickVTKItem() override;

  using vtkUserData = vtkSmartPointer<vtkObject>;

  /**
   * Set up the graphics surface format and api.
   *
   * This method sets the graphics API to OpenGLRhi and sets up the surface format for intermixed
   * VTK and QtQuick rendering. Use this method before instantiating a QApplication/QGuiApplication
   * typically this will be in your main.cpp file.
   */
  static void setGraphicsApi();

  ///@{
  /**
   * This is where the VTK initializiation should be done including creating a pipeline and
   * attaching it to the window
   *
   * \note All VTK objects are owned by and run on the QML render thread!!  This means you CAN NOT
   * touch any VTK state from any place other than in this method, in destroyingVTK or in your
   * dispatch_async() functions!!
   *
   * \note All VTK objects must be stored in the vtkUserData object returned from this method.
   *       They will be destroyed if the underlaying QSGNode (which must contain all VTK objects) is
   * destroyed.
   *
   * \note At any moment the QML SceneGraph can decide to delete the underlying QSGNode.
   *       If this happens this method will be called again to (re)create all VTK objects used by
   * this node
   *
   * \note Because of this you must be prepared to reset all the VTK state associated with any QML
   * property you have attached to this node during this execution of this method.
   *
   * \note At the time of this method execution, the GUI thread is blocked. Hence, it is safe to
   *       perform state synchronization between the GUI elements and the VTK classes here.
   *
   * \param renderWindow The VTK render window that creates this object's pixels for display
   *
   * \return The vtkUserData object associated with the VTK render window
   */
  virtual vtkUserData initializeVTK(vtkRenderWindow* renderWindow)
  {
    Q_UNUSED(renderWindow) return {};
  }
  ///@}

  ///@{
  /**
   * At any moment the QML SceneGraph can decide to delete the underlying QSGNode.
   * If this happens this method will be called before the VTK objects used by
   * this node are destroyed
   *
   * \note All VTK objects are owned by and run on the QML render thread!!  This means you CAN NOT
   * touch any VTK state from any place other than in this method, in initializeVTK, or in
   * your dispatch_async() functions!!
   *
   * \note At the time of this method execution, the GUI thread is blocked. Hence, it is safe to
   *       perform state synchronization between the GUI elements and the VTK classes here.
   *
   * \param renderWindow The VTK render window that creates this object's pixels for display
   *
   * \param userData The object associated with the VTK render window
   */
  virtual void destroyingVTK(vtkRenderWindow* renderWindow, vtkUserData userData)
  {
    Q_UNUSED(renderWindow);
    Q_UNUSED(userData);
  }
  ///@}

  ///@{
  /**
   * This is the function that enqueues an async command that will be executed just before VTK
   * renders
   *
   * \note All VTK objects are owned by and run on the QML render thread!!  This means you CAN NOT
   * touch any VTK state from any place other than in your function object passed as a parameter
   * here in initializeVTK(), or in destroyingVTK() methods!!
   *
   * \note This function most likely will only be called from the qt-gui-thread, eg. from a QML
   * button click-handler, but may be called from a dispatch_async() function as well.
   *
   * \note At the time of the async command execution, the GUI thread is blocked. Hence, it is safe
   * to perform state synchronization between the GUI elements and the VTK classes in the async
   * command function.
   *
   * \param f The async command you want to execute. It's parameters are 1.) The VTK render
   *  window that creates this object's pixels for display 2.) userData An optional User Data object
   *  associated with the VTK render window
   */
  void dispatch_async(std::function<void(vtkRenderWindow* renderWindow, vtkUserData userData)> f);
  ///@}

protected:
  void scheduleRender();

  bool event(QEvent*) override;

  QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*) override;
  bool isTextureProvider() const override;
  QSGTextureProvider* textureProvider() const override;
  void releaseResources() override;

private Q_SLOTS:
  void invalidateSceneGraph();

private: // NOLINT(readability-redundant-access-specifiers)
  Q_DISABLE_COPY(QQuickVTKItem)
  Q_DECLARE_PRIVATE_D(_d_ptr, QQuickVTKItem)
  QScopedPointer<QQuickVTKItemPrivate> _d_ptr;
};

VTK_ABI_NAMESPACE_END
#endif // QQuickVTKItem_h
