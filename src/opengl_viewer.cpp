#include "opengl_viewer.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <mutex>
#include <vector>
#include <atomic>
#include "camera.h"
#include "renderer.h"
#include "shader_program.h"
#include "shader_vert.h"
#include "shader_frag.h"
#include "common/vector3.h"
#include "common/matrix3x3.h"
#include "common/transform.h"

namespace simple_viewer {

    //// lock: one global lock is enough
    static std::mutex mtx;

    //// frame interval
    static std::atomic<int> frame_dt(16);

    //// camera
    static std::atomic<Camera*> camera(nullptr);
    static std::atomic<bool> camera_movable(true);

    //// shader
    static ShaderProgram* shader = nullptr;

    //// object
    static std::vector<std::pair<int, Renderer*>> objs;
    static int max_id = -1;

    //// axis
    static LineRenderer* axis_line = nullptr;
    static ConeRenderer* axis_arrow = nullptr;
    static common::Vector3<float> axis_color[3] = { {0.93, 0.11, 0.14}, {0.13, 0.69, 0.3}, {0, 0.64, 0.91} }; // NOLINT
    static std::atomic<bool> show_axis(true);

    //// state
    static std::vector<int> mouse_state(50, 1); // NOLINT
    static std::vector<int> key_state(128, 1); // NOLINT

    #define SV_RENDER_OBJ(obj) \
    do { if (!(obj)->isInited()) (obj)->init(1, 2); \
    (obj)->render(); } while (0)

    static void drawAxis(int axis, const common::Matrix3x3<float>& basis) {
        if (!axis_line) return;
        shader->setVec3("gColor", axis_color[axis]);

        // show the axis strip
        axis_line->setWidth(2);
        common::Matrix3x3<float> rot(axis != 0, axis == 0, 0, -(float)(axis == 0),
            axis == 1, -(float)(axis == 2), 0, axis == 2, axis != 2);
        shader->setMat3("gWorldBasis", basis * rot);
        shader->setVec3("gWorldOrigin", common::Vector3<float>(0, 0, -8));
        shader->setFloat("gAmbientIntensity", 1.0f);
        shader->setFloat("gDiffuseIntensity", 0);
        SV_RENDER_OBJ(axis_line);

        // show the top arrow
        common::Vector3<float> offset(axis == 0, axis == 1, axis == 2);
        shader->setVec3("gWorldOrigin", common::Vector3<float>(0, 0, -8) + basis * offset * 0.5);
        shader->setFloat("gAmbientIntensity", 0.5f);
        shader->setFloat("gDiffuseIntensity", 0.8f);
        SV_RENDER_OBJ(axis_arrow);
    }

    static void drawMesh(MeshRenderer* mesh) {
        shader->setFloat("gAmbientIntensity", 0.5f);
        shader->setFloat("gDiffuseIntensity",  0.8f);
        SV_RENDER_OBJ(mesh);
    }

    static void drawCube(CubeRenderer* cube) {
        shader->setFloat("gAmbientIntensity", 0.5f);
        shader->setFloat("gDiffuseIntensity", 0.8f);
        SV_RENDER_OBJ(cube);
    }

    static void drawCylinder(CylinderRenderer* cyl) {
        shader->setFloat("gAmbientIntensity", 0.5f);
        shader->setFloat("gDiffuseIntensity", 0.8f);
        SV_RENDER_OBJ(cyl);
    }

    static void drawCone(ConeRenderer* cone) {
        shader->setFloat("gAmbientIntensity", 0.5f);
        shader->setFloat("gDiffuseIntensity", 0.8f);
        SV_RENDER_OBJ(cone);
    }

    static void drawLine(LineRenderer* line) {
        shader->setFloat("gAmbientIntensity", 1.0f);
        shader->setFloat("gDiffuseIntensity", 0);
        SV_RENDER_OBJ(line);
    }

    static void drawObjects() {
        std::unique_lock<std::mutex> lock(mtx);
        for (int i = (int)objs.size() - 1; i >= 0; i--) {
            // check if object is deleted
            if (objs[i].first == -1) {
                objs[i].second->deinit();
                delete objs[i].second;
                objs.erase(objs.begin() + i);
                continue;
            }

            // render object
            auto& transform = objs[i].second->getTransform();
            shader->setMat3("gWorldBasis", transform.getBasis());
            shader->setVec3("gWorldOrigin", transform.getOrigin());
            shader->setVec3("gColor", objs[i].second->getColor());
            if (objs[i].second->type() == RenderType::R_MESH) {
                drawMesh(dynamic_cast<MeshRenderer*>(objs[i].second));
            } else if (objs[i].second->type() == RenderType::R_CUBE) {
                drawCube(dynamic_cast<CubeRenderer*>(objs[i].second));
            } else if (objs[i].second->type() == RenderType::R_CYLINDER) {
                drawCylinder(dynamic_cast<CylinderRenderer*>(objs[i].second));
            } else if (objs[i].second->type() == RenderType::R_CONE) {
                drawCone(dynamic_cast<ConeRenderer*>(objs[i].second));
            } else if (objs[i].second->type() == RenderType::R_LINE) {
                drawLine(dynamic_cast<LineRenderer*>(objs[i].second));
            }
        }
    }

