workspace "Animated-Presentation"
    configurations {"Debug", "Release"}
    startproject "Animated Presentation"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

newoption {
    trigger = "wasm",
    description = "Choose whether or not to make build files for wasm",
}

project "Animated-Presentation"
    location "Animated-Presentation"
    kind "ConsoleApp"
    language "C"

    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")

    filter "options:wasm"
        targetname ("%{prj.name}.js")
    filter {}

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.c",
    }

    includedirs
    {
        "%{prj.name}/src",
    }

    defines 
    {
        "AP_OPENGL"
    }

    links { }

    filter { "system:linux", "options:not wasm" }
        links 
        {
            "m", "X11", "GL", "GLX", "dl"
        }
        buildoptions { }

    filter { "system:windows", "options:not wasm" }
        staticruntime "On"
        systemversion "latest"
        
        links 
        {
            "gdi32.lib", "kernel32", "user32", "winmm", "opengl32"
        }

    filter { "system:windows", "action:vs2022" }
        cdialect "C17"

    filter { "system:windows", "action:gmake or gmake2 or win_gmake" }
        toolset "clang"

    filter "options:wasm"
        buildoptions 
        {
            "-fPIC",
        }

        linkoptions
        {
            "-sFETCH=1",
            "-sWASM=1",
            "-sALLOW_MEMORY_GROWTH=1",
            "-sASYNCIFY=1",
            "-sFORCE_FILESYSTEM=1",
            "-sOFFSCREEN_FRAMEBUFFER=1",
            "-sMIN_WEBGL_VERSION=2",
            "-sMAIN_MODULE=1"
        }
        links { "m", "GL" }

    filter { "options:wasm", "system:linux" }
        linkoptions { "--cache \"../emcc-cache\""}

    filter "options:not wasm"
        architecture "x64"
    
    filter "configurations:Debug"
        symbols "On"

        defines 
        {
            "DEBUG", "AP_ASSERT"
        }

    filter "configurations:Release"
        optimize "On"
        defines "NDEBUG"