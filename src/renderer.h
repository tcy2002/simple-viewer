#pragma once

#include "common/general.h"
#include "common/mesh.h"
#include "common/transform.h"

namespace simple_viewer {

    // Note: This module is not thread-safe, so the upper module
    // should ensure that only one thread is modifying the mesh

    // must be consistent with the ObjType in opengl_viewer.h
    enum RenderType {
        R_MESH = 1,
        R_CUBE = 2,
        R_CYLINDER = 3,
        R_CONE = 4,
        R_LINE = 5,
        R_SPHERE = 6
    };

    /**
     * @brief Basic render
     */
    class Renderer {
    protected:
        unsigned int VAO, VBO, EBO;
        unsigned long long _vertex_count;
        float *_vertices;
        unsigned long long _triangle_count;
        unsigned int *_indices;

        COMMON_BOOL_GET(inited, Inited)
        COMMON_BOOL_GET(dynamic, Dynamic)
        COMMON_MEMBER_SET_GET(common::Transform<float>, transform, Transform)
        COMMON_MEMBER_SET_GET(common::Vector3<float>, color, Color)

    public:
        Renderer();
        Renderer(const Renderer& other) = delete;
        virtual ~Renderer();

        virtual int type() const = 0;
        virtual void init(int VAP_position, int VAP_normal);
        virtual void deinit();
        virtual void render(bool line) = 0;
    };

    /**
     * @brief Mesh renderer
     */
    class MeshRenderer : public Renderer {
    protected:
        void loadMesh(const common::Mesh<float>& mesh);

    public:
        explicit MeshRenderer(const common::Mesh<float>& mesh, bool dynamic = false);

        int type() const override { return RenderType::R_MESH; }
        void render(bool line) override;

        bool updateMesh(const common::Mesh<float>& mesh);
    };

    /**
     * @brief Cube renderer
     */
    class CubeRenderer : public Renderer {
    protected:
        void loadCube(const common::Vector3<float>& size);

    public:
        explicit CubeRenderer(const common::Vector3<float>& size, bool dynamic = false);

        int type() const override { return RenderType::R_CUBE; }
        void render(bool line) override;

        bool updateCube(const common::Vector3<float>& size);
    };

    class CylinderRenderer : public Renderer {
    protected:
        void loadCylinder(float radius, float height);

    public:
        explicit CylinderRenderer(float radius, float height, bool dynamic = false);

        int type() const override { return RenderType::R_CYLINDER; }
        void render(bool line) override;

        bool updateCylinder(float radius, float height);
    };

    /**
     * @brief Cone renderer
     */
    class ConeRenderer : public Renderer {
    protected:
        void loadCone(float radius, float height);

    public:
        explicit ConeRenderer(float radius, float height, bool dynamic = false);

        int type() const override { return RenderType::R_CONE; }
        void render(bool line) override;

        bool updateCone(float radius, float height);
    };

    /**
     * @brief Sphere renderer
     */
    class SphereRenderer : public Renderer {
    protected:
        void loadSphere(float radius);

    public:
        explicit SphereRenderer(float radius, bool dynamic = false);

        int type() const override { return RenderType::R_SPHERE; }
        void render(bool line) override;

        bool updateSphere(float radius);
    };

    /**
     * @brief Line renderer
     */
    class LineRenderer : public Renderer {
    protected:
        COMMON_MEMBER_SET_GET(float, width, Width)

        void loadLine(const std::vector<float>& points);

    public:
        explicit LineRenderer(const std::vector<float>& points, bool dynamic = false);

        int type() const override { return RenderType::R_LINE; }
        void render(bool line) override;

        bool updateLine(const std::vector<float>& points);
    };

    // TODO
    class CharRenderer : public Renderer {
    protected:
        static std::vector<std::vector<common::Mesh<float>>> _char_vertices;
    };

} // namespace simple_viewer