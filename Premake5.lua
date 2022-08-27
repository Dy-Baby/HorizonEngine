enginedir = "%{wks.location}/../Source/Engine"
function enginepath(path)
    return enginedir .. "/" .. path
end

editordir = "%{wks.location}/../Source/Editor"
function editorpath(path)
    return editordir .. "/" .. path
end

thirdpartydir = "%{wks.location}/../ThirdParty"
function thirdpartypath(path)
    return thirdpartydir .. "/" .. path
end

plugindir = "%{wks.location}/../Plugins"
function pluginpath(path)
    return plugindir .. "/" .. path
end

function sourcedirs(dirs)
    if type(dirs) ~= "table" then dirs = {dirs} end
    for _, dir in ipairs(dirs) do
    files {
        dir .. "/**.h",  
        dir .. "/**.c", 
        dir .. "/**.hpp",
        dir .. "/**.cpp",
        dir .. "/**.cppm",
        dir .. "/**.inl",
        dir .. "/**.hsf",
    }
    end
end

function plugin(name)
    project(name)
        kind "SharedLib"
        language "C++"
        cppdialect "C++20"
        location "%{wks.location}/Plugins/%{prj.name}"
        targetdir "%{wks.location}/Bin/%{cfg.buildcfg}"
        dependson { 
            "Engine",
        }
        sourcedirs {
            "Plugins/" .. name
        }
end

workspace "Horizon"
    location "Build"
    configurations {
		"Debug",
		"Release",
	}
	flags {
		"MultiProcessorCompile",
	}
    startproject "EditorLauncher"

filter { 'files:**.cppm' }
    buildaction 'ClCompile'

filter "configurations:Debug"
    defines { 
        "HE_CONFIG_DEBUG",
        "DEBUG",
    }
    symbols "On"

filter "configurations:Release"
    defines { 
        "HE_CONFIG_RELEASE"
    }
    optimize "On"

 
filter "system:windows"
    platforms "Win64"
    systemversion "latest"

filter "platforms:Win64"
    defines { 
        "HE_PLATFORM_WINDOWS",
        "_CRT_SECURE_NO_WARNINGS",
        "_ITERATOR_DEBUG_LEVEL=0",
        "_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING",
    }
    staticruntime "On"
    architecture "x64"
    buildoptions {
        "/wd5105",
        "/utf-8",
    }
    linkoptions {
        "/ignore:4006",
    }
    disablewarnings {
    }

group "Source"
    include "Source/Engine"
    include "Source/Editor"
    include "Source/EditorLauncher"
group ""

group "ThirdParty"
    include "ThirdParty/yaml-cpp"
group ""

group "Samples"
    include "Samples/CaptureFrames"
    include "Samples/SkyAtmosphere"
    include "Samples/ModelViewer"
    include "Samples/PathTracing"
    include "Samples/RealTimeRayTracing"
group ""