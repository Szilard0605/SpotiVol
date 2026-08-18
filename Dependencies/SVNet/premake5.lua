project "SVNet"
    kind "StaticLib"
    language "C++"
    cppdialect "C++14"
    staticruntime "Off"

    targetdir ("%{wks.location}/build/%{prj.name}/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}")

    files {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp"
    }
