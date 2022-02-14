#include "stdafx.h"

#include "graphics/mesh.h"

#if defined(HEMLOCK_USING_OPENGL)
template <bool indexed, ui32 vertex_size, ui32 colour_size, ui32 precision>
static bool __upload_basic_mesh(auto mesh_data, hg::MeshDataVolatility volatility, IN OUT GLuint* vao, IN OUT GLuint* vbo, IN OUT GLuint* ibo) {
    assert(mesh_data.vertices != nullptr);
    assert(vao != nullptr);
    assert(vbo != nullptr);

    if constexpr (indexed) {
        assert(mesh_data.indices != nullptr);
        assert(ibo != nullptr);
    }

    glCreateVertexArrays(1, vao);

    glCreateBuffers(1, vbo);
    glNamedBufferData(*vbo, vertex_size * mesh_data.vertex_count, mesh_data.vertices, static_cast<GLenum>(volatility));

    // Associate VBO to VAO.
    glVertexArrayVertexBuffer(*vao, 0, *vbo, 0, vertex_size);

    if constexpr (indexed) {
        glCreateBuffers(1, ibo);
        glNamedBufferData(*ibo, 4 * mesh_data.index_count, mesh_data.indices, static_cast<GLenum>(volatility));

        // Associate IBO to VAO.
        glVertexArrayElementBuffer(*vao, *ibo);
    }

    glEnableVertexArrayAttrib(*vao, hg::MeshAttribID::POSITION);
    glEnableVertexArrayAttrib(*vao, hg::MeshAttribID::UV_COORDS);
    if constexpr (colour_size > 0)
    glEnableVertexArrayAttrib(*vao, hg::MeshAttribID::COLOUR);

    glVertexArrayAttribFormat(*vao, hg::MeshAttribID::POSITION,            3, precision == 4 ? GL_FLOAT : GL_DOUBLE, GL_FALSE,             0);
    glVertexArrayAttribFormat(*vao, hg::MeshAttribID::UV_COORDS,           2, precision == 4 ? GL_FLOAT : GL_DOUBLE, GL_FALSE, 3 * precision);
    if constexpr (colour_size > 0)
    glVertexArrayAttribFormat(*vao, hg::MeshAttribID::COLOUR,    colour_size, precision == 4 ? GL_FLOAT : GL_DOUBLE, GL_TRUE, 5 * precision);

    glVertexArrayAttribBinding(*vao, hg::MeshAttribID::POSITION,  0);
    glVertexArrayAttribBinding(*vao, hg::MeshAttribID::UV_COORDS, 0);
    if constexpr (colour_size > 0)
    glVertexArrayAttribBinding(*vao, hg::MeshAttribID::COLOUR,    0);

    return static_cast<bool>(mesh_data.vertex_count);
}

template <bool indexed>
static void __dispose_mesh(GLuint vao, GLuint vbo, GLuint ibo) {
    assert(vao != 0);
    assert(vbo != 0);
    if constexpr (indexed) assert(ibo != 0);

    glDeleteBuffers(1, &vbo);
    if constexpr (indexed) glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &vao);
}
#endif // defined(HEMLOCK_USING_OPENGL)


/***************************\
 *                         *
 * Colourless Entry-points *
 * ----------------------- *
\***************************/

bool hg::upload_mesh(
    const Colourless_MeshData2D_32& mesh_data,
                       MeshHandles& handles,
                 MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 0, 4>(mesh_data, volatility, &handles.vao, &handles.vbo, nullptr);
}

bool hg::upload_mesh(
    const Colourless_MeshData2D_64& mesh_data,
                       MeshHandles& handles,
                 MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 0, 8>(mesh_data, volatility, &handles.vao, &handles.vbo, nullptr);
}

bool hg::upload_mesh(
    const Colourless_MeshData3D_32& mesh_data,
                       MeshHandles& handles,
                 MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 0, 4>(mesh_data, volatility, &handles.vao, &handles.vbo, nullptr);
}

bool hg::upload_mesh(
    const Colourless_MeshData3D_64& mesh_data,
                       MeshHandles& handles,
                 MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 0, 8>(mesh_data, volatility, &handles.vao, &handles.vbo, nullptr);
}

bool hg::upload_mesh(
    const Colourless_IndexedMeshData2D_32& mesh_data,
                       IndexedMeshHandles& handles,
                        MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 0, 4>(mesh_data, volatility, &handles.vao, &handles.vbo, &handles.ibo);
}

bool hg::upload_mesh(
    const Colourless_IndexedMeshData2D_64& mesh_data,
                       IndexedMeshHandles& handles,
                        MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 0, 8>(mesh_data, volatility, &handles.vao, &handles.vbo, &handles.ibo);
}

bool hg::upload_mesh(
    const Colourless_IndexedMeshData3D_32& mesh_data,
                       IndexedMeshHandles& handles,
                        MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 0, 4>(mesh_data, volatility, &handles.vao, &handles.vbo, &handles.ibo);
}

