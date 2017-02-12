import qbs

Product {
    type: "application"
    Depends {name: "cpp"}

    consoleApplication: true
    cpp.cxxLanguageVersion: "c++11"
    property stringList includePaths: ["include"]
    cpp.commonCompilerFlags: ["-Wno-unused-variable", "-Wno-unused-parameter"]

    Properties {
        condition: qbs.targetOS.contains("linux")
        cpp.includePaths: includePaths.concat([
            "/usr/include/bullet",
        ])
        cpp.staticLibraries: [
            "glfw3"
        ]
        cpp.dynamicLibraries: [
            "GLEW", "glfw", "GL",
            "BulletDynamics", "BulletCollision", "LinearMath",
            "X11", "Xxf86vm", "Xrandr", "Xinerama", "Xcursor",
            "dl", "pthread",
        ]
    }

    Properties {
        condition: qbs.targetOS.contains("windows")

        property string libraries: "D:\\Programs\\misc\\OpenGL"
        cpp.defines: ["GLEW_STATIC"]
        cpp.includePaths: includePaths.concat([
            libraries + "/stb",
            libraries + "/glm",
            libraries + "/glew-1.13.0/include",
            libraries + "/glfw-3.1.2.bin.WIN32/include",
            libraries + "/bullet3-2.83.7/src",
        ])
        cpp.libraryPaths: [
            libraries + "/bullet3-2.83.7/bin",
            libraries + "/glew-1.13.0/lib/Release/Win32",
            libraries + "/glfw-3.1.2.bin.WIN32/lib-mingw-w64",
        ]
        cpp.staticLibraries: [
            "BulletDynamics_gmake", "BulletCollision_gmake", "LinearMath_gmake",
            "glew32s", "glfw3",
        ]
        cpp.dynamicLibraries: [
            "gdi32", "opengl32",
        ]
    }

    Group {
        name: "Data files"
        files: [
            "data/**",
        ]
    }

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

    Group {
        name: "Models"
        files: [
            "models/**",
        ]
    }

    Group {
        name: "Miscellaneous files"
        files: [
            "config.lua",
            "screenshot.jpg",
            "readme.txt",
            "credits.txt",
        ]
    }

    Group {     // Properties for the produced executable
        fileTagsFilter: product.type
        qbs.install: true
    }
}
