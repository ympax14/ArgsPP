# ArgsPP

**ArgsPP** est une bibliothèque C++ *header-only* légère, robuste et facile d'utilisation permettant d'analyser, valider et extraire les arguments passés en ligne de commande (runtime arguments).

## 🚀 Fonctionnalités

* **Header-only** : Un seul fichier à inclure, aucune compilation séparée nécessaire.
* **Typage fort & Conversions** : Récupération facile en `int`, `float`, `double`, `bool`, `std::string` ou `const char*`.
* **Validation stricte** : Gestion des arguments requis, des valeurs par défaut et restriction aux valeurs autorisées.
* **Génération automatique de l'aide** : Affichage clair des options disponibles via `--help` ou si aucun argument n'est fourni.
* **Flexibilité** : Support des "flags" booléens automatiques et modification dynamique du préfixe (ex: `--`, `-`, `/`).
* **Arguments dynamiques** : Capture des arguments passés à la volée même s'ils n'ont pas été enregistrés au préalable.

## 🛠 Installation

Glissez simplement le fichier `ArgsPP.hpp` dans le dossier de votre projet et incluez-le :

```cpp
#include "ArgsPP.hpp"
```

---

## 📖 Guide d'utilisation

Voici un exemple complet (`main.cpp`) démontrant toutes les capacités de la librairie **ArgsPP**.

```cpp
#include "ArgsPP.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    // Instanciation du parseur
    ArgsPP parser;

    // ---------------------------------------------------------
    // 1. CONFIGURATION DU PRÉFIXE (Optionnel)
    // Par défaut, le préfixe est "--", mais on peut le modifier :
    // ---------------------------------------------------------
    parser.setArgumentsPrefix("--"); 


    // ---------------------------------------------------------
    // 2. ENREGISTREMENT DES ARGUMENTS
    // ---------------------------------------------------------
    
    // A. Argument REQUIS (Pas de valeur par défaut)
    parser.registerArgument("input", "Fichier source à traiter", true);

    // B. Argument OPTIONNEL (Nécessite impérativement une valeur par défaut)
    parser.registerArgument("threads", "Nombre de threads à allouer", false, "4");

    // C. Argument avec VALEURS AUTORISÉES restreintes
    // registerArgument retourne une référence sur l'Arg, on peut donc le chainer.
    ArgsPP::Arg& modeArg = parser.registerArgument("mode", "Mode d'exécution", false, "fast");
    modeArg.addAllowedValue("fast");
    modeArg.addAllowedValue("safe");
    modeArg.addAllowedValue("strict");

    // D. FLAG BOOLEAN (Argument sans valeur explicite)
    // Si l'utilisateur passe "--verbose" sans valeur, la lib lui assignera "true".
    parser.registerArgument("verbose", "Activer les logs détaillés", false, "false");


    // ---------------------------------------------------------
    // 3. PARSING ET VALIDATION
    // ---------------------------------------------------------
    try {
        // La méthode parseArgs gère automatiquement :
        // - L'appel de l'aide (--help ou -h) -> exit(0)
        // - L'appel sans arguments -> affiche l'aide et exit(0)
        // - La validation des arguments requis
        // - La vérification des valeurs autorisées (throw invalid_argument)
        parser.parseArgs(argc, argv);
        
    } catch (const std::exception& e) {
        // Capture les erreurs (ex: argument requis manquant, mauvaise valeur)
        std::cerr << "[ERREUR] " << e.what() << std::endl;
        return EXIT_FAILURE;
    }


    // ---------------------------------------------------------
    // 4. ACCÈS ET CONVERSION DES VALEURS
    // L'opérateur [] retourne un pointeur constant (const Arg*) ou nullptr.
    // ---------------------------------------------------------

    // Format Chaîne de caractères (String & C-String)
    const ArgsPP::Arg* inputArg = parser["input"];
    if (inputArg != nullptr) {
        std::cout << "Fichier source : " << inputArg->getValue() << std::endl;
        // Ou en C-string pour d'anciennes API : inputArg->toCChar()
    }

    // Conversion en Entier (int)
    const ArgsPP::Arg* threadsArg = parser["threads"];
    if (threadsArg != nullptr) {
        int threadCount = threadsArg->toInt();
        std::cout << "Threads : " << threadCount << std::endl;
    }

    // Conversion Booléenne (bool)
    const ArgsPP::Arg* verboseArg = parser["verbose"];
    if (verboseArg != nullptr && verboseArg->toBoolean() == true) {
        std::cout << "Mode verbeux : ACTIVÉ" << std::endl;
    }

    // ---------------------------------------------------------
    // 5. ARGUMENTS DYNAMIQUES (Non-enregistrés)
    // Si l'utilisateur passe --magic 42, la librairie le capture quand même !
    // ---------------------------------------------------------
    const ArgsPP::Arg* magicArg = parser["magic"];
    if (magicArg != nullptr) {
        // Conversions disponibles : toInt(), toDouble(), toFloat(), toBoolean(), toCChar()
        std::cout << "Argument dynamique capturé : " << magicArg->toFloat() << std::endl;
    }

    return EXIT_SUCCESS;
}
```

## 🖥 Exemple de rendu dans le terminal

Si vous exécutez votre programme avec l'argument `--help` (ou sans aucun argument), **ArgsPP** générera automatiquement cet affichage basé sur vos descriptions et contraintes :

```text
Options disponibles :
  --input    Fichier source à traiter [Requis]
  --threads  Nombre de threads à allouer [Défaut : 4]
  --mode     Mode d'exécution [Défaut : fast]
             (Valeurs autorisées : fast, safe, strict)
  --verbose  Activer les logs détaillés [Défaut : false]
```

## ⚠️ Gestion des Erreurs

La librairie utilise le système d'exceptions standard C++ (`<stdexcept>`). Il est recommandé d'entourer l'appel à `parseArgs` et `setValue` d'un bloc `try/catch` :

* `std::runtime_error` : Levée si un argument requis est manquant lors du parsing.
* `std::invalid_argument` : Levée si une valeur passée ne fait pas partie du set `allowedValues` ou si la création de l'argument est mal formatée (ex: argument optionnel sans valeur par défaut).