#include "Plot3DParser.h"
/*////////////////////////////////////////////////////////////////////////////
Plot3DParser.cpp

Parse a Plot3D file.

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
#include <cassert>
#include <string>
#include <fstream>

#include "FortranSequentialInputStream.h"

HBTK::Plot3D::Plot3DParser::Plot3DParser()
	: single_block(false),
	parse_as_binary(true),
	number_of_dimensions(-1)
{
}


HBTK::Plot3D::Plot3DParser::~Plot3DParser()
{
}


void HBTK::Plot3D::Plot3DParser::add_2D_block_function(std::function<bool(HBTK::StructuredMeshBlock2D)> func)
{
	assert(func);
	m_mesh_2d_functions.push_back(func);
	return;
}


void HBTK::Plot3D::Plot3DParser::add_3D_block_function(std::function<bool(HBTK::StructuredMeshBlock3D)> func)
{
	assert(func);
	m_mesh_3d_functions.push_back(func);
	return;
}


void HBTK::Plot3D::Plot3DParser::main_parser(std::ifstream & input_stream, std::ostream & error_stream)
{
	if (!input_stream) { throw - 1; }
	if (!error_stream) { throw - 1; }
	assert((number_of_dimensions == 2) || (number_of_dimensions == 3));

	if (number_of_dimensions == 2) {
		parse_2d(input_stream, error_stream);
	}
	else if (number_of_dimensions == 3) {
		parse_3d(input_stream, error_stream);
	}
	else {
		assert(false);
	}
	return;
}


void HBTK::Plot3D::Plot3DParser::parse_2d(std::ifstream & input_stream, std::ostream & error_stream)
{
	if (parse_as_binary) {
		parse_binary(input_stream, error_stream, 2);
	}
	else {
		parse_ascii(input_stream, error_stream, 2);
	}
}

void HBTK::Plot3D::Plot3DParser::parse_3d(std::ifstream & input_stream, std::ostream & error_stream)
{
	if (parse_as_binary) {
		parse_binary(input_stream, error_stream, 3);
	}
	else {
		parse_ascii(input_stream, error_stream, 3);
	}
}


void HBTK::Plot3D::Plot3DParser::parse_ascii(std::ifstream & input_stream, std::ostream & error_stream, int dimensions)
{
	assert(dimensions <= 3);
	assert(dimensions >= 2);
	int number_of_blocks;
	std::vector<std::vector<int>> extents;
	extents.resize(dimensions);
	std::string this_line;
	int line_number = 1;

	try {
		if (single_block) {
			number_of_blocks = 1;
		}
		else {
			std::getline(input_stream, this_line);
			line_number++;
			number_of_blocks = std::atoi(this_line.data());
		}

		for (auto &vect : extents) { vect.resize(number_of_blocks); }

		for (int n = 0; n < number_of_blocks; n++) {
			std::getline(input_stream, this_line);
			line_number++;
			auto strings = tokenise(this_line);
			if ((int)strings.size() < dimensions) throw line_number;
			for (int m = 0; m < dimensions; m++) {
				extents[m][n] = std::stoi(strings[m]);
			}
		}

		for (int n = 0; n < number_of_blocks; n++) {
			HBTK::StructuredMeshBlock3D mesh;
			int i_ext = extents[0][n];
			int j_ext = extents[1][n];
			int k_ext = (dimensions == 3 ? extents[2][n] : 1);
			mesh.set_extent({ i_ext, j_ext, k_ext } );

			auto read_bin = [&](int i, int j, int k, std::ifstream & input, int xyz_idx) {
				double tmp_val = 0;
				input_stream >> tmp_val;
				auto coord = mesh.coord({ i, j, k });
				coord[xyz_idx] = tmp_val;
				mesh.set_coord({ i, j, k }, coord);
			};

			apply_function_to_input_array(i_ext, j_ext, k_ext,
				[&](int i, int j, int k, std::ifstream & input) { read_bin(i, j, k, input, 0); },
				input_stream);
			apply_function_to_input_array(i_ext, j_ext, k_ext,
				[&](int i, int j, int k, std::ifstream & input) { read_bin(i, j, k, input, 1); },
				input_stream);

			if (dimensions == 3) {
				apply_function_to_input_array(i_ext, j_ext, k_ext,
					[&](int i, int j, int k, std::ifstream & input) { read_bin(i, j, k, input, 2); },
					input_stream);
				for (auto & function : m_mesh_3d_functions) {
					if (!function(mesh)) break;
				}
			}
			else {
				HBTK::StructuredMeshBlock2D mesh2d;
				mesh2d.set_extent({ i_ext, j_ext });
				for (int i = 0; i < i_ext; i++) {
					for (int j = 0; j < j_ext; j++) {
						auto coord = mesh.coord({ i, j, 0 });
						mesh2d.coord({ i, j }) = { coord[0], coord[1] };
					}
				}
				for (auto & function : m_mesh_2d_functions) {
					if (!function(mesh2d)) break;
				}
			}
		} // End iteration over blocks.
	}
	catch (...) { throw line_number; }
}

void HBTK::Plot3D::Plot3DParser::parse_binary(std::ifstream & input_stream, std::ostream & error_stream, int dimensions)
{
	assert(dimensions > 1);
	assert(dimensions <= 3);
	HBTK::FortranSequentialInputStream fortran_input;
	int number_of_blocks;
	std::vector<std::vector<int>> extents;
	extents.resize(dimensions);
#pragma pack(1)
	struct double_buffer {
		double value;
	} double_buffer;
#pragma pack(1)
	struct int_buffer {
		int value;
	} int_buffer;

	if (single_block) {
		number_of_blocks = 1;
	}
	else {
		fortran_input.record_open(input_stream);
		unpack_binary_to_struct(input_stream, int_buffer);
		number_of_blocks = int_buffer.value;
		fortran_input.record_close(input_stream);
		if (number_of_blocks < 1) throw - 1;
	}
	for (auto &extent : extents) { extent.resize(number_of_blocks); }

	fortran_input.record_open(input_stream);
	for (int n = 0; n < number_of_blocks; n++) {
		for (int m = 0; m < dimensions; m++) {
			unpack_binary_to_struct(input_stream, int_buffer);
			extents[m][n] = int_buffer.value;
		}
	}
	fortran_input.record_close(input_stream);

	try {
		for (int n = 0; n < number_of_blocks; n++) {
			HBTK::StructuredMeshBlock3D mesh;
			int i_ext = extents[0][n];
			int j_ext = extents[1][n];
			int k_ext = (dimensions == 3 ? extents[2][n] : 1);
			mesh.set_extent({ i_ext, j_ext, k_ext });


			auto read_bin = [&](int i, int j, int k, std::ifstream & input, int xyz_idx) {
				unpack_binary_to_struct(input_stream, double_buffer);
				auto coord = mesh.coord({ i, j, k });
				coord[xyz_idx] = double_buffer.value;
				mesh.set_coord({ i, j, k }, coord);
			};


			fortran_input.record_open(input_stream);
			apply_function_to_input_array(i_ext, j_ext, k_ext,
				[&](int i, int j, int k, std::ifstream & input) { read_bin(i, j, k, input, 0); },
				input_stream);
			apply_function_to_input_array(i_ext, j_ext, k_ext,
				[&](int i, int j, int k, std::ifstream & input) { read_bin(i, j, k, input, 1); },
				input_stream);

			if (dimensions == 3) {
				apply_function_to_input_array(i_ext, j_ext, k_ext,
					[&](int i, int j, int k, std::ifstream & input) { read_bin(i, j, k, input, 2); },
					input_stream);
				for (auto & function : m_mesh_3d_functions) {
					if (!function(mesh)) break;
				}
			}
			else {
				HBTK::StructuredMeshBlock2D mesh2d;
				mesh2d.set_extent({ i_ext, j_ext });
				for (int i = 0; i < i_ext; i++) {
					for (int j = 0; j < j_ext; j++) {
						auto coord = mesh.coord({ i, j, 0 });
						mesh2d.coord({ i, j }) = { coord[0], coord[1] };
					}
				}
				for (auto & function : m_mesh_2d_functions) {
					if (!function(mesh2d)) break;
				}
			}
			fortran_input.record_close(input_stream);
		} // End For over mesh blocks
	} // End try
	catch(...) { throw 1; }

	return;
}

void HBTK::Plot3D::Plot3DParser::apply_function_to_input_array(int i_ext, int j_ext, int k_ext, 
	std::function<void(int, int, int, std::ifstream&)> func, std::ifstream & input_stream)
{
	for (int k = 0; k < k_ext; k++) {
		for (int j = 0; j < j_ext; j++) {
			for (int i = 0; i < i_ext; i++) {
				func(i, j, k, input_stream);
			}
		}
	}
}


