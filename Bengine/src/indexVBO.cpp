#include <iostream>

#include <glm.hpp>
#include <vector>
#include <map>
#include <string.h>

bool is_near(float v1, float v2) {
	return fabs(v1 - v2) < 0.01f;
}

bool getSimilarVertexIndex(
	std::vector<float> & in_vertex,
	std::vector<float> & out_vbo,
	unsigned short & result,
	int vertex_size
) {
	// Lame linear search
	for (unsigned int i = 0; i < out_vbo.size() / vertex_size; i++) {
		if (
			is_near(in_vertex[0], out_vbo[i * vertex_size]) &&
			is_near(in_vertex[1], out_vbo[i * vertex_size + 1]) &&
			is_near(in_vertex[2], out_vbo[i * vertex_size + 2]) &&
			is_near(in_vertex[3], out_vbo[i * vertex_size + 3]) &&
			is_near(in_vertex[4], out_vbo[i * vertex_size + 4]) &&
			is_near(in_vertex[5], out_vbo[i * vertex_size + 5]) &&
			is_near(in_vertex[6], out_vbo[i * vertex_size + 6]) &&
			is_near(in_vertex[7], out_vbo[i * vertex_size + 7])
			) {
			result = i;
			return true;
		}
	}
	// No other vertex could be used instead.
	// Looks like we'll have to add it to the VBO.
	return false;
}

void indexVBO(
	std::vector<float>& in_vbo,
	std::vector<unsigned short>& out_ebo,
	std::vector<float>& out_vbo,
	int vertex_size) {

	// For each input vertex
	for (unsigned int i = 0; i < in_vbo.size() / vertex_size; i++) {

		unsigned int vertex_start = i * vertex_size;
		unsigned int vertex_end = i * vertex_size + vertex_size;
		std::vector<float> vertex;

		for (unsigned int j = vertex_start; j < vertex_end; j++) {
			vertex.push_back(in_vbo[j]);
		}

		unsigned short index;
		bool found = getSimilarVertexIndex(vertex, out_vbo, index, vertex_size);

		if (found) { // A similar vertex is already in the VBO, use it instead !
			out_ebo.push_back(index);
		}
		else { // If not, it needs to be added in the output data.
			for (unsigned int j = 0; j < vertex.size(); j++)
				out_vbo.push_back(vertex[j]);

			unsigned short new_index = (unsigned short)out_vbo.size() / vertex_size - 1;
			out_ebo.push_back(new_index);
		}
	}
}