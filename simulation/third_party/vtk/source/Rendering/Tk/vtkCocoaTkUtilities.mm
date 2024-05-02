// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause

#ifndef MAC_OSX_TK
#define MAC_OSX_TK 1
#endif

#import "vtkCocoaMacOSXSDKCompatibility.h" // Needed to support old SDKs
#import <Cocoa/Cocoa.h>

// The order of these two headers is important
#import "tkMacOSXInt.h"
#import "vtkTk.h"

#import "vtkCocoaTkUtilities.h"
#import "vtkObjectFactory.h"
#import "vtkTcl.h"

vtkStandardNewMacro(vtkCocoaTkUtilities);

void vtkCocoaTkUtilities::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

// Getting an NSView from a Tk widget is strictly internal to Tk, so we
// have to duplicate that functionality here.  Hopefully this will be
// included in the distributed PrivateHeaders in later releases of Tk.
void* vtkCocoaTkUtilities::GetDrawableView(Tk_Window window)
{
  MacDrawable* macWin = reinterpret_cast<TkWindow*>(window)->privatePtr;

  if (!macWin)
  {
    return nil;
  }
  else if (macWin->toplevel && (macWin->toplevel->flags & TK_EMBEDDED))
  {
    // can't handle embedded window, but not sure if this will ever happen
    return nil;
  }
  else if (macWin->toplevel)
  {
    macWin = macWin->toplevel;
  }

  TkMacOSXMakeRealWindowExist(macWin->winPtr);
  NSView* result = macWin->view;

  return reinterpret_cast<void*>(result);
}
