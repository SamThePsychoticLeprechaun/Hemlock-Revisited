#include "stdafx.h"

#include "graphics/glsl_program.h"

GLuint hg::GLSLProgram::current = 0;

hg::GLSLProgram::GLSLProgram() :
    m_id(0),
    m_vertex_id(0), m_frag_id(0),
    m_is_linked(false)
{
    /* Empty */
}

void hg::GLSLProgram::init(ShaderCache* shader_cache) {
    if (initialised()) return;
    m_id = glCreateProgram();

    m_shader_cache = shader_cache;
}

void hg::GLSLProgram::dispose() {
    // Clear the vertex shader if it exists.
    if (m_vertex_id != 0) {
        glDeleteShader(m_vertex_id);
        m_vertex_id = 0;
    }

    // Clear the fragment shader if it exists.
    if (m_frag_id != 0) {
        glDeleteShader(m_frag_id);
        m_frag_id = 0;
    }

    // Clear the shader program if it exists.
    if (m_id != 0) {
        glDeleteProgram(m_id);
        m_id = 0;
        m_is_linked = false;
    }

    // Clear the attribute map.
    ShaderAttributeMap().swap(m_attributes);
}

hg::ShaderCreationResult hg::GLSLProgram::add_shader(const ShaderInfo& shader) {
    // If the program is in an uneditable state, fail.
    if (!editable()) {
        on_shader_add_fail(ShaderCreationResult::NON_EDITABLE);
        return ShaderCreationResult::NON_EDITABLE;
    }

    // Ensure we are targetting a valid shader type, that is not yet built.
    switch (shader.type) {
        case ShaderType::VERTEX:
            if (m_vertex_id != 0) {
                on_shader_add_fail(ShaderCreationResult::VERTEX_EXISTS);
                return ShaderCreationResult::VERTEX_EXISTS;
            }
            break;
        case ShaderType::FRAGMENT:
            if (m_frag_id != 0) {
                on_shader_add_fail(ShaderCreationResult::FRAG_EXISTS);
                return ShaderCreationResult::FRAG_EXISTS;
            }
            break;
        default:
            on_shader_add_fail(ShaderCreationResult::INVALID_STAGE);
            return ShaderCreationResult::INVALID_STAGE;
    }

    // Create the shader, ready for compilation.
    GLuint shader_id = glCreateShader(static_cast<GLenum>(shader.type));
    if (shader_id == 0) {
        on_shader_add_fail(ShaderCreationResult::CREATE_FAIL);
        return ShaderCreationResult::CREATE_FAIL;
    }

    // Read in the shader code.
    std::string* shader_code = m_shader_cache->fetch(shader.filepath);
    if (!shader_code) {
        on_shader_add_fail(ShaderCreationResult::READ_FAIL);
        return ShaderCreationResult::READ_FAIL;
    }
    char* buffer = (*shader_code).data();

    // Compile our shader code.
    glShaderSource(shader_id, 1, &buffer, nullptr);
    glCompileShader(shader_id);

    // Check if we succeeded in compilation.
    GLint status = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint max_length = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &max_length);

        char* log = new char[max_length + 1];
        glGetShaderInfoLog(shader_id, max_length, nullptr, log);
        log[max_length] = '\0';

        on_shader_add_fail(ShaderCreationResult::COMPILE_FAIL);
        on_shader_compilation_fail(log);

        glDeleteShader(shader_id);
        return ShaderCreationResult::COMPILE_FAIL;
    }

    // Set the appropriate shader ID.
    switch(shader.type) {
        case ShaderType::VERTEX:
            m_vertex_id = shader_id;
            break;
        case ShaderType::FRAGMENT:
            m_frag_id   = shader_id;
            break;
    }

    return ShaderCreationResult::SUCCESS;
}

