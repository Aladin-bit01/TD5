// Solutionnaire du TD3 INF1015 hiver 2024
// Par Francois-R.Boyer@PolyMtl.ca
#pragma once
// Structures mémoires pour une collection de films.

#include <string>
#include <memory>
#include <functional>
#include <cassert>
#include "gsl/span"
#include <iostream>
using gsl::span;
using namespace std;

struct Film; struct Acteur; // Permet d'utiliser les types alors qu'ils seront défini après.


struct Acteur
{
	string nom; int anneeNaissance = 0; char sexe = '\0';
};



class ListeFilms {
public:
	ListeFilms() = default;
	void ajouterFilm(Film* film);
	void enleverFilm(const Film* film);
	shared_ptr<Acteur> trouverActeur(const string& nomActeur) const;
	span<Film*> enSpan() const;
	int size() const { return nElements; }
	void detruire(bool possedeLesFilms = false);
	Film*& operator[] (int index) { assert(0 <= index && index < nElements);  return elements[index]; }
	Film* trouver(const function<bool(const Film&)>& critere) {
		for (auto& film : enSpan())
			if (critere(*film))
				return film;
		return nullptr;
	}

private:
	void changeDimension(int nouvelleCapacite);

	int capacite = 0, nElements = 0;
	Film** elements = nullptr; // Pointeur vers un tableau de Film*, chaque Film* pointant vers un Film.
};

template <typename T>
class Liste {
public:
	Liste() = default;
	explicit Liste(int capaciteInitiale) :  // explicit n'est pas matière à ce TD, mais c'est un cas où c'est bon de l'utiliser, pour ne pas qu'il construise implicitement une Liste à partir d'un entier, par exemple "maListe = 4;".
		capacite_(capaciteInitiale),
		elements_(make_unique<shared_ptr<T>[]>(capacite_))
	{
	}
	Liste(const Liste<T>& autre) :
		capacite_(autre.nElements_),
		nElements_(autre.nElements_),
		elements_(make_unique<shared_ptr<T>[]>(nElements_))
	{
		for (int i = 0; i < nElements_; ++i)
			elements_[i] = autre.elements_[i];
	}
	//NOTE: On n'a pas d'operator= de copie, ce n'était pas nécessaire pour répondre à l'énoncé. On aurait facilement pu faire comme dans les notes de cours et définir l'operator= puis l'utiliser dans le constructeur de copie.
	//NOTE: Nos constructeur/operator= de move laissent la liste autre dans un état pas parfaitement valide; il est assez valide pour que la destruction et operator= de move fonctionnent, mais il ne faut pas tenter d'ajouter, de copier ou d'accéder aux éléments de cette liste qui "n'existe plus". Normalement le move sur les classes de la bibliothèque standard C++ laissent les objets dans un "valid but unspecified state" (https://en.cppreference.com/w/cpp/utility/move). Pour que l'état soit vraiment valide, on devrait remettre à zéro la capacité et nombre d'éléments de l'autre liste.
	Liste(Liste<T>&&) = default;  // Pas nécessaire, mais puisque c'est si simple avec unique_ptr...
	Liste<T>& operator= (Liste<T>&&) noexcept = default;  // Utilisé pour l'initialisation dans lireFilm.


	Liste<T>& operator= (const Liste<T>& autre) {
		if (this == &autre) return *this;
		capacite_ = autre.capacite_;
		nElements_ = autre.nElements_;
		elements_ = make_unique<shared_ptr<T>[]>(capacite_);
		for (int i = 0; i < nElements_; ++i) {
			elements_[i] = autre.elements_[i];
		}
		return *this;
	}

	void ajouter(shared_ptr<T> element)
	{
		assert(nElements_ < capacite_);  // Comme dans le TD précédent, on ne demande pas la réallocation pour ListeActeurs...
		elements_[nElements_++] = move(element);
	}
	// Noter que ces accesseurs const permettent de modifier les éléments; on pourrait vouloir des versions const qui retournent des const shared_ptr, et des versions non const qui retournent des shared_ptr.  En C++23 on pourrait utiliser "deducing this".
	shared_ptr<T>& operator[] (int index) const { assert(0 <= index && index < nElements_); return elements_[index]; }
	span<shared_ptr<T>> enSpan() const { return span(elements_.get(), nElements_); }

private:
	int capacite_ = 0, nElements_ = 0;
	unique_ptr<shared_ptr<T>[]> elements_;
};

using ListeActeurs = Liste<Acteur>;