bool hg::upload_mesh(
    const Colourless_IndexedMeshData3D_64& mesh_data,
                       IndexedMeshHandles& handles,
                        MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 0, 8>(mesh_data, volatility, &handles.vao, &handles.vbo, &handles.ibo);
}


/********************\
 *                  *
 * RGB Entry-points *
 * ---------------- *
\********************/

bool hg::upload_mesh(
    const RGB_MeshData2D_32& mesh_data,
                MeshHandles& handles,
          MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 3, 4>(mesh_data, volatility, &handles.vao, &handles.vbo, nullptr);
}

bool hg::upload_mesh(
    const RGB_MeshData2D_64& mesh_data,
                MeshHandles& handles,
          MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 3, 8>(mesh_data, volatility, &handles.vao, &handles.vbo, nullptr);
}

bool hg::upload_mesh(
    const RGB_MeshData3D_32& mesh_data,
                MeshHandles& handles,
          MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 3, 4>(mesh_data, volatility, &handles.vao, &handles.vbo, nullptr);
}

bool hg::upload_mesh(
    const RGB_MeshData3D_64& mesh_data,
                MeshHandles& handles,
          MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 3, 8>(mesh_data, volatility, &handles.vao, &handles.vbo, nullptr);
}

bool hg::upload_mesh(
    const RGB_IndexedMeshData2D_32& mesh_data,
                IndexedMeshHandles& handles,
                 MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 3, 4>(mesh_data, volatility, &handles.vao, &handles.vbo, &handles.ibo);
}

bool hg::upload_mesh(
    const RGB_IndexedMeshData2D_64& mesh_data,
                IndexedMeshHandles& handles,
                 MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 3, 8>(mesh_data, volatility, &handles.vao, &handles.vbo, &handles.ibo);
}

bool hg::upload_mesh(
    const RGB_IndexedMeshData3D_32& mesh_data,
                IndexedMeshHandles& handles,
                 MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 3, 4>(mesh_data, volatility, &handles.vao, &handles.vbo, &handles.ibo);
}

bool hg::upload_mesh(
    const RGB_IndexedMeshData3D_64& mesh_data,
                IndexedMeshHandles& handles,
                 MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 3, 8>(mesh_data, volatility, &handles.vao, &handles.vbo, &handles.ibo);
}


/*********************\
 *                   *
 * RGBA Entry-points *
 * ----------------- *
\*********************/

bool hg::upload_mesh(
    const RGBA_MeshData2D_32& mesh_data,
                 MeshHandles& handles,
           MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 4, 4>(mesh_data, volatility, &handles.vao, &handles.vbo, nullptr);
}

bool hg::upload_mesh(
    const RGBA_MeshData2D_64& mesh_data,
                 MeshHandles& handles,
           MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 4, 8>(mesh_data, volatility, &handles.vao, &handles.vbo, nullptr);
}

bool hg::upload_mesh(
    const RGBA_MeshData3D_32& mesh_data,
                 MeshHandles& handles,
           MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 4, 4>(mesh_data, volatility, &handles.vao, &handles.vbo, nullptr);
}

bool hg::upload_mesh(
    const RGBA_MeshData3D_64& mesh_data,
                 MeshHandles& handles,
           MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<false, sizeof(decltype(mesh_data.vertices[0])), 4, 8>(mesh_data, volatility, &handles.vao, &handles.vbo, nullptr);
}

bool hg::upload_mesh(
    const RGBA_IndexedMeshData2D_32& mesh_data,
                 IndexedMeshHandles& handles,
                  MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 4, 4>(mesh_data, volatility, &handles.vao, &handles.vbo, &handles.ibo);
}

bool hg::upload_mesh(
    const RGBA_IndexedMeshData2D_64& mesh_data,
                 IndexedMeshHandles& handles,
                  MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 4, 8>(mesh_data, volatility, &handles.vao, &handles.vbo, &handles.ibo);
}

bool hg::upload_mesh(
    const RGBA_IndexedMeshData3D_32& mesh_data,
                 IndexedMeshHandles& handles,
                  MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 4, 4>(mesh_data, volatility, &handles.vao, &handles.vbo, &handles.ibo);
}

bool hg::upload_mesh(
    const RGBA_IndexedMeshData3D_64& mesh_data,
                 IndexedMeshHandles& handles,
                  MeshDataVolatility volatility /*= MeshDataVolatility::DYNAMIC*/
) {
    return __upload_basic_mesh<true, sizeof(decltype(mesh_data.vertices[0])), 4, 8>(mesh_data, volatility, &handles.vao, &handles.vbo, &handles.ibo);
}

void hg::dispose_mesh(const MeshHandles& handles) {
    __dispose_mesh<false>(handles.vao, handles.vbo, 0);
}

void hg::dispose_mesh(const IndexedMeshHandles& handles) {
    __dispose_mesh<true>(handles.vao, handles.vbo, handles.ibo);
}
