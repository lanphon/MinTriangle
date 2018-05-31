SET(TARGET MinTriangle)
FILE(GLOB SRC
    *.cpp
    *.h
    *.rc
    *.fx
    )
ADD_EXECUTABLE(${TARGET} WIN32
    ${SRC}
    )
TARGET_COMPILE_DEFINITIONS(${TARGET} PUBLIC
    UNICODE
    _UNICODE
    )
TARGET_LINK_LIBRARIES(${TARGET}
    d3d11
    d3dcompiler
    dxguid
    )
