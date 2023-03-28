workspace "Animated-Presentation"
    configurations { "Debug", "Release" }
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

function init_common() 
    language "C"

    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    targetdir ("bin/" .. outputdir)
    targetprefix ""

    filter "options:not wasm"
        architecture "x64"

    filter "options:not no-clang"
        toolset "clang"
        buildoptions { "-Wno-missing-braces" }

    filter { "options:wasm", "system:linux" }
        linkoptions { "--cache \"../../emcc-cache\""}

    filter "configurations:Debug"
        symbols "On"

        defines {
            "DEBUG", "AP_ASSERT"
        }

    filter "configurations:Release"
        optimize "On"
        defines { "NDEBUG" }

end

project "core"
    location "src/core"
    kind "SharedLib"

    includedirs {
        "src/%{prj.name}/include"
    }

    files {
        "src/%{prj.name}/**.h",
        "src/%{prj.name}/**.c",
    }

    init_common()

    if _OPTIONS["wasm"] then
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
                "-sSIDE_MODULE=2"
            }
            links { "m", "GL" }

            targetextension ".wasm"

            filter { "options:wasm", "system:linux" }
                linkoptions { "--cache \"../../emcc-cache\""}
    else
        filter "system:linux"
            links {
                "m", "X11", "GL", "GLX", "dl"
            }

        filter "system:windows"
            systemversion "latest"

            links {
                "gdi32", "kernel32", "user32", "winmm", "opengl32"
            }
    end

project "app"
    location "src/app"
    kind "ConsoleApp"
    
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

    init_common()

    if _OPTIONS["wasm"] then
        filter "options:wasm"
            buildoptions  {
                "-fPIC",
            }

            linkoptions {
                "-sFETCH=1",
                "-sWASM=1",
                "-sALLOW_MEMORY_GROWTH=1",
                "-sASYNCIFY=1",
                "-sASYNCIFY_IMPORTS=os_sleep_milliseconds",
                "-sFORCE_FILESYSTEM=1",
                "-sOFFSCREEN_FRAMEBUFFER=1",
                "-sMIN_WEBGL_VERSION=2",
                "-sMAIN_MODULE=2"
            }

            targetextension ".js"

        filter { "options:wasm", "system:linux" }
            linkoptions { "--cache \"../../emcc-cache\""}

    else
        -- TODO: Test if links are necessary
        filter "system:linux"
            links {
                "m", "X11", "GL", "GLX", "dl"
            }

        filter "system:windows"
            systemversion "latest"

    end

project "plugin_basic"
    location "plugins/basic"
    kind "SharedLib"

    files {
        "plugins/basic/**.h",
        "plugins/basic/**.c",
    }

    includedirs {
        "plugins/basic",
        "src/core/include",
        "src/app/include"
    }

    links {
        "core",
    }

    init_common()

    filter { }
        targetdir("bin/" .. outputdir .. "/plugins")

    if _OPTIONS["wasm"] then
        filter "options:wasm"
            buildoptions  {
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
                "-sSIDE_MODULE=2"
            }

            targetextension ".wasm"

        filter { "options:wasm", "system:linux" }
            linkoptions { "--cache \"../../emcc-cache\""}
    else
        filter "system:linux"
            links {
                "m", "X11", "GL", "GLX", "dl"
            }

        filter "system:windows"
            systemversion "latest"

            links {
                "gdi32", "kernel32", "user32", "winmm", "opengl32"
            }
    end

project "plugin_text"
    location "plugins/text"
    kind "SharedLib"

    files {
        "plugins/text/**.h",
        "plugins/text/**.c",
    }

    includedirs {
        "plugins/text",
        "src/core/include",
        "src/app/include"
    }

    links {
        "core",
    }

    init_common()

    filter { }
        targetdir("bin/" .. outputdir .. "/plugins")

    if _OPTIONS["wasm"] then
        filter "options:wasm"
            buildoptions  {
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
                "-sSIDE_MODULE=2"
            }

            targetextension ".wasm"

        filter { "options:wasm", "system:linux" }
            linkoptions { "--cache \"../../emcc-cache\""}
    else
        filter "system:linux"
            links {
                "m", "X11", "GL", "GLX", "dl"
            }

        filter "system:windows"
            systemversion "latest"

            links {
                "gdi32", "kernel32", "user32", "winmm", "opengl32"
            }
    end

