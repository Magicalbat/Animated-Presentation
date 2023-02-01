workspace "Animated-Presentation"
    configurations {"Debug", "Release"}
    architecture "x64"
    startproject "Animated Presentation"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDirs = {}

project "Animated-Presentation"
    location "Animated-Presentation"
    kind "ConsoleApp"
    language "C"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

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

    filter "system:linux"
        links 
        {
            "m", "X11", "GL", "GLX", "dl"
        }
        buildoptions
        {
            --"-Wall"
        }

    filter "system:windows"
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

    filter "configurations:Debug"
        symbols "On"

        defines 
        {
            "DEBUG", "AP_ASSERT"
        }

    filter "configurations:Release"
        optimize "On"
        defines "NDEBUG"