//Nouvelle classe interface Affichable
class Affichable {
public : 
	virtual void afficher(ostream& os) const = 0;
	virtual ~Affichable() = default;
};

//Nouvelle Classe Item
class Item : public Affichable
{
public:
	virtual ~Item() override = default;
	Item(){}
	Item (string titre, int anneeSortie):titre_(titre), anneeSortie_(anneeSortie){}
	friend shared_ptr<Acteur> ListeFilms::trouverActeur(const string& nomActeur) const;
	friend Film* lireFilm(istream& fichier, ListeFilms& listeFilms);
	const string accesTitre() const { return titre_; }
	int accesAnneeSortie() const { return anneeSortie_; }
	void setTitre(string titre) { titre_ = titre; }
	void setAnneeSortie(int anneeSortie) { anneeSortie_ = anneeSortie; }
	void afficher(ostream& os) const override {
		os << "Titre: " << titre_ << ", Date: " << anneeSortie_ << endl; 
	}
	private:
		string titre_;
		int anneeSortie_;
};


//Struct Film transforme en classe qui herite de ITEM
class Film: virtual public Item
{
	public:
		Film(){}
		Film(string titre, int anneeSortie, string realisateur,int recette, ListeActeurs acteurs): 
			Item(titre, anneeSortie), realisateur_(realisateur), recette_(recette), acteurs_(acteurs){}
		friend shared_ptr<Acteur> ListeFilms::trouverActeur(const string& nomActeur) const;
		friend Film* lireFilm(istream& fichier, ListeFilms& listeFilms);
		const string& accesRealisateur()const { return realisateur_; }
		int accesRecette() const { return recette_; }
		/*const ListeActeurs& accesActeurs()  { return acteurs_;  }*/
		void setRealisateur(string realisateur) { realisateur_ = realisateur; }
		void setRecette(int recette) { recette_ = recette;}
		void setActeurs(ListeActeurs acteurs) { acteurs_ = acteurs;}
		ListeActeurs acteurs_;
		void afficher(ostream& os) const override {
			os << "_____________________________________________________________________________________________________"
				<< endl << endl;;
			Item::afficher(os);
			os << "Realisateur: " << realisateur_ << ", Recette: " << recette_ << "M$" << std::endl;
			os << "Acteurs: " << endl; 
			for (shared_ptr<Acteur> acteur : acteurs_.enSpan()) {
				os << "Nom: " << acteur.get()->nom << ", Annee de naissance: " << acteur.get()->anneeNaissance
					<< ", Sexe: " << acteur.get()->sexe << endl;
			}
		}
	private:
		//ListeActeurs acteurs_;
		string realisateur_ = " ";
		int recette_ = 0; 
		
};

//Nouvelle Classe Livre qui herite de Item
class Livre : virtual public Item
{
	public:
		Livre();
		Livre(string titre,string auteur, int annee, int copiesVendus,int nombreDePages): Item(titre, annee),
		auteur_(auteur), copiesVendus_(copiesVendus), nombreDePages_(nombreDePages){}

		const string& accesAuteur() const { return auteur_; }
		int accesCopiesVendus() const { return copiesVendus_; }
		int accesNbPages() const { return nombreDePages_; }
		void setAuteur(string auteur) { auteur_ = auteur; }
		void setcopiesVendus(int copiesVendus) { copiesVendus_ = copiesVendus; }
		void setnombreDePages(int nombreDePages) { nombreDePages_ = nombreDePages; }
		void afficher(ostream& os) const override {
			Item::afficher(os);
			os << "Auteur: " << auteur_ << ", Copies vendues: " << copiesVendus_ << " millions, Pages: " << nombreDePages_ << std::endl;
		}
	private:
		string auteur_= " ";
		int copiesVendus_ = 0;
		int nombreDePages_ = 0;
};

//Partie 4, nouvelle classe FilmLivre
class FilmLivre : public Film, public Livre {
public:
	FilmLivre(const Film& film, const Livre& livre):
		Item(film.accesTitre(), film.accesAnneeSortie()),
		Film(film),
		Livre(livre) {}

	void afficher(ostream& os) const override {
		Film::afficher(os);
		// Affiche les informations supplémentaires de Livre, mais sans dupliquer le titre et l'année
		os << "Auteur: " << Livre::accesAuteur()
			<< ", Copies vendues: " << Livre::accesCopiesVendus()
			<< " millions, Pages: " << Livre::accesNbPages() << endl;
	}
};







