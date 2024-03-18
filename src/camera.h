#pragma once

#include <mutex>
#include "common/vector3.h"
#include "common/transform.h"

namespace simple_viewer {

    /**
     * @brief A UE-styled, thread-safe 3rd-person camera
     */
    class Camera {
    public:
        Camera();

        void mouse(int button, int state, int x, int y);
        void motion(int x, int y);
        void keyboard(unsigned char key, int state);
        void reshape(int width, int height);

        void setPosition(const common::Vector3<float>& position);
        void setYaw(float yaw);
        void setPitch(float pitch);
        void setProj(float fov_degree, float aspect, float near_, float far_);

        const common::Transform<float>& getTransform(long long time);
        float getProj(int i) const;

        void reset();

    private:
        std::mutex _mutex_trans, _mutex_proj;
        float _yaw, _pitch;
        common::Vector3<float> _position;
        float _fov, _aspect, _near, _far;
        common::Transform<float> _transform;
        float _proj[4];

        const float MoveSpeedMin = 0.05f, MoveSpeedMax = 10.f, MoveSpeedStep = 1.5f;
        float _move_speed = MoveSpeedMin;
        const float RotateSpeed = 0.004f;
        float _rotate_speed = RotateSpeed;
        const float WheelStep = 8.0f;
        const float KeyStep = 200.0f;

        int _state_left = 1, _state_middle = 1, _state_right = 1;
        int _state_w = 1, _state_a = 1, _state_s = 1, _state_d = 1;
        int _state_q = 1, _state_e = 1;
        int _last_x = -1, _last_y = -1;
        long long _last_time = -1;
    };

} // namespace simple_viewer