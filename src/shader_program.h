#pragma once

#include "common/general.h"

namespace simple_viewer {

    class ShaderProgram {
    private:
        int _program_id;

        static int createShader(const char* buf, int type);
        static void compileShaderAndCheck(int shader);
        static void linkProgramAndCheck(int program);

    public:
        ShaderProgram(const char* vert_path, const char* frag_path);
        ~ShaderProgram();

        void use() const;

        void setBool(const char* name, bool value) const;
        void setInt(const char* name, int value) const;
        void setFloat(const char* name, float value) const;
        void setVec3(const char* name, const common::Vector3<float>& value) const;
        void setMat3(const char* name, const common::Matrix3<float>& value) const;
    };

} // namespace simple_viewer