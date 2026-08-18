project "SpotiVolClient"
    kind "WindowedApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "Off"

    targetdir ("%{wks.location}/build/%{prj.name}/" .. outputdir)
    objdir ("%{wks.location}/build/%{prj.name}/" .. outputdir .. "/obj")
    debugdir ("%{prj.location}")

    files {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp"
    }

    includedirs {
        "%{IncludeDirs.SVNet}",
        "%{IncludeDirs.ImGui}"
    }

    libdirs {
        "%{LibraryDirs}.SVNet",
        "%{LibraryDirs.ImGui}"
    }

    links {
        "SVNet",
        "ImGui",
        "d3d11.lib",
        "dxgi.lib",
        "ws2_32.lib",
        "Ole32.lib"    
    }

    filter { 'system:windows' }
        files { 'Resources/**.rc', 'Resources/**.ico' }
        vpaths { ['Resources/*'] = { '*.rc', '**.ico' } }
    filter {}