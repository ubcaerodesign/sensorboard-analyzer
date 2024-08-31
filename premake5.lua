workspace "MyProject"
    configurations { "Debug", "Release" }
    platforms { "x64" }

project "MyProject"
    kind "ConsoleApp"
    language "C"
    cdialect "C99"
    targetdir "bin/%{cfg.buildcfg}"

    files { "src/**.c" }
    includedirs { "include" }
    libdirs { "lib" }

    filter "platforms:x64"
        architecture "x86_64"

    filter "system:windows"
        links { "raylib", "opengl32", "gdi32", "winmm" }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
