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
        "helper.h",
        "global.h",
        "world.h",
    ]

    Group {
        name: "Single header libraries"
        files: [
            "stb_image.h",
            "tiny_obj_loader.h",
        ]
    }

    Group {
        name: "Object source files"
        files: [
            "sphere.cpp",
            "object.cpp",
            "cube.cpp",
            "sphere.h",
            "object.h",
            "cube.h",
            "wavefront.cpp",
            "wavefront.h",
        ]
    }

    Group {
        name: "Vertex shaders"
        files: [
            "basic.vert",
            "lighting.vert",
            "texture.vert",
            "wavefront.vert",
        ]
    }

    Group {
        name: "Fragment shaders"
        files: [
            "basic.frag",
            "lighting.frag",
            "texture.frag",
            "wavefront.frag",
        ]
    }

    Group {
        name: "Textures"
        files: [
            "diamond_block.png",
            "earth.jpg",
            "firemap.png",
        ]
    }

    Group {
        name: "Models: simple"
        files: [
            "models/simple.mtl",
            "models/simple.obj",
        ]
    }

    Group {
        name: "Models: nanoMiku"
        files: [
            "models/nanoMiku.mtl",
            "models/nanoMiku.obj",
        ]
    }

    Group {
        name: "Miscellaneous files"
        files: [
            "config.lua",
        ]
    }

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}
