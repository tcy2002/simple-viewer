#include "renderer.h"

#include <GL/glew.h>
#include "default_mesh.h"

namespace simple_viewer {

    Renderer::Renderer():
            VAO(0), VBO(0), EBO(0),
            _vertex_count(0), _vertices(nullptr),
            _triangle_count(0), _indices(nullptr),
            _inited(false), _dynamic(false),
            _transform(common::Transform<float>::identity()),
            _color({1, 1, 1}) {}

    Renderer::~Renderer() {
        deinit(); // NOLINT
        delete[] _vertices;
        delete[] _indices;
    }

    void Renderer::init(int VAP_position, int VAP_normal) {
        if (_inited) return;
        auto draw_mode = _dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
        if (VAO == 0) {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);
        }

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, (GLsizei)(sizeof(float) * 6 * _vertex_count), _vertices, draw_mode);
        glEnableVertexAttribArray(VAP_position);
        glEnableVertexAttribArray(VAP_normal);
        glVertexAttribPointer(VAP_position, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0); // NOLINT
        glVertexAttribPointer(VAP_normal, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void *)(sizeof(float) * 3));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizei)(sizeof(unsigned int) * 3 * _triangle_count), _indices, draw_mode);
        glBindVertexArray(0);
        _inited = true;
    }

    void Renderer::deinit() {
        if (VAO == 0) return;
        glDeleteVertexArrays(1, &VAO); VAO = 0;
        glDeleteBuffers(1, &VBO); VBO = 0;
        glDeleteBuffers(1, &EBO); EBO = 0;
        _inited = false;
    }

    void MeshRenderer::loadMesh(const common::Mesh<float>& mesh) {
        // load vertex data
        _vertex_count = mesh.vertices.size();
        _vertices = new float[_vertex_count * 6];
        int i = 0;
        for (unsigned int j = 0; j < _vertex_count; j++) {
            auto& v = mesh.vertices[j].position;
            auto& n = mesh.vertices[j].normal;
            _vertices[i++] = v.x();
            _vertices[i++] = v.y();
            _vertices[i++] = v.z();
            _vertices[i++] = n.x();
            _vertices[i++] = n.y();
            _vertices[i++] = n.z();
        }

        // load face data, and transform into triangles
        _triangle_count = 0;
        for (auto& f : mesh.faces) {
            _triangle_count += f.indices.size() - 2;
        }
        _indices = new unsigned int[_triangle_count * 3];
        unsigned long long n; i = 0;
        for (auto& f : mesh.faces) {
            n = f.indices.size();
            for (unsigned int j = 1; j < n - 1; j++) {
                _indices[i++] = f.indices[0];
                _indices[i++] = f.indices[j];
                _indices[i++] = f.indices[j + 1];
            }
        }
    }

    MeshRenderer::MeshRenderer(const common::Mesh<float>& mesh, bool dynamic):
        Renderer() {
        loadMesh(mesh);
        _dynamic = dynamic;
        _color = {0.3f, 0.25f, 0.8f};
    }

    void MeshRenderer::render(bool line) {
        if (!_inited) return;
        glBindVertexArray(VAO);
        if (line) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glDrawElements(GL_TRIANGLES, (GLsizei)(_triangle_count * 3),
                       GL_UNSIGNED_INT, (void*)0); // NOLINT
        glBindVertexArray(0);
    }

    bool MeshRenderer::updateMesh(const common::Mesh<float>& mesh) {
        if (!_dynamic) return false;
        loadMesh(mesh);
        _inited = false;
        return true;
    }

    void CubeRenderer::loadCube(const common::Vector3<float> &size) {
        // load vertex data
        _vertex_count = _cube_mesh.vertices.size();
        _vertices = new float[_vertex_count * 6];
        int i = 0;
        for (unsigned int j = 0; j < _vertex_count; j++) {
            auto& v = _cube_mesh.vertices[j].position;
            auto& n = _cube_mesh.vertices[j].normal;
            _vertices[i++] = (v.x() * size.x());
            _vertices[i++] = (v.y() * size.y());
            _vertices[i++] = (v.z() * size.z());
            _vertices[i++] = (n.x());
            _vertices[i++] = (n.y());
            _vertices[i++] = (n.z());
        }

        // load face data, and transform into triangles
        _triangle_count = 0;
        for (auto& f : _cube_mesh.faces) {
            _triangle_count += f.indices.size() - 2;
        }
        _indices = new unsigned int[_triangle_count * 3];
        unsigned long long n; i = 0;
        for (auto& f : _cube_mesh.faces) {
            n = f.indices.size();
            for (unsigned int j = 1; j < n - 1; j++) {
                _indices[i++] = f.indices[0];
                _indices[i++] = f.indices[j];
                _indices[i++] = f.indices[j + 1];
            }
        }
    }

    CubeRenderer::CubeRenderer(const common::Vector3<float> &size, bool dynamic) {
        loadCube(size);
        _dynamic = dynamic;
        _color = {0.3f, 0.25f, 0.8f};
    }

    void CubeRenderer::render(bool line) {
        if (!_inited) return;
        glBindVertexArray(VAO);
        if (line) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glDrawElements(GL_TRIANGLES, (GLsizei)(_triangle_count * 3),
                       GL_UNSIGNED_INT, (void*)0); // NOLINT
        glBindVertexArray(0);
    }

    bool CubeRenderer::updateCube(const common::Vector3<float> &size) {
        if (!_dynamic) return false;
        loadCube(size);
        _inited = false;
        return true;
    }

    void CylinderRenderer::loadCylinder(float radius, float height) {
        // load vertex data
        radius *= 2;
        _vertex_count = _cylinder_mesh.vertices.size();
        _vertices = new float[_vertex_count * 6];
        int i = 0;
        for (unsigned int j = 0; j < _vertex_count; j++) {
            auto& v = _cylinder_mesh.vertices[j].position;
            auto& n = _cylinder_mesh.vertices[j].normal;
            _vertices[i++] = (v.x() * radius);
            _vertices[i++] = (v.y() * height);
            _vertices[i++] = (v.z() * radius);
            _vertices[i++] = (n.x());
            _vertices[i++] = (n.y());
            _vertices[i++] = (n.z());
        }

        // load face data, and transform into triangles
        _triangle_count = 0;
        for (auto& f : _cylinder_mesh.faces) {
            _triangle_count += f.indices.size() - 2;
        }
        _indices = new unsigned int[_triangle_count * 3];
        unsigned long long n; i = 0;
        for (auto& f : _cylinder_mesh.faces) {
            n = f.indices.size();
            for (unsigned int j = 1; j < n - 1; j++) {
                _indices[i++] = f.indices[0];
                _indices[i++] = f.indices[j];
                _indices[i++] = f.indices[j + 1];
            }
        }
    }

    CylinderRenderer::CylinderRenderer(float radius, float height, bool dynamic) {
        loadCylinder(radius, height);
        _dynamic = dynamic;
        _color = {0.3f, 0.25f, 0.8f};
    }

    void CylinderRenderer::render(bool line) {
        if (!_inited) return;
        glBindVertexArray(VAO);
        if (line) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glDrawElements(GL_TRIANGLES, (GLsizei)(_triangle_count * 3),
                       GL_UNSIGNED_INT, (void*)0); // NOLINT
        glBindVertexArray(0);
    }

    bool CylinderRenderer::updateCylinder(float radius, float height) {
        if (!_dynamic) return false;
        loadCylinder(radius, height);
        _inited = false;
        return true;
    }

    void ConeRenderer::loadCone(float radius, float height) {
        // load vertex data
        float r = std::sqrt(radius * radius + height * height);
        float sin_a_r = radius / r / 0.4472136f;
        float cos_a_r = height / r / 0.8944272f;
        radius *= 2;
        _vertex_count = _cone_mesh.vertices.size();
        _vertices = new float[_vertex_count * 6];
        int i = 0;
        for (unsigned int j = 0; j < _vertex_count; j++) {
            auto& v = _cone_mesh.vertices[j].position;
            auto& n = _cone_mesh.vertices[j].normal;
            _vertices[i++] = (v.x() * radius);
            _vertices[i++] = (v.y() * height);
            _vertices[i++] = (v.z() * radius);
            _vertices[i++] = (n.x() * cos_a_r);
            _vertices[i++] = (n.y() * sin_a_r);
            _vertices[i++] = (n.z() * cos_a_r);
        }

        // load face data, and transform into triangles
        _triangle_count = 0;
        for (auto& f : _cone_mesh.faces) {
            _triangle_count += f.indices.size() - 2;
        }
        _indices = new unsigned int[_triangle_count * 3];
        unsigned long long n; i = 0;
        for (auto& f : _cone_mesh.faces) {
            n = f.indices.size();
            for (unsigned int j = 1; j < n - 1; j++) {
                _indices[i++] = f.indices[0];
                _indices[i++] = f.indices[j];
                _indices[i++] = f.indices[j + 1];
            }
        }
    }

    ConeRenderer::ConeRenderer(float radius, float height, bool dynamic):
            Renderer() {
        loadCone(radius, height);
        _dynamic = dynamic;
        _color = {0.3f, 0.25f, 0.8f};
    }

    void ConeRenderer::render(bool line) {
        if (!_inited) return;
        glBindVertexArray(VAO);
        if (line) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glDrawElements(GL_TRIANGLES, (GLsizei)(_triangle_count * 3),
                       GL_UNSIGNED_INT, (void*)0); // NOLINT
        glBindVertexArray(0);
    }

    bool ConeRenderer::updateCone(float radius, float height) {
        if (!_dynamic) return false;
        loadCone(radius, height);
        _inited = false;
        return true;
    }

    void SphereRenderer::loadSphere(float radius) {
        // load vertex data
        radius *= 2;
        _vertex_count = _sphere_mesh.vertices.size();
        _vertices = new float[_vertex_count * 6];
        int i = 0;
        for (unsigned int j = 0; j < _vertex_count; j++) {
            auto& v = _sphere_mesh.vertices[j].position;
            auto& n = _sphere_mesh.vertices[j].normal;
            _vertices[i++] = (v.x() * radius);
            _vertices[i++] = (v.y() * radius);
            _vertices[i++] = (v.z() * radius);
            _vertices[i++] = (n.x());
            _vertices[i++] = (n.y());
            _vertices[i++] = (n.z());
        }

        // load face data, and transform into triangles
        _triangle_count = 0;
        for (auto& f : _sphere_mesh.faces) {
            _triangle_count += f.indices.size() - 2;
        }
        _indices = new unsigned int[_triangle_count * 3];
        unsigned long long n; i = 0;
        for (auto& f : _sphere_mesh.faces) {
            n = f.indices.size();
            for (unsigned int j = 1; j < n - 1; j++) {
                _indices[i++] = f.indices[0];
                _indices[i++] = f.indices[j];
                _indices[i++] = f.indices[j + 1];
            }
        }
    }

    SphereRenderer::SphereRenderer(float radius, bool dynamic):
            Renderer() {
        loadSphere(radius);
        _dynamic = dynamic;
        _color = {0.3f, 0.25f, 0.8f};
    }

    void SphereRenderer::render(bool line) {
        if (!_inited) return;
        glBindVertexArray(VAO);
        if (line) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glDrawElements(GL_TRIANGLES, (GLsizei)(_triangle_count * 3),
                       GL_UNSIGNED_INT, (void*)0); // NOLINT
        glBindVertexArray(0);
    }

    bool SphereRenderer::updateSphere(float radius) {
        if (!_dynamic) return false;
        loadSphere(radius);
        _inited = false;
        return true;
    }

    void LineRenderer::loadLine(const std::vector<float>& points) {
        _vertex_count = (points.size() - 3) * 2 / 3;
        _vertices = new float[_vertex_count * 6]{};
        int i = 6;
        _vertices[0] = points[0];
        _vertices[1] = points[1];
        _vertices[2] = points[2];
        for (int k = 3; k < (int)points.size() - 3; k += 3) {
            float x = points[k], y = points[k + 1], z = points[k + 2];
            _vertices[i++] = x;
            _vertices[i++] = y;
            _vertices[i++] = z;
            i += 3;
            _vertices[i++] = x;
            _vertices[i++] = y;
            _vertices[i++] = z;
            i += 3;
        }
        int j = (int)points.size() - 3;
        _vertices[i++] = points[j];
        _vertices[i++] = points[j + 1];
        _vertices[i] = points[j + 2];
    }

    LineRenderer::LineRenderer(const std::vector<float>& points, bool dynamic):
            Renderer(), _width(1) {
        loadLine(points);
        _dynamic = dynamic;
        _color = {1.0f, 0.95f, 0.0f};
    }

    void LineRenderer::render(bool line) {
        if (!_inited) return;
        glBindVertexArray(VAO);
        glLineWidth((float)_width);
        glDrawArrays(GL_LINES, 0, (GLsizei)(_vertex_count * 3));
        glBindVertexArray(0);
    }

    bool LineRenderer::updateLine(const std::vector<float>& points) {
        if (!_dynamic) return false;
        loadLine(points);
        _inited = false;
        return true;
    }

} // namespace simple_viewer