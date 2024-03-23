#pragma once
/// Ajout des numéros de ligne au rapport de fuites.
/// Pour aller avec verification_allocation.cpp .  Doit être inclus dans tous les fichiers qui font des "new".
/// Malheureusement incompatible avec le "placement new" et l'appel direct à l'"operator new".
/// \author Francois-R.Boyer@PolyMtl.ca
/// \version 2022-03
/// \since   2021-01

void* operator new  (size_t sz, const char* nom_fichier, int ligne_fichier);
void* operator new[](size_t sz, const char* nom_fichier, int ligne_fichier);
void operator delete  (void* ptr, const char* nom_fichier, int ligne_fichier) noexcept;
void operator delete[](void* ptr, const char* nom_fichier, int ligne_fichier) noexcept;

// Série de macros compliquées pour faire l'équivalent de #define new new( __FILE__ , __LINE__ )
// mais que new_ permet d'accéder au mot clé new original.  (Utilise __VA_OPT__ de C++20)
#if !defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL
// On n'a pas trouvé de manière de le faire avec l'ancien préprocesseur de VisualStudio
// utiliser l'option /Zc:preprocessor (activer "Utiliser un préprocesseur conforme aux normes" dans les options de projet C/C++ > Préprocesseur); pour ne pas avoir de "warning" avec cette option, il faut au moins le SDK version 2104 (10.0.20348.0), voir https://developercommunity2.visualstudio.com/t/std:c17-generates-warning-compiling-Win/1249671?preview=true
#define new new(__FILE__, __LINE__ )
#else
#define OPEN_PARENTHESIS (
#define CLOSE_PARENTHESIS )
#define EAT_FIRST_ARG(first,...) __VA_ARGS__
#define EXPAND_AGAIN(X) X
#define IF_NON_RECURIVE_USE(macro_to_test_for_recursive_use,...) EXPAND_AGAIN(EAT_FIRST_ARG OPEN_PARENTHESIS macro_to_test_for_recursive_use(,__VA_ARGS__) CLOSE_PARENTHESIS)
#define NEW_(...) new __VA_ARGS__
#define new new IF_NON_RECURIVE_USE(NEW_,(__FILE__, __LINE__))
#define new_ NEW_()
#endif
