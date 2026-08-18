project "SpotiVol"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "Off"

    targetdir ("%{wks.location}/build/%{prj.name}/" .. outputdir)
    objdir ("%{wks.location}/build/%{prj.name}/" .. outputdir .. "/obj")
    debugdir ("%{prj.location}/")

    files {
        "**.h",
        "**.hpp",
        "**.cpp"
    }

    includedirs {
        "%{IncludeDirs.SVNet}"
    }

    libdirs {
        "%{LibraryDirs.SVNet}"
    }

    links {
        "ImGui",
        "SVNet",
        "ws2_32.lib",
        "Ole32.lib",
        "comctl32.lib"
    }

    flags  { 
        "NoPCH"
    }