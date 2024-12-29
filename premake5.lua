workspace "SimpleNES" -- sln文件名
    architecture "x64"
    configurations { "Debug", "Release" }

IncludeDir={}
IncludeDir["SFML"]="vendor/SFML/include"

LibDir={}
LibDir["SFML"]="vendor/SFML/lib"

project "SimpleNES"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    targetdir("bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")
    objdir("bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}")

    files {
        "include/**.h",
        "src/**.cpp",
    }

    includedirs {
        "include",
        "src",
        "%{IncludeDir.SFML}"
    }

    defines{
        "_CRT_SECURE_NO_WARNINGS"
    }

    libdirs{
        "%{LibDir.SFML}"
    }

    links{
        "sfml-graphics",
        "sfml-window",
        "sfml-system",
        "sfml-audio"
    }

    filter "system:windows"
        systemversion "latest"
        postbuildcommands {"{COPY} %{cfg.buildtarget.relpath} %{wks.location}NESEmulator/ "}

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"