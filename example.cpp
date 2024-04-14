#include "opengl_viewer.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <string>

#ifdef _WIN32
#define SLEEP(t) Sleep(t)
#else
#include <unistd.h>
#define SLEEP(t) usleep((t) * 1000)
#endif

void output_obj_debug(const common::Mesh<float>& mesh, const std::string& path) {
	std::ofstream ofs(path);
	for (auto& vert : mesh.vertices) {
		ofs << "v " << vert.position.x << " " << vert.position.y << " " << vert.position.z << "\n";
	}
	for (auto& face : mesh.faces) {
		ofs << "f ";
		for (auto& i : face.indices) {
			ofs << i + 1 << " ";
		}
		ofs << "\n";
	}
	ofs.close();
}

void input_obj_debug(common::Mesh<float>& mesh, const std::string& path) {
	std::ifstream ifs(path);
	std::string type;
	ifs >> type;
	while (ifs.good()) {
		if (type == "v") {
			float x, y, z;
			ifs >> x >> y >> z >> type;
			mesh.vertices.push_back({ {x, y, z}, {0, 1, 0} });
		}
		else {
			mesh.faces.push_back({});
			auto& face = mesh.faces.back().indices;
			ifs >> type;
			while (type != "v" && type != "f" && ifs.good()) {
				face.push_back(std::stoi(type) - 1);
				ifs >> type;
			}
		}
	}
	ifs.close();
}

int main()
{
	std::thread render_thread([] {
		simple_viewer::setCamera(common::Vector3<float>(0, 0, 10), 0, 0);
		simple_viewer::open("SimpleViewer", 800, 600);
		});

	simple_viewer::setTargetFrameRate(60);
	common::Mesh<float> mesh;
	input_obj_debug(mesh, DATA_DIR "terrain.obj");
	common::Mesh<float>::perVertexNormal(mesh);
	int id0 = simple_viewer::addObj({ simple_viewer::OBJ_MESH, false, mesh });
    simple_viewer::addObj({simple_viewer::OBJ_LINE, false,
                           {0.0, 1.0, 0.0, 1.0, 2.0, 0.0, 0.0, 3.0, 0.0, -1.0, 2.0, 0.0, 0.0, 1.0, 0.0}});
	common::Transform<float> trans;
	trans.setOrigin({-5000, -13000, -5000});
	simple_viewer::updateObj({ simple_viewer::OBJ_UPDATE_TRANSFORM, id0, simple_viewer::OBJ_MESH, trans });
	simple_viewer::updateObj({ simple_viewer::OBJ_UPDATE_COLOR, id0, simple_viewer::OBJ_MESH, common::Vector3<float>{ 0.71f, 0.90f, 0.11f } });
	int id1 = simple_viewer::addObj({ simple_viewer::OBJ_CUBE, false, 1, 1, 1 });
	int id2 = simple_viewer::addObj({ simple_viewer::OBJ_CYLINDER, false, 0.5, 1 });
	int id3 = simple_viewer::addObj({ simple_viewer::OBJ_CONE, false, 1, 1 });
    int id4 = simple_viewer::addObj({simple_viewer::OBJ_SPHERE, false, 0.5});
    simple_viewer::updateObj({simple_viewer::OBJ_UPDATE_TRANSFORM, id4, simple_viewer::OBJ_SPHERE,
                              common::Transform<float>(common::Matrix3x3<float>::identity(),
                                                       common::Vector3<float>(0, -2, 0))});

	float angle = 0;
	clock_t tick = -1;
	while (simple_viewer::getKeyState('q') != 0 && simple_viewer::isOpen()) {
		SLEEP(16);
		if (tick == -1) { tick = clock(); continue; }
		clock_t tick_now = clock();
		double dt = (tick_now - tick) / 1000.0;
		tick = tick_now;

		trans.setRotation({ 1, 1, 0 }, angle);
		trans.setTranslation({ 3, 0, 0 });
		simple_viewer::updateObj({ simple_viewer::OBJ_UPDATE_TRANSFORM, id1, simple_viewer::OBJ_CUBE, trans });
		trans.setTranslation({ -3, 0, 0 });
		simple_viewer::updateObj({ simple_viewer::OBJ_UPDATE_TRANSFORM, id2, simple_viewer::OBJ_CYLINDER, trans });
		trans.setTranslation({ 0, 0, 0 });
		simple_viewer::updateObj({ simple_viewer::OBJ_UPDATE_TRANSFORM, id3, simple_viewer::OBJ_CONE, trans });

		angle += (float)(0.2 * dt);
		if (angle > 2 * 3.14159265359) angle = 0;
	}

    simple_viewer::close();
    render_thread.join();
}
