-- premake5.lua
workspace "SimuladorECR"
   configurations { "Debug", "Release" }
   platforms { "Win32", "Win64", "Linux" }

project "SimuladorECR"
   kind "WindowedApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   objdir "obj"
   
   dependson {"wxWidgets"}

   files ({
		"**.h",
		"**.hpp",
		"src/**.cpp",
		"**.lua"
	})
	
	defines{"wxCFG=_st"}

  filter "configurations:Debug"
    defines { "DEBUG", "_DEBUG" }
    symbols "On"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"

  filter "platforms:Win32"
    defines{"__WXMSW__","WIN32"}
    system "windows"
    architecture "x86"
	staticruntime "on"

	includedirs({
		"include",
		"thirdparty/wxWidgets/include",
		"thirdparty/wxWidgets/include/msvc"
	})
	
	libdirs({
		"thirdparty/wxWidgets/lib/vc_lib_st"
	})

  filter "platforms:Win64"
    defines{"__WXMSW__","WIN64"}
    system "windows"
    architecture "x86_64"
	staticruntime "on"

	includedirs({
		"include",
		"thirdparty/wxWidgets/include",
		"thirdparty/wxWidgets/include/msvc"
	})
	
	libdirs({
		"thirdparty/wxWidgets/lib/vc_x64_lib_st"
	})

  filter "platforms:Linux"
    defines{"LINUX"}
    system "linux"

	includedirs({
		"include"
	})

	buildoptions{"`wx-config --cxxflags`"}
	linkoptions{"`wx-config --libs`"}

project "wxWidgets"
   kind "Makefile"
   objdir()
   
   location("thirdparty/wxWidgets")
   targetdir("%{prj.location}/lib")
   includedirs{}

   cleancommands {
	   
   }
   
   prebuildcommands {
		"git submodule update --init --recursive --remote %{prj.location}"
   }
   
   filter "platforms:Win32"
   
	   buildcommands {
		  "%comspec% /k \"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvars32.bat\"",
		  "{CHDIR} %{prj.location}build/msw",
		  "nmake /f makefile.vc BUILD=%{cfg.buildcfg:lower()} RUNTIME_LIBS=static CFG=_st TARGET_CPU=X86"
	   }
   
	   rebuildcommands {
		  "%comspec% /k \"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvars32.bat\"",
		  "{CHDIR} %{prj.location}build/msw",
		  "nmake /f makefile.vc BUILD=%{cfg.buildcfg:lower()} RUNTIME_LIBS=static CFG=_st TARGET_CPU=X86"
	   }
   
   filter "platforms:Win64"
   
	   buildcommands {
		  "%comspec% /k \"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\"",
		  "{CHDIR} %{prj.location}build/msw",
		  "nmake /f makefile.vc BUILD=%{cfg.buildcfg:lower()} RUNTIME_LIBS=static CFG=_st TARGET_CPU=X64"
	   }
   
	   rebuildcommands {
		  "%comspec% /k \"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\"",
		  "{CHDIR} %{prj.location}build/msw",
		  "nmake /f makefile.vc BUILD=%{cfg.buildcfg:lower()} RUNTIME_LIBS=static CFG=_st TARGET_CPU=X64"
	   }

	filter "platforms:Linux"
	
		buildcommands {
		  "{CHDIR} %{prj.location}",
		  "./configure --disable-shared",
		  "make -j$(nproc)"
	   }
   
	   rebuildcommands {
		  "{CHDIR} %{prj.location}",
		  "./configure --disable-shared",
		  "make -j$(nproc)"
	   }