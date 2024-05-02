//VTK::System::Dec

// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
// this shader is used to implement lighting in the fragment shader
// it handles setting up the basic varying variables for the fragment shader

// all variables that represent positions or directions have a suffix
// indicating the coordinate system they are in. The possible values are
// MC - Model Coordinates
// WC - WC world coordinates
// VC - View Coordinates
// DC - Display Coordinates

in vec4 vertexMC;

// frag position in VC
//VTK::PositionVC::Dec

// optional normal declaration
//VTK::Normal::Dec

// Texture coordinates
//VTK::TCoord::Dec

// material property values
//VTK::Color::Dec

// camera and actor matrix values
//VTK::Camera::Dec

//VTK::Glyph::Dec

// clipping plane vars
//VTK::Clip::Dec

void main()
{
  //VTK::Glyph::Impl

  //VTK::Clip::Impl

  //VTK::Color::Impl

  //VTK::Normal::Impl

  //VTK::TCoord::Impl

  // frag position in VC
  //VTK::PositionVC::Impl
}