    static void display() {
        if (camera.load() == nullptr || shader == nullptr) return;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render objects
        auto& camera_transform = camera.load()->getTransform(glutGet(GLUT_ELAPSED_TIME));
        shader->setMat3("gCameraBasis", camera_transform.getBasis());
        shader->setVec3("gCameraOrigin", camera_transform.getOrigin());
        shader->setFloat("gProj[0]", (float)camera.load()->getProj(0));
        shader->setFloat("gProj[1]", (float)camera.load()->getProj(1));
        shader->setFloat("gProj[2]", (float)camera.load()->getProj(2));
        shader->setFloat("gProj[3]", (float)camera.load()->getProj(3));
        shader->setVec3("gScreenOffset", common::Vector3<float>::zeros());
        drawObjects();

        // render axes
        if (show_axis.load()) {
            shader->setMat3("gCameraBasis", common::Matrix3x3<float>::identity());
            shader->setVec3("gCameraOrigin", common::Vector3<float>::zeros());
            auto width = glutGet(GLUT_WINDOW_WIDTH);
            auto height = glutGet(GLUT_WINDOW_HEIGHT);
            float aspect = (float)width / (float)height;
            shader->setVec3("gScreenOffset", common::Vector3<float>(-1.0f + 0.2f / aspect, -0.8, 0));
            auto cam_inv_basis = camera_transform.getBasis().transposed();
            glClear(GL_DEPTH_BUFFER_BIT);
            drawAxis(0, cam_inv_basis);
            drawAxis(1, cam_inv_basis);
            drawAxis(2, cam_inv_basis);
        }

        glutSwapBuffers();
    }

    static void reshape(int width, int height) {
        if (camera.load() == nullptr) return;
        camera.load()->reshape(width, height);
        glViewport(0, 0, width, height);
    }

    static void keyboard(unsigned char key, int, int) {
        if (camera.load() == nullptr) return;
        if (camera_movable.load()) camera.load()->keyboard(key, 0);
        std::unique_lock<std::mutex> lock(mtx);
        key_state[key] = 2;
    }

    static void keyboardUp(unsigned char key, int, int) {
        if (camera.load() == nullptr) return;
        if (camera_movable.load()) camera.load()->keyboard(key, 1);
        std::unique_lock<std::mutex> lock(mtx);
        key_state[key] = 3;
    }

    static void mouse(int button, int state, int x, int y) {
        if (camera.load() == nullptr) return;
        if (camera_movable.load()) camera.load()->mouse(button, state, x, y);
        std::unique_lock<std::mutex> lock(mtx);
        mouse_state[button] = state + 2;
    }

    static void motion(int x, int y) {
        if (camera.load() == nullptr) return;
        camera.load()->motion(x, y);
    }

    static void timer(int) {
        glutPostRedisplay();
        glutTimerFunc(frame_dt.load(), timer, 1);
    }

    static void close_() {
        if (shader == nullptr) return;
        delete shader; shader = nullptr;
        // reset camera state
        if (camera.load() != nullptr) {
            camera.load()->reset();
        }
        std::unique_lock<std::mutex> lock(mtx);
        // deinit objects
        for (auto& obj: objs) {
            obj.second->deinit();
        }
        // deinit axes
        if (axis_line) axis_line->deinit();
        if (axis_arrow) axis_arrow->deinit();
    }

    void open(const std::string &name, int width, int height) {
        if (glutGetWindow() != 0) {
            throw std::runtime_error("OpenglViewer is already opened");
        }

        int argc = 0; char **argv = nullptr;
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
        glutInitWindowSize(width, height);
        glutCreateWindow(name.c_str());
        glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
        glewInit();

        glutSetKeyRepeat(0);
        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glutKeyboardFunc(keyboard);
        glutKeyboardUpFunc(keyboardUp);
        glutMouseFunc(mouse);
        glutMotionFunc(motion);
        glutTimerFunc(frame_dt.load(), timer, 1);
        glutCloseFunc(close_);

        // init a globally used shader
        shader = new ShaderProgram(shader_vert, shader_frag);
        shader->use();
        shader->setVec3("gLightDirection", common::Vector3<float>(1, -2, -3).normalized());

        // init axes
        if (!axis_line) axis_line = new LineRenderer({ 0, 0, 0, 0, 0.5, 0 });
        if (!axis_arrow) axis_arrow = new ConeRenderer(0.06, 0.15);

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.6, 0.85, 0.918, 1.);
        glClearDepth(1.);

