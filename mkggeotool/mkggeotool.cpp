#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include "geo.h"

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		std::cout << "MK Gold GEO Tool - Convert Models to OBJ\n"
			"Usage: mkggeotool <file>\n";
		return 1;
	}

	std::ifstream pFile(argv[1], std::ifstream::binary);

	if (!pFile)
	{
		std::cout << "ERROR: Could not open " << argv[1] << "!" << std::endl;
		return 1;
	}
	if (pFile)
	{
		geo_header geo;
		pFile.read((char*)&geo, sizeof(geo_header));

		if (!(geo.header[0] == '0' && geo.header[1] == '.' && geo.header[2] == '1' && geo.header[3] == 'v'))
		{
			std::cout << "ERROR: " << argv[1] << " is not a GEO archive!" << std::endl;
			return 1;
		}

		std::vector<geo_entry> Models;
		std::vector<int> vTempFaces;
		for (int i = 0; i < geo.files; i++)
		{
			geo_entry ent;
			pFile.read((char*)&ent, sizeof(geo_entry));
			Models.push_back(ent);
		}
		std::string name = argv[1] + std::string(".obj");
		std::ofstream obj(name, std::ofstream::binary);
		std::vector<group_info> vGroups;
		// processing model extraction
		for (int i = 0; i < geo.files; i++)
		{
			std::vector<obj_v> vVerts;
			std::vector<obj_uv> vMaps;
			std::vector<obj_vn> vNorm;
			std::vector<obj_face> vFaces;

			for (int a = 0; a < Models[i].models; a++)
			{
				group_info grp;

				std::string groupName = "G" + std::to_string(i) + "_" + std::to_string(a);

				grp.name = groupName;
				grp.belong = i;
				geometry_info info;
				pFile.read((char*)&info, sizeof(geometry_info));


				for (int x = 0; x < info.vertexes; x++)
				{
					obj_vn norm;
					pFile.read((char*)&norm, sizeof(obj_vn));
					vNorm.push_back(norm);

					obj_v vert;
					pFile.read((char*)&vert, sizeof(obj_v));
					vVerts.push_back(vert);
					
					obj_uv map;
					pFile.read((char*)&map, sizeof(obj_uv));
					vMaps.push_back(map);
					
					// no face data,  1 = 123, 2=  246 and so on
					vTempFaces.push_back(x);
				
				}

				for (int z = 0; z < vTempFaces.size() - 2; z++)
				{
					short f1, f2, f3;
					if (z & 1)
					{
						f1 = vTempFaces[z];
						f2 = vTempFaces[z + 2];
						f3 = vTempFaces[z + 1];
					}
					else
					{
						f1 = vTempFaces[z];
						f2 = vTempFaces[z + 1];
						f3 = vTempFaces[z + 2];
					}
					if (f1 == f2 || f2 == f3 || f1 == f3)
						continue;
					obj_face face = { f1 ,f2 ,f3 };
					vFaces.push_back(face);
				}


				vTempFaces.clear();
				grp.vFaces = vFaces;
				grp.vMaps = vMaps;
				grp.vNorm = vNorm;
				grp.vVerts = vVerts;

				vFaces.clear();
				vMaps.clear();
				vNorm.clear();
				vVerts.clear();

				vGroups.push_back(grp);
			}		
		}

		// convert to obj

		int baseFaceSize = 0;
		int baseVertSize = 0;
		obj << "; obj created using mkggeotool by ermaccer\n" << std::setprecision(4) << std::fixed;

		int todo = vGroups.size();

		for (int i = 0; i < vGroups.size(); i++)
		{
			todo--;
			std::cout << "Processing " << 100 - (todo * 100 / vGroups.size()) << "%\r";
		
			
			obj << "; " << i + 1 << " V:" << vGroups[i].vVerts.size() <<  std::endl;
			for (int z = 0; z < vGroups[i].vVerts.size(); z++)
			{
				obj << "v " << vGroups[i].vVerts[baseVertSize + z].x << " " << vGroups[i].vVerts[baseVertSize + z].y << " " << vGroups[i].vVerts[baseVertSize + z].z << std::endl;
			}
			for (int z = 0; z < vGroups[i].vVerts.size(); z++)
			{
				obj << "vn " << vGroups[i].vNorm[baseVertSize + z].norm[0] << " " << vGroups[i].vNorm[baseVertSize + z].norm[1] << " " << vGroups[i].vNorm[baseVertSize + z].norm[2] << std::endl;
			}
			for (int z = 0; z < vGroups[i].vVerts.size(); z++)
			{
				obj << "vt " << vGroups[i].vMaps[baseVertSize + z].u << " " << 1.0f - vGroups[i].vMaps[baseVertSize + z].v << std::endl;
			}
			
			obj << "g " << vGroups[i].name.c_str() << std::endl;

			for (int x = 0; x < vGroups[i].vFaces.size(); x++)
			{
				int temp[3];
				temp[0] = vGroups[i].vFaces[x].face[0] + baseFaceSize;
				temp[1] = vGroups[i].vFaces[x].face[1] + baseFaceSize;
				temp[2] = vGroups[i].vFaces[x].face[2] + baseFaceSize;


				obj << "f " << temp[0] + 1 << "/" << temp[0] + 1 << "/" << temp[0] + 1
					<< " "
					<< temp[1] + 1 << "/" << temp[1] + 1 << "/" << temp[1] + 1
					<< " "
					<< temp[2] + 1 << "/" << temp[2] + 1 << "/" << temp[2] + 1 << std::endl;
			}


			baseFaceSize += vGroups[i].vVerts.size();

		}
		std::cout << std::endl;
		std::cout << "Finished!\nSaved as " << name.c_str() << std::endl;
		obj.close();

	}
}
