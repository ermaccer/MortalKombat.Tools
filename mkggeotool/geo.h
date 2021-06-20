#pragma once

struct geo_header {
	char header[4]; //1.0v
	int  archiveSize;
	int  files;
	char pad[20] = {};
};

struct geo_entry {
	short unk;
	short models;
	int   offset; // - headersize
	char pad[24] = {};
};

struct geometry_info {
	int		unk;
	short	unk2;
	short   vertexes;
	char pad[24] = {};
};

struct vector3d {
	float x;
	float y;
	float z;
};


struct uv {
	float u;
	float v;
};

struct obj_v { float x, y, z; };
struct obj_uv { float u, v; };
struct obj_vn { float norm[3]; };
struct obj_face { int face[3]; };

struct group_info {
	int belong;
	std::string name;

	std::vector<obj_v> vVerts;
	std::vector<obj_uv> vMaps;
	std::vector<obj_vn> vNorm;
	std::vector<obj_face> vFaces;

};