hg::ShaderCreationResults hg::GLSLProgram::add_shaders(const std::string& vertex_path, const std::string& fragment_path) {
    return ShaderCreationResults{
        add_shader(ShaderInfo{ ShaderType::VERTEX,   vertex_path   }),
        add_shader(ShaderInfo{ ShaderType::FRAGMENT, fragment_path })
    };
}

hg::ShaderLinkResult hg::GLSLProgram::link() {
    // If the program is in an uneditable state, fail.
    if (!editable()) return ShaderLinkResult::NON_EDITABLE;

    // If we are missing either shader, fail.
    if (!m_vertex_id) return ShaderLinkResult::VERTEX_MISSING;
    if (!m_frag_id)   return ShaderLinkResult::FRAG_MISSING;

    // Attach our shaders, link program and then detach shaders.
    glAttachShader(m_id, m_vertex_id);
    glAttachShader(m_id, m_frag_id);

    glLinkProgram(m_id);

    glDetachShader(m_id, m_vertex_id);
    glDetachShader(m_id, m_frag_id);

    // Clean up our now redundant shaders.
    glDeleteShader(m_vertex_id);
    glDeleteShader(m_frag_id);
    m_vertex_id = 0;
    m_frag_id   = 0;

    // Get the result of linking.
    GLint status = 0;
    glGetProgramiv(m_id, GL_LINK_STATUS, &status);
    m_is_linked = (status == GL_TRUE);

    // If we failed to link, get info log and then fail.
    if (!m_is_linked) {
        GLint max_length = 0;
        glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &max_length);

        char* log = new char[max_length + 1];
        glGetProgramInfoLog(m_id, max_length, nullptr, log);
        log[max_length] = '\0';

        on_shader_link_fail(log);

        return ShaderLinkResult::LINK_FAIL;
    }

    return ShaderLinkResult::SUCCESS;
}

bool hg::GLSLProgram::set_attribute(const std::string& name, GLuint index) {
    return set_attribute({name, index});
}

bool hg::GLSLProgram::set_attribute(const ShaderAttribute& attribute) {
    if (!editable()) return false;

    glBindAttribLocation(m_id, attribute.second, attribute.first.data());
    m_attributes[attribute.first] = attribute.second;

    return true;
}

bool hg::GLSLProgram::set_attributes(const ShaderAttributes& attributes) {
    if (!editable()) return false;

    for (auto& attribute : attributes) {
        glBindAttribLocation(m_id, attribute.second, attribute.first.data());
        m_attributes[attribute.first] = attribute.second;
    }

    return true;
}

GLuint hg::GLSLProgram::uniform_location(const std::string& name) const {
    // Cannot find location of uniform until the program has been linked.
    if (!linked()) return GL_INVALID_OPERATION;

    return glGetUniformLocation(m_id, name.data());
}

void hg::GLSLProgram::enable_vertex_attrib_arrays(GLuint vao) const {
    for (auto& attribute : m_attributes) {
        glEnableVertexArrayAttrib(vao, attribute.second);
    }
}

void hg::GLSLProgram::disable_vertex_attrib_arrays(GLuint vao) const {
    for (auto& attribute : m_attributes) {
        glDisableVertexArrayAttrib(vao, attribute.second);
    }
}

bool hg::GLSLProgram::enable_vertex_attrib_array(GLuint vao, const std::string& name) const {
    try {
        glEnableVertexArrayAttrib(vao, m_attributes.at(name));
    } catch (std::out_of_range&) {
        return false;
    }
    return true;
}

bool hg::GLSLProgram::disable_vertex_attrib_array(GLuint vao, const std::string& name) const {
    try {
        glDisableVertexArrayAttrib(vao, m_attributes.at(name));
    } catch (std::out_of_range&) {
        return false;
    }
    return true;
}

void hg::GLSLProgram::use() {
    if (!in_use()) {
        glUseProgram(m_id);
        GLSLProgram::current = m_id;
    }
}

void hg::GLSLProgram::unuse() {
    if (GLSLProgram::current != 0) {
        glUseProgram(0);
        GLSLProgram::current = 0;
    }
}
