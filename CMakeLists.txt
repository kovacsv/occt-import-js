cmake_minimum_required (VERSION 3.6)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

set (CMAKE_SUPPRESS_REGENERATION 1)
set (CMAKE_CONFIGURATION_TYPES Debug;Release;RelWithDebInfo;MinSizeRel)

add_definitions (-DUNICODE -D_UNICODE)

project (OcctImportJS)

function (SetCompilerOptions module)
	target_compile_options (${module} PUBLIC "$<$<CONFIG:Debug>:-DDEBUG>")
	if (WIN32)
		set (AdditionalWarnings /w44061 /w44062 /w44265 /w44266 /w44355 /w44596 /w44800)
		target_compile_options (${module} PUBLIC /W3 ${AdditionalWarnings})
	else ()
		target_compile_options (${module} PUBLIC -std=c++11)
	endif ()
endfunction ()

# OcctImportJS

set (OcctImportJSSourcesFolder occt-import-js/src)
file (GLOB
	OcctImportJSSourceFiles
	${OcctImportJSSourcesFolder}/*.hpp
	${OcctImportJSSourcesFolder}/*.cpp
)
source_group ("Sources" FILES ${OcctImportJSSourceFiles})
add_library (OcctImportJS ${OcctImportJSSourceFiles})

SetCompilerOptions (OcctImportJS)

set_target_properties(OcctImportJS PROPERTIES OUTPUT_NAME occt-import-js)
set_target_properties (OcctImportJS PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

# OcctImportJSExample

set (OcctImportJSExampleSourcesFolder occt-import-js/example)
file (GLOB
	OcctImportJSExampleSourceFiles
	${OcctImportJSExampleSourcesFolder}/*.hpp
	${OcctImportJSExampleSourcesFolder}/*.cpp
)
source_group ("Sources" FILES ${OcctImportJSExampleSourceFiles})
add_executable (OcctImportJSExample ${OcctImportJSExampleSourceFiles})
SetCompilerOptions (OcctImportJSExample)
target_include_directories (OcctImportJSExample PUBLIC ${OcctImportJSSourcesFolder})
target_link_libraries (OcctImportJSExample OcctImportJS)
set_target_properties (OcctImportJSExample PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
