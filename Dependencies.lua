IncludeDirs = {}

IncludeDirs["ImGui"] = "%{wks.location}/Dependencies/ImGui"
IncludeDirs["SVNet"] = "%{wks.location}/Dependencies/SVNet/src"

LibraryDirs = {}

LibraryDirs["ImGui"] = "%{wks.location}/build/ImGui/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
LibraryDirs["SVNet"] = "%{wks.location}/build/SVNet/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"