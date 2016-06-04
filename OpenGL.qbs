import qbs

Product {
    property string libraries: "D:\\Programs\\misc\\OpenGL"
    type: "application"
    Depends {name: "cpp"}

    consoleApplication: true
    cpp.cxxLanguageVersion: "c++11"
    cpp.defines: ["GLEW_STATIC"]
    cpp.includePaths: [
        "include",
        libraries + "/stb",
        libraries + "/glm",
        libraries + "/glew-1.13.0/include",
        libraries + "/glfw-3.1.2.bin.WIN32/include",
    ]
    cpp.libraryPaths: [
        libraries + "/glew-1.13.0/lib/Release/Win32",
        libraries + "/glfw-3.1.2.bin.WIN32/lib-mingw-w64",
    ]
    cpp.staticLibraries: [
        "glew32s", "glfw3",
    ]
    cpp.dynamicLibraries: [
        "gdi32", "opengl32",
    ]

    Group {
        name: "Header files"
        files: [
            "include/*",
        ]
    }

    Group {
        name: "Source files"
        files: [
            "*.cpp",
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

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}
