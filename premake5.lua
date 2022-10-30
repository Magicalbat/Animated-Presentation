workspace "Animated Presentation"
    configurations {"Debug", "Release"}
    architecture "x64"
    startproject "Animated Presentation"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDirs = {}

project "Animated Presentation"
    location "Animated Presentation"
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

    links { }

    filter "system:windows"
        cdialect "C17"
        staticruntime "On"
        systemversion "latest"
        
        defines
        {
            "AP_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        buildoptions "/MDd"
        symbols "On"

        defines {
			"DEBUG", "AP_ASSERT"
        }

    filter "configurations:Release"
        buildoptions "/MD"
        optimize "On"
        defines "NDEBUG"
