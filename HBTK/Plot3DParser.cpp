#include "stdafx.h"
#include "Plot3DParser.h"

#include <cassert>
#include <string>
#include <fstream>

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


void HBTK::Plot3D::Plot3DParser::main_parser(std::ifstream & input_stream, std::ofstream & error_stream)
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


void HBTK::Plot3D::Plot3DParser::parse_2d(std::ifstream & input_stream, std::ofstream & error_stream)
{
	if (parse_as_binary) {
		parse_2d_binary(input_stream, error_stream);
	}
	else {
		parse_ascii(input_stream, error_stream, 2);
	}
}

void HBTK::Plot3D::Plot3DParser::parse_3d(std::ifstream & input_stream, std::ofstream & error_stream)
{
	if (parse_as_binary) {
		assert(false);
	}
	else {
		parse_ascii(input_stream, error_stream, 3);
	}
}


void HBTK::Plot3D::Plot3DParser::parse_2d_binary(std::ifstream & input_stream, std::ofstream & error_stream)
{
	int number_of_blocks;
	std::vector<int> i_extent, j_extent;
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
		unpack_binary_to_struct(input_stream, int_buffer);
		number_of_blocks = int_buffer.value;
		if (number_of_blocks < 1) throw -1;
	}
	i_extent.resize(number_of_blocks);
	j_extent.resize(number_of_blocks);

	for (int n = 0; n < number_of_blocks; n++) {
		unpack_binary_to_struct(input_stream, int_buffer);
		i_extent[n] = int_buffer.value;
		unpack_binary_to_struct(input_stream, int_buffer);
		j_extent[n] = int_buffer.value;
	}

	for (int n = 0; n < number_of_blocks; n++) {
		HBTK::StructuredMeshBlock2D mesh;
		mesh.set_extent(i_extent[n], j_extent[n]);
		for (int j = 0; j < j_extent[n]; j++) {
			for (int i = 0; i < i_extent[n]; i++) {
				unpack_binary_to_struct(input_stream, double_buffer);
				std::get<0>(mesh.coord(i, j)) = double_buffer.value;
			}
		}
		for (int j = 0; j < j_extent[n]; j++) {
			for (int i = 0; i < i_extent[n]; i++) {
				unpack_binary_to_struct(input_stream, double_buffer);
				std::get<1>(mesh.coord(i, j)) = double_buffer.value;
			}
		}
		for (auto & function : m_mesh_2d_functions) {
			if (!function(mesh)) break;
		}
	}

	return;
}
/*
void HBTK::Plot3D::Plot3DParser::parse_2d_ascii(std::ifstream & input_stream, std::ofstream & error_stream)
{
	int number_of_blocks;
	std::vector<int> i_extent, j_extent;
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

		i_extent.resize(number_of_blocks);
		j_extent.resize(number_of_blocks);

		for (int n = 0; n < number_of_blocks; n++) {
			std::getline(input_stream, this_line);
			line_number++;
			auto strings = tokenise(this_line);
			if (strings.size() < 2) throw line_number;
			i_extent[n] = std::stoi(strings[0]);
			j_extent[n] = std::stoi(strings[1]);
		}
		
		for (int n = 0; n < number_of_blocks; n++) {
			HBTK::StructuredMeshBlock2D mesh;
			mesh.set_extent(i_extent[n], j_extent[n]);
			for (int j = 0; j < j_extent[n]; j++) {
				for (int i = 0; i < i_extent[n]; i++) {
					std::getline(input_stream, this_line);
					line_number++;
					auto coord = mesh.coord(i, j);
					std::get<0>(coord) = std::stod(this_line);
				}
			}
			for (int j = 0; j < j_extent[n]; j++) {
				for (int i = 0; i < i_extent[n]; i++) {
					std::getline(input_stream, this_line);
					line_number++;
					auto coord = mesh.coord(i, j);
					std::get<1>(coord) = std::stod(this_line);
				}
			}

			for (auto & function : m_mesh_2d_functions) {
				if (!function(mesh)) break;
			}
		} // End iteration over blocks.
	}
	catch (...) { throw line_number; }
}
*/
void HBTK::Plot3D::Plot3DParser::parse_ascii(std::ifstream & input_stream, std::ofstream & error_stream, int dimensions)
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
			mesh.set_extent(i_ext, j_ext, k_ext);

			for (int k = 0; k < k_ext; k++) {
				for (int j = 0; j < j_ext; j++) {
					for (int i = 0; i < i_ext; i++) {
						std::getline(input_stream, this_line);
						line_number++;
						auto coord = mesh.coord(i, j, k);
						std::get<0>(coord) = std::stod(this_line);
					}
				}
			}
			for (int k = 0; k < k_ext; k++) {
				for (int j = 0; j < j_ext; j++) {
					for (int i = 0; i < i_ext; i++) {
						std::getline(input_stream, this_line);
						line_number++;
						auto coord = mesh.coord(i, j, k);
						std::get<1>(coord) = std::stod(this_line);
					}
				}
			}
			if (dimensions == 3) {
				for (int k = 0; k < k_ext; k++) {
					for (int j = 0; j < j_ext; j++) {
						for (int i = 0; i < i_ext; i++) {
							std::getline(input_stream, this_line);
							line_number++;
							auto coord = mesh.coord(i, j, k);
							std::get<1>(coord) = std::stod(this_line);
						}
					}
				}
				for (auto & function : m_mesh_3d_functions) {
					if (!function(mesh)) break;
				}
			}
			else {
				HBTK::StructuredMeshBlock2D mesh2d;
				mesh2d.set_extent(i_ext, j_ext);
				for (int i = 0; i < i_ext; i++) {
					for (int j = 0; j < j_ext; j++) {
						double x, y, z;
						std::tie(x, y, z) = mesh.coord(i, j, 0);
						mesh2d.coord(i, j) = std::tie(x, y);
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
