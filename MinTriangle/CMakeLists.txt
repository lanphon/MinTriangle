SET(TARGET MinTriangle)
FILE(GLOB SRC
    *.cpp
    *.h
    *.rc
    *.fx
    )
link_directories(
"C:/Users/BE HAPPY/source/repos/D3D11On12/build/src/Debug"
"C:/Users/BE HAPPY/source/repos/D3D12TranslationLayer/build/packages/WinPixEventRuntime.1.0.190604001/bin/x64"
)
ADD_EXECUTABLE(${TARGET} WIN32
    ${SRC}
    )
TARGET_COMPILE_DEFINITIONS(${TARGET} PUBLIC
    UNICODE
    _UNICODE
    _CRT_SECURE_NO_WARNINGS
    )
set(EXTRA_LIBS WinPixEventRuntime d3d12 dxgi d3d11on12)
TARGET_LINK_LIBRARIES(${TARGET}
    d3d11
    d3dcompiler
    dxguid
    ${EXTRA_LIBS}
    )
