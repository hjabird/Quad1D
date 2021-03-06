#pragma once
/*////////////////////////////////////////////////////////////////////////////
CartesianFiniteLine.h

A straight finite line in Cartesian space.

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

#include "CartesianPoint.h"
#include "CartesianVector.h"

namespace HBTK {
	class CartesianLine3D {
	public:
		CartesianLine3D();

		// Initialise from the origin and a point on the line.
		// Direction becomes point_on_line - origin
		CartesianLine3D(const CartesianPoint3D & origin, const CartesianPoint3D & point_on_line);

		// Inialise from origin and direction.
		CartesianLine3D(const CartesianPoint3D & origin, const CartesianVector3D & direction);
		~CartesianLine3D();

		// Get a point on the line. position * direction from origin.
		CartesianPoint3D operator()(double position) const;
		CartesianPoint3D evaluate(double position) const;

		CartesianPoint3D & origin();
		const CartesianPoint3D & origin() const;
		CartesianVector3D & direction();
		const CartesianVector3D & direction() const;
		
		// Distance between this line and a point.
		double distance(const CartesianPoint3D & other) const;
		double distance(const CartesianLine3D & other) const;

		// Intersection - returns double corresponding to position between
		// start and end of line as 0-1.
		double intersection(const CartesianPoint3D & other) const;
		double intersection(const CartesianLine3D & other) const;

		bool operator==(const CartesianLine3D & other) const;
		bool operator!=(const CartesianLine3D & other) const;

	protected:
		CartesianPoint3D m_origin;
		CartesianVector3D m_direction;
	};

	class CartesianLine2D {
	public:
		CartesianLine2D();
		CartesianLine2D(const CartesianPoint2D & origin, const CartesianPoint2D & point_on_line);
		CartesianLine2D(const CartesianPoint2D & origin, const CartesianVector2D & direction);
		~CartesianLine2D();

		// Get a point on the line. position * direction from origin.
		CartesianPoint2D operator()(double position) const;
		CartesianPoint2D evaluate(double position) const;

		CartesianPoint2D & origin();
		const CartesianPoint2D & origin() const;
		CartesianVector2D & direction();
		const CartesianVector2D & direction() const;

		bool operator==(const CartesianLine2D & other) const;
		bool operator!=(const CartesianLine2D & other) const;

	protected:
		CartesianPoint2D m_origin;
		CartesianVector2D m_direction;
	};
}

