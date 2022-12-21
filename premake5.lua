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
        "m", "X11", "GL", "GLX"
      }

      defines
      {
        "AP_PLATFORM_LINUX"
      }

    --[[filter "system:windows"
        cdialect "C17"
        staticruntime "On"
        systemversion "latest"
        
        defines
        {
            "AP_PLATFORM_WINDOWS"
        }

        links 
        {
            "winmm", "opengl32"
        }]]

    filter "configurations:Debug"
        symbols "On"

        defines 
        {
			"DEBUG", "AP_ASSERT"
        }

    filter "configurations:Release"
        optimize "On"
        defines "NDEBUG"
