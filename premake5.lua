include "Dependencies.lua"

workspace "SpotiVol"
architecture "x64"
startproject "SpotiVol"
configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
    include "Dependencies/ImGui"
    include "Dependencies/SVNet"
group ""

include "SpotiVol"
include "SpotiVolClient"