workspace "Animated-Presentation"
    configurations {"Debug", "Release"}
    startproject "app"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

newoption {
    trigger = "no-clang",
    description = "Avoids using clang."
}

newoption {
    trigger = "wasm",
    description = "Choose whether or not to make build files for wasm",
}

project "core"
    location "src/core"
    kind "SharedLib"
    language "C"

    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")

    files {
        "src/%{prj.name}/**.h",
        "src/%{prj.name}/**.c",
    }

    includedirs {
        "src/%{prj.name}/include"
    }

    defines { "AP_OPENGL" }

    filter "options:not wasm"
        architecture "x64"

    filter "options:not no-clang"
        toolset "clang"

    filter { "system:linux", "options:not wasm" }
        links {
            "m", "X11", "GL", "GLX", "dl"
        }

    filter { "system:windows", "not options: wasm" }
        systemversion "latest"

        links {
            "gdi32.lib", "kernel32", "user32", "winmm", "opengl32"
        }

    filter "options:wasm"
        buildoptions {
            "-fPIC",
        }

        linkoptions {
            "-sFETCH=1",
            "-sWASM=1",
            "-sALLOW_MEMORY_GROWTH=1",
            "-sASYNCIFY=1",
            "-sFORCE_FILESYSTEM=1",
            "-sOFFSCREEN_FRAMEBUFFER=1",
            "-sMIN_WEBGL_VERSION=2",
            "-sSIDE_MODULE=1"
        }
        links { "m", "GL" }

    filter { "options:wasm", "system:linux" }
        linkoptions { "--cache \"../emcc-cache\""}

    filter "configurations:Debug"
        symbols "On"

        defines {
            "DEBUG", "AP_ASSERT"
        }

    filter "configurations:Release"
        optimize "On"
        defines { "NDEBUG" }

project "app"
    location "src/app"
    kind "ConsoleApp"
    language "C"

    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")

    files {
        "src/%{prj.name}/**.h",
        "src/%{prj.name}/**.c",
    }

    includedirs {
        "src/%{prj.name}/include",
        "src/core/include"
    }

    links {
        "core",
    }

    defines { "AP_OPENGL" }


    filter "options:not wasm"
        architecture "x64"

    filter "options:not no-clang"
        toolset "clang"

    filter { "system:linux", "options:not wasm" }
        postbuildcommands {
            "{COPYFILE} %{wks.location}/bin/" .. outputdir .. "/core/libcore.so %{wks.location}/bin/" .. outputdir .. "/app/libcore.so"
        }


        links {
            "m", "X11", "GL", "GLX", "dl"
        }

    filter { "system:windows", "not options: wasm" }
        systemversion "latest"

        postbuildcommands {
            "{COPYFILE} %{wks.location}/bin/" .. outputdir .. "/core/core.dll %{wks.location}/bin/" .. outputdir .. "/app/core.dll"
        }

        links {
            "gdi32.lib", "kernel32", "user32", "winmm", "opengl32"
        }

    filter "options:wasm"
        buildoptions  {
            --"-fPIC",
        }

        linkoptions {
            --"-sFETCH=1",
            "-sWASM=1",
            --"-sALLOW_MEMORY_GROWTH=1",
            --"-sASYNCIFY=1",
            --"-sFORCE_FILESYSTEM=1",
            --"-sOFFSCREEN_FRAMEBUFFER=1",
            --"-sMIN_WEBGL_VERSION=2",
            "-sMAIN_MODULE=1"
        }

    filter { "options:wasm", "system:linux" }
        linkoptions { "--cache \"../emcc-cache\""}

    filter "configurations:Debug"
        symbols "On"

        defines {
            "DEBUG", "AP_ASSERT"
        }

    filter "configurations:Release"
        optimize "On"
        defines { "NDEBUG" }
