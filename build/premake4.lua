-- A solution contains projects, and defines the available configurations
solution "MinTriangle"
configurations {
    "Debug",
    "Release",
}

configuration "vs*"
do
    buildoptions {
        "/wd4996",
    }
end

configuration "windows"
do
    defines {
        "WIN32",
        "_WINDOWS",
    }
end

configuration "Debug"
do
    defines { "DEBUG" }
    flags { "Symbols" }
    libdirs {
        "Debug",
    }
    targetdir "debug"
end

configuration "Release"
do
    defines { "NDEBUG" }
    flags { "Optimize" }
    targetdir "release"
    libdirs {
        "Release",
    }
end

configuration {}

include "../01.CreateDevice"
include "../02.D2D_HUD"
include "../03.MinShaderPipeline"
include "../MinTriangle"

