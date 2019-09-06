-- premake5.lua
workspace "SimuladorECR"
   configurations { "Debug", "Release" }
   platforms { "Win32", "Win64", "Linux" }
   location "proj_%{_ACTION}"

project "SimuladorECR"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   objdir "obj"
   
   includedirs({
		"include",
		--"../wxWidgetsSrc/include",
		--"../wxWidgetsSrc/include/msvc"
	})

   files ({
		"**.h",
		"**.hpp",
		"src/**.cpp",
		"**.lua"
	})

   buildoptions{"`wx-config --cxxflags`"}
   linkoptions{"`wx-config --libs`"}


  filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"

  filter  "platforms:Win32"
    defines{"WIN32"}
    system "windows"
    architecture "x32"

  filter  "platforms:Win64"
    defines{"WIN64"}
    system "windows"
    architecture "x64"

  filter  "platforms:Linux"
    defines{"LINUX"}
    system "linux"
