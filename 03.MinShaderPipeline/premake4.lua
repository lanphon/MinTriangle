project "03.MinShaderPipeline"
--language "C
language "C++"
--kind "StaticLib"
--kind "SharedLib"
--kind "ConsoleApp"
kind "WindowedApp"

flags {
    "Unicode",
    "WinMain",
}
files {
    "*.cpp",
    "*.rc",
    "*.h",
    "*.fx",
    "../01.CreateDevice/DXGIManager.cpp",
    "../01.CreateDevice/D3DManager.cpp",
    "../01.CreateDevice/D3DResource.cpp",
    "../01.CreateDevice/CompileShaderFromFile.cpp",
    "../01.CreateDevice/windowutil.cpp",
    "../02.D2D_HUD/D2DManager.cpp",
}
includedirs {
    "$(BOOST_DIR)",
}
buildoptions {
}
defines {
}
libdirs {
}
links {
    "d3d11",
    "d3dcompiler",
    "dxguid",
    "d2d1",
    "dwrite",
}

