# Fonctions de base pour générer avec CMake en INF1015.  Par Francois-R.Boyer@PolyMtl.ca
# Inclure ce fichier avant "project( )", après "cmake_minimum_required( )" et certains "set" globaux.

set(CMAKE_CXX_STANDARD 20)    # On compile en C++20,
set(CMAKE_CXX_EXTENSIONS OFF) # sans les extensions de GNU.


# La raison pour laquelle on fait une variable d'environnement VCPKG_ROOT.
# S'il n'arrive pas à détecter le bon triplet, la variable d'environnement VCPKG_TARGET_TRIPLET peut aussi être configurée.
if (DEFINED ENV{VCPKG_ROOT})
	message(STATUS "VCPKG_ROOT: $ENV{VCPKG_ROOT}")
	if (DEFINED ENV{VCPKG_TARGET_TRIPLET})
		message(VERBOSE "VCPKG_TARGET_TRIPLET: $ENV{VCPKG_TARGET_TRIPLET}")
		set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_TARGET_TRIPLET}")
	endif()
	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()


# Affiche quelques variables utiles s'il y a un problème avec vcpkg.  Certaines variables existent juste après la commande "project", qui utilise le "toolchain", donc il faut appeler cette fonction après "project".
function(afficher_infos_generateur)
	# On peut utiliser VERBOSE au lieu de STATUS, qui a besoin de CMake 3.15 (juillet 2019), et forcer ce log level.
	message(VERBOSE "CMake      generator: ${CMAKE_GENERATOR}")
	message(VERBOSE "            platform: ${CMAKE_GENERATOR_PLATFORM}")
	message(VERBOSE "    host system name: ${CMAKE_HOST_SYSTEM_NAME}")
	if (NOT CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
		message(VERBOSE "         system name: ${CMAKE_SYSTEM_NAME}")
	endif()
	if (NOT CMAKE_CXX_PLATFORM_ID STREQUAL CMAKE_HOST_SYSTEM_NAME)
		message(VERBOSE "        C++ platform: ${CMAKE_CXX_PLATFORM_ID}")
	endif()
	message(VERBOSE " C++ compiler ID+Ver: ${CMAKE_CXX_COMPILER_ID}   ${CMAKE_CXX_COMPILER_VERSION}")
	if (CMAKE_SIZEOF_VOID_P)
		math(EXPR COMPILER_BITNESS "${CMAKE_SIZEOF_VOID_P} * 8")
		message(VERBOSE "      target bitness: ${COMPILER_BITNESS} bit")
	endif()
	message(VERBOSE "      host processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
	if (DEFINED ENV{VCPKG_ROOT})
		message(STATUS "VCPKG triplet: ${VCPKG_TARGET_TRIPLET}")
	endif()
endfunction()

# Pour Qt (basé sur https://doc.qt.io/qt-5/cmake-get-started.html )
# Doit être fait avant le "add_executable" pour que le compilateur MOC soit exécuté; le build devrait afficher un message comme "Automatic MOC and UIC for target".
macro(configurer_Qt_auto_tout)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	set(CMAKE_AUTOUIC ON)
	if (CMAKE_VERSION VERSION_LESS "3.7.0")
		set(CMAKE_INCLUDE_CURRENT_DIR ON)
	endif()
endmacro()

# Affiche des informations sur Qt; doit être après le find_package(Qt5 ...)
function(afficher_version_Qt)
	if (DEFINED Qt6_DIR)
		message(STATUS  "Qt6   dir: ${Qt6_DIR}")
		message(VERBOSE "  version: ${Qt6_VERSION}")
	elseif (DEFINED Qt5_DIR)
		message(STATUS  "Qt5   dir: ${Qt5_DIR}")
		message(VERBOSE "  version: ${Qt5_VERSION}")
	else()
		message(STATUS  "No Qt?_dir")
	endif()
endfunction()

function(afficher_toutes_variables)
	# De https://stackoverflow.com/a/9328525
	get_cmake_property(_variableNames VARIABLES)
	list (SORT _variableNames)
	foreach (_variableName ${_variableNames})
		message(STATUS "${_variableName}=${${_variableName}}")
	endforeach()
endfunction()

# Les options de compilation (add_compile_options doit être avant add_executable)
macro(configurer_options_compilation_globales)
	if (NOT MSVC)
		# Les flags de compilation. Ceux-ci reproduisent un peu le comportement de MSVC avec /W4.
		# (Voir https://stackoverflow.com/a/9862800  pour une bonne liste d'options d'avertissement.)
		add_compile_options(-fsigned-char -Wall -Wextra -Wpedantic -Woverloaded-virtual -Wno-unknown-pragmas -Wno-enum-compare -Wshadow)
	else()
		string(REGEX REPLACE "(^| )/W[0-4]" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
		# message(STATUS "CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} ; COMPILE_OPTIONS=${COMPILE_OPTIONS}")
		add_compile_options(/nologo /Zi /W4 /permissive- )
		if (CMAKE_VERSION VERSION_LESS "3.13.0")
		else()
			add_link_options(/nologo)
		endif()
	endif()
	# afficher_toutes_variables()
endmacro()

# Ajoute une "custom target" pour exécuter gcov.  Doit être après le "add_executable" et doit lui passer le nom de l'exécutable.
function(ajouter_target_couverture EXECUTABLE_NAME)
	if (NOT MSVC)
		target_compile_options(${EXECUTABLE_NAME} PRIVATE --coverage)
		if (CMAKE_VERSION VERSION_LESS "3.13.0")
			set_target_properties(${EXECUTABLE_NAME} PROPERTIES COMPILE_OPTIONS --coverage)
		else()
			target_link_options(${EXECUTABLE_NAME} PRIVATE --coverage)
		endif()

		# Efface les fichiers .gcda (couveture de code) après le build car ils ne correspondent plus au nouveau build. 
		add_custom_command(
			TARGET ${EXECUTABLE_NAME}
			POST_BUILD
			COMMAND rm -f "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${EXECUTABLE_NAME}.dir/*.gcda"
			COMMAND rm -f "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${EXECUTABLE_NAME}.dir/**/*.gcda"
		)

		# Si on trouve gcov, on fait une cible avec nom ..._couveture pour exécuter gcov et obtenir la couverture de code.
		find_program(GCOV_PATH gcov)
		if (GCOV_PATH)
			add_custom_target(${EXECUTABLE_NAME}_couverture
				# Vous devez exécuter le programme avant de "build" cette cible, ou décommenter la ligne suivante qui l'exécute mais ne fonctionne pas si le programme a besoin d'interagir avec l'usager:
				# COMMAND ${EXECUTABLE_NAME}
			 
				# Analyse le fichier de couverture:
				COMMAND ${GCOV_PATH} -s "${ProjetTest_SOURCE_DIR}" -r -f -m "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${EXECUTABLE_NAME}.dir/*.gcda"

				WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
				DEPENDS ${EXECUTABLE_NAME}
			)
		endif()
	else()
		# Dans MSVC, le répertoire pour CodeCoverage.h n'est pas par défaut dans INCLUDE en ligne de commande (mais elle l'est dans l'environnement VS).
		if (DEFINED ENV{VCINSTALLDIR} AND EXISTS "$ENV{VCINSTALLDIR}Auxiliary/VS/include")
			target_include_directories(${EXECUTABLE_NAME} SYSTEM PRIVATE "$ENV{VCINSTALLDIR}Auxiliary/VS/include")
		endif()
	endif()
endfunction()

macro(configurer_GoogleTest EXECUTABLE_NAME)
	# Pour Google Test (basé sur https://stackoverflow.com/questions/1620918/cmake-and-libpthread )
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	find_package(Threads REQUIRED)
	target_link_libraries(${EXECUTABLE_NAME} Threads::Threads)
	add_compile_definitions(_POSIX_C_SOURCE=200809L)  # Requis pour compiler GoogleTest sous Cygiwin, voir https://github.com/google/googletest/issues/813
endmacro()