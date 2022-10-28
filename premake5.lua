-- premake5.lua
workspace "SimuladorECR"
   configurations { "Debug", "Release" }
   platforms { "Win32", "Win64", "Linux" }

project "SimuladorECR"
   kind "WindowedApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   objdir "obj"

   files ({
		"**.h",
		"**.hpp",
		"src/**.cpp",
		"**.lua"
	})
	
	defines{
		"__WXMSW__"
	}

  filter "configurations:Debug"
    defines { "DEBUG", "_DEBUG" }
    symbols "On"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"

  filter "platforms:Win32"
    defines{"WIN32"}
    system "windows"
    architecture "x86"
	staticruntime "on"

	includedirs({
		"include",
		"../wxWidgets-3.2.1/include",
		"../wxWidgets-3.2.1/include/msvc"
	})
	
	libdirs({
		"../wxWidgets-3.2.1/lib/vc_lib_mt"
	})

  filter "platforms:Win64"
    defines{"WIN64"}
    system "windows"
    architecture "x86_64"
	staticruntime "on"

	includedirs({
		"include",
		"../wxWidgets-3.2.1/include",
		"../wxWidgets-3.2.1/include/msvc"
	})
	
	libdirs({
		"../wxWidgets-3.2.1/lib/vc_x64_lib_st"
	})

  filter "platforms:Linux"
    defines{"LINUX"}
    system "linux"

	includedirs({
		"include"
	})

	buildoptions{"`wx-config --cxxflags`"}
	linkoptions{"`wx-config --libs`"}
