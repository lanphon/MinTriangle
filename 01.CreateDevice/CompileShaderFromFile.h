#pragma once

#include <windows.h>
#include <string>
#include <d3dcommon.h>


struct StageInfo
{
    // "vsMain"
    std::string entrypoint;
    // "vs_4_0_level_9_1"
    std::string model;
};

struct ShaderInfo
{
    std::wstring path;
    StageInfo vs;
    StageInfo gs;
    StageInfo ps;
};


/// <returns>S_OK for success, or failure code</returns>
HRESULT CompileShaderFromFile(const std::wstring &szFileName
        , const StageInfo &info, ID3D10Blob** ppBlobOut);

