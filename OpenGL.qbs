import qbs

CppApplication {
    property string libraries: "D:\\Programs\\misc\\OpenGL"

    consoleApplication: true
    cpp.cxxLanguageVersion: "c++11"
    cpp.defines: ["GLEW_STATIC"]
    cpp.includePaths: [
        "./Helper_Functions/include",
        libraries + "/stb",
        libraries + "/glm",
        libraries + "/glew-1.13.0/include",
        libraries + "/glfw-3.1.2.bin.WIN32/include",
        libraries + "/bullet3-2.83.7/src",
    ]
    cpp.libraryPaths: [
        libraries + "/bullet3-2.83.7/bin",
        libraries + "/glew-1.13.0/lib/Release/Win32",
        libraries + "/glfw-3.1.2.bin.WIN32/lib-mingw-w64",
    ]
    cpp.linkerFlags: [
        "-lBulletDynamics_gmake", "-lBulletCollision_gmake", "-lLinearMath_gmake",
        "-lglew32s",
        "-lglfw3", "-lgdi32", "-lopengl32",
    ]

    files: [
        "main.cpp",
        "helper.cpp",
        "sphere.cpp",
        "object.cpp",
        "cube.cpp",
        "basic.vert",
        "basic.frag",
        "lighting.vert",
        "lighting.frag",
        "texture.vert",
        "texture.frag",
        "config.lua",
        "helper.h",
        "sphere.h",
        "global.h",
        "object.h",
        "cube.h",
        "stb_image.h",
        "world.h",
    ]

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}