        glutMainLoop();
    }

    void close() {
        if (glutGetWindow() != 0) {
            glutLeaveMainLoop(); // leave is ok (will trigger close_())
        }
    }

    void setTargetFrameRate(int fps) {
        int dt = 1000 / fps;
        if (dt < 0) throw std::runtime_error("Invalid frame rate");
        frame_dt.store(dt);
    }

    void setCamera(const common::Vector3<float>& position, float yaw, float pitch) {
        if (camera.load() == nullptr) {
            camera.store((new Camera));
            // initialize projection of the camera
            float aspect = 1.0;
            if (glutGetWindow() != 0) {
                auto width = glutGet(GLUT_WINDOW_WIDTH);
                auto height = glutGet(GLUT_WINDOW_HEIGHT);
                aspect = (float)width / (float)height;
            }
            camera.load()->setProj(45., aspect, .1, 100000.);
        }
        camera.load()->setPosition(position);
        camera.load()->setYaw(yaw);
        camera.load()->setPitch(pitch);
    }

    void setCameraMovable(bool move) {
        camera_movable.store(move);
    }

    static int findObj(int id, int type) {
        if (id < 0) return -1;
        int size = (int)objs.size();
        for (int idx = 0; idx < size; idx++) {
            if (objs[idx].first == id &&
                objs[idx].second->type() == type)
                return idx;
        }
        return -1;
    }

    int addObj(const ObjInitParam &param) {
        std::unique_lock<std::mutex> lock(mtx);
        switch (param.type) {
            case ObjType::OBJ_MESH:
                objs.emplace_back(++max_id, new MeshRenderer(param.mesh, param.dynamic));
                break;
            case ObjType::OBJ_CUBE:
                objs.emplace_back(++max_id, new CubeRenderer(param.size, param.dynamic));
                break;
            case ObjType::OBJ_CYLINDER:
                objs.emplace_back(++max_id, new CylinderRenderer((float)param.size.x,
                    (float)param.size.y, param.dynamic));
                break;
            case ObjType::OBJ_CONE:
                objs.emplace_back(++max_id, new ConeRenderer((float)param.size.x,
                    (float)param.size.y, param.dynamic));
                break;
            case ObjType::OBJ_LINE:
                objs.emplace_back(++max_id, new LineRenderer(param.line, param.dynamic));
                break;
            default:
                throw std::runtime_error("Unknown object type");
        }
        return max_id;
    }

    bool updateObj(const ObjUpdateParam &param) {
        std::unique_lock<std::mutex> lock(mtx);
        int obj_idx;
        if (param.act_type != OBJ_CLEAR_ALL_TYPE && param.act_type != OBJ_CLEAR_ALL) {
            obj_idx = findObj(param.obj_id, param.obj_type);
            if (obj_idx < 0) return false;
        }

        switch (param.act_type) {
            case OBJ_UPDATE_TRANSFORM:
                objs[obj_idx].second->setTransform(param.transform);
                return true;
            case OBJ_UPDATE_COLOR:
                objs[obj_idx].second->setColor(param.vec);
                return true;
            case OBJ_UPDATE_MESH:
                return dynamic_cast<MeshRenderer*>(objs[obj_idx].second)->updateMesh(param.mesh);
            case OBJ_UPDATE_CUBE:
                return dynamic_cast<CubeRenderer*>(objs[obj_idx].second)->updateCube(param.vec);
            case OBJ_UPDATE_CYLINDER:
                return dynamic_cast<CylinderRenderer*>(objs[obj_idx].second)
                    ->updateCylinder((float)param.vec.x, (float)param.vec.y);
            case OBJ_UPDATE_CONE:
                return dynamic_cast<ConeRenderer*>(objs[obj_idx].second)
                    ->updateCone((float)param.vec.x, (float)param.vec.y);
            case OBJ_UPDATE_LINE:
                return dynamic_cast<LineRenderer*>(objs[obj_idx].second)->updateLine(param.line);
            case OBJ_UPDATE_LINE_WIDTH:
                dynamic_cast<LineRenderer*>(objs[obj_idx].second)->setWidth(param.line_width);
                return true;
            case OBJ_DEL:
                objs[obj_idx].first = -1;
                return true;
            case OBJ_CLEAR_ALL_TYPE:
                for (auto& obj: objs) {
                    if (obj.second->type() == param.obj_type) {
                        obj.first = -1;
                    }
                }
                return true;
            case OBJ_CLEAR_ALL:
                for (auto& obj: objs) {
                    obj.first = -1;
                }
                return true;
            default:
                throw std::runtime_error("Unknown update command type");
        }
    }

    void showAxis(bool show) {
        std::unique_lock<std::mutex> lock(mtx);
        show_axis.store(show);
    }

    int getMouseState(int button) {
        if (button < 0 || button >= 50) return -1;
        std::unique_lock<std::mutex> lock(mtx);
        int state = mouse_state[button];
        if (state > 1) mouse_state[button] -= 2;
        return state;
    }

    int getKeyState(char key) {
        if (key < 0) return -1;
        std::unique_lock<std::mutex> lock(mtx);
        int state = key_state[key];
        if (state > 1) key_state[key] -= 2;
        return state;
    }

} // namespace simple_viewer