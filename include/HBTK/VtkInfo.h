#pragma once
/*////////////////////////////////////////////////////////////////////////////
VtkInfo.h

Information on Vtk elements

Copyright 2018 HJA Bird

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/////////////////////////////////////////////////////////////////////////////

#include <string>

#include "CartesianVector.h"
#include "VtkCellType.h"

namespace HBTK {
	namespace Vtk {
		// HBTK <-> VTK Types:
		// VtkInt32 -> int
		// VtkScalar -> double
		// VtkVector -> CartesianVector3D

		// Returns a string describing the element given by ele_id
		const std::string element_name(int ele_id);
		const std::string element_name(CellType ele_id);

		// Returns the number of nodes of element given by ele_id.
		// Returns -2 for invalid / unknown ele_id, and -1 for 
		// variable number of nodes.
		const int element_node_count(int ele_id);
		const int element_node_count(CellType ele_id);

		// Returns the number of dimensions of an element given by ele_id
		const int element_dimensions(int ele_id);
		const int element_dimensions(CellType ele_id);

		// Retuns the equivalent id number for a GMSH element. 
		// Conveniently, node ordering is the same.
		// Returns -1 for no equavalent.
		const int to_gmsh_element_id(int ele_id);
		const int to_gmsh_element_id(CellType ele_id);
	}
}
