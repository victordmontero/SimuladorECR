-- premake5.lua
workspace "SimuladorECR"
   configurations { "Debug", "Release" }
   platforms { "Win32", "Win64", "Linux" }
   location "proj_%{_ACTION}"

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

	includedirs({
		"include",
		os.getenv("WXWIN").."/include",
		os.getenv("WXWIN").."/include/msvc"
	})
	
	libdirs({
		os.getenv("WXWIN").."/lib/vc_lib"
	})

  filter  "platforms:Win64"
    defines{"WIN64"}
    system "windows"
    architecture "x64"

	includedirs({
		"include",
		os.getenv("WXWIN").."/include",
		os.getenv("WXWIN").."/include/msvc"
	})
	
	libdirs({
		os.getenv("WXWIN").."/lib/vc_lib"
	})

  filter  "platforms:Linux"
    defines{"LINUX"}
    system "linux"

	includedirs({
		"include"
	})

	buildoptions{"`wx-config --cxxflags`"}
	linkoptions{"`wx-config --libs`"}
