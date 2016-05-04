import qbs

CppApplication {
    property string libraries: "D:\\Programs\\misc\\OpenGL"

    consoleApplication: true
    cpp.cxxLanguageVersion: "c++11"
    cpp.defines: ["GLEW_STATIC"]
    cpp.includePaths: [
        "include",
        libraries + "/stb",
        libraries + "/glm",
        libraries + "/glew-1.13.0/include",
        libraries + "/glfw-3.1.2.bin.WIN32/include",
        libraries + "/bullet3-2.83.7/src",
    ]
    cpp.libraryPaths: [
        libraries + "/bullet3-2.83.7/bin",
        libraries + "/glew-1.13.0/lib/Release/Win32",
        //libraries + "/glew-1.13.0.build/build/cmake/build/lib",
        libraries + "/glfw-3.1.2.bin.WIN32/lib-mingw-w64",
    ]
    cpp.linkerFlags: [
        "-lBulletDynamics_gmake", "-lBulletCollision_gmake", "-lLinearMath_gmake",
        "-lglew32s",
        "-lglfw3", "-lgdi32", "-lopengl32",
    ]

    files: [
        "camera.cpp",
        "camera.h",
        "main.cpp",
        "helper.cpp",
        "helper.h",
        "global.h",
        "world.h",
    ]

    Group {
        name: "Include"
        files: [
            "include/*",
        ]
    }

    Group {
        name: "Objects src"
        files: [
            "cubeenclosed.cpp",
            "cubeenclosed.h",
            "skybox.cpp",
            "skybox.h",
            "sphere.*",
            "object.*",
            "cube.*",
            "wavefront.*",
        ]
    }

    Group {
        name: "Shaders"
        files: [
            "shaders/*",
        ]
    }

    Group {
        name: "Textures"
        files: [
            "images/*",
        ]
    }

    Group {
        name: "Models"
        files: [
            "models/simple.mtl",
            "models/simple.obj",
            "models/nanoMiku/nanoMiku.mtl",
            "models/nanoMiku/nanoMiku.obj",
            "models/arena/arena_01.mtl",
            "models/arena/arena_01.obj",
            "models/nanoMiku/tex1.png",
        ]
    }

    Group {
        name: "Miscellaneous files"
        files: [
            "config.lua",
            "README.md",
        ]
    }

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}
