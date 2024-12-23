#include "camera.h"

#define SV_PI 3.1415926f

namespace simple_viewer {

    Camera::Camera(): // NOLINT
            _yaw(0.f), _pitch(0.f),
            _position(common::Vector3<float>::Zero()),
            _fov(SV_PI / 2.f),
            _transform(common::Transform<float>::identity()),
            _proj{1.f, 1.f, -1.0002f, -0.20002f} {}

    void Camera::mouse(int button, int state, int, int) {
        std::unique_lock<std::mutex> lock(_mutex_trans);

        switch (button) {
            case 0: _state_left = state; break;
            case 1: _state_middle = state; break;
            case 2: _state_right = state; break;
            default: break;
        }

        if (button <= 2 || state == 0) {
            _last_x = -1, _last_y = -1;
            return;
        }

        if ((_state_left == 0 || _state_middle == 0 || _state_right == 0) && button == 3) {
            _move_speed *= MoveSpeedStep;
            if (_move_speed > MoveSpeedMax) _move_speed = MoveSpeedMax;
        } else if ((_state_left == 0 || _state_middle == 0 || _state_right == 0) && button == 4) {
            _move_speed /= MoveSpeedStep;
            if (_move_speed < MoveSpeedMin) _move_speed = MoveSpeedMin;
        } else if (button == 3 || button == 4) {
            auto forward = -_transform.getAxis(2);
            _position += forward * ((float)(button == 3 ? 1 : -1) * _move_speed * WheelStep);
        }
    }

    void Camera::motion(int x, int y) {
        std::unique_lock<std::mutex> lock(_mutex_trans);

        float dx = _last_x != -1 ? (float)(x - _last_x) : 0;
        float dy = _last_y != -1 ? (float)(y - _last_y) : 0;
        _last_x = x, _last_y = y;

        if (_state_left == 0 && _state_right == 1) {
            _yaw -= dx * _rotate_speed;
            if (_yaw > SV_PI) _yaw -= 2.f * SV_PI;
            else if (_yaw < -SV_PI) _yaw += 2.f * SV_PI;
            common::Vector3<float> front(std::sin(_yaw), 0.f, std::cos(_yaw));
            _position += front * (dy * _move_speed);
        } else if ((_state_left == 1 && _state_middle == 0) || (_state_left == 0 && _state_right == 0)) {
            auto right_yaw = (float)(_yaw + SV_PI / 2);
            common::Vector3<float> right(std::sin(right_yaw), 0., std::cos(right_yaw));
            _position += right * (dx * _move_speed);
            _position -= common::Vector3<float>::UnitY() * (dy * _move_speed);
        } else if (_state_left == 1 && _state_middle == 1 && _state_right == 0) {
            _yaw -= dx * _rotate_speed;
            if (_yaw > SV_PI) _yaw -= 2.f * SV_PI;
            else if (_yaw < -SV_PI) _yaw += 2.f * SV_PI;
            _pitch += dy * _rotate_speed;
            if (_pitch > SV_PI / 2.f) _pitch = SV_PI / 2.f;
            else if (_pitch < -SV_PI / 2.f) _pitch = -SV_PI / 2.f;
        }
    }

    void Camera::keyboard(unsigned char key, int state) {
        std::unique_lock<std::mutex> lock(_mutex_trans);
        switch (key) {
            case 'w': case 'W': _state_w = state; break;
            case 'a': case 'A': _state_a = state; break;
            case 's': case 'S': _state_s = state; break;
            case 'd': case 'D': _state_d = state; break;
            case 'q': case 'Q': _state_q = state; break;
            case 'e': case 'E': _state_e = state; break;
            default: break;
        }
    }

    void Camera::reshape(int width, int height) {
        std::unique_lock<std::mutex> lock(_mutex_proj);
        _aspect = (float)width / (float)height;
        _proj[0] = _proj[1] / _aspect;
    }

    void Camera::setPosition(const common::Vector3<float>& position) {
        std::unique_lock<std::mutex> lock(_mutex_trans);
        _position = position;
    }

    void Camera::setYaw(float yaw) {
        std::unique_lock<std::mutex> lock(_mutex_trans);
        _yaw = yaw;
    }

    void Camera::setPitch(float pitch) {
        std::unique_lock<std::mutex> lock(_mutex_trans);
        _pitch = pitch;
    }

    void Camera::setProj(float fov_degree, float aspect, float near_, float far_) {
        std::unique_lock<std::mutex> lock(_mutex_proj);

        _fov = (float)(fov_degree * SV_PI / 180.);
        _aspect = aspect;
        _near = near_;
        _far = far_;

        _proj[1] = 1.0f / std::tan(_fov / 2.0f);
        _proj[0] = _proj[1] / _aspect;
        _proj[2] = -(_far + _near) / (_far - _near);
        _proj[3] = -2.0f * _far * _near / (_far - _near);
    }

    const common::Transform<float>& Camera::getTransform(long long time) {
        std::unique_lock<std::mutex> lock(_mutex_trans);
        if (_state_left == 0 || _state_right == 0 || _state_middle == 0) {
            float dt = _last_time != -1 ? (float)(time - _last_time) / 1000 : 0;
            _last_time = time;
            auto forward = _transform.getAxis(2);
            auto right = _transform.getAxis(0);
            auto up = _transform.getAxis(1);
            _position += forward * ((float)(_state_w - _state_s) * _move_speed * KeyStep * dt);
            _position -= right * ((float)(_state_d - _state_a) * _move_speed * KeyStep * dt);
            _position += up * ((float)(_state_e - _state_q) * _move_speed * KeyStep * dt);
        } else {
            _last_time = -1;
        }

        _transform.setEulerRotation(-_pitch, _yaw, 0., common::Transform<float>::RotType::S_ZXY);
        _transform.setTranslation(_position);
        return _transform;
    }

    float Camera::getProj(int i) const {
        return _proj[i];
    }

    void Camera::reset() {
        _state_left = _state_middle = _state_right = 1;
        _state_w = _state_a = _state_s = _state_d = 1;
        _state_q = _state_e = 1;
        _last_x = _last_y = -1;
        _last_time = -1;
    }

} // namespace simple_viewer