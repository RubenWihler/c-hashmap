# C Generic Hashmap

Une implémentation de hashmap générique dynamique en C.
Elle est basée sur une table de hachage et utilise des listes chaînées pour gérer les collisions.

Le fichier `src/hashmap.h` contient des explications détaillées sur l'utilisation, les fonctionnalités et les limitations de la hashmap.

## 🚀 Fonctionnalités

- [x] Ajouter un élément
- [x] Supprimer un élément
- [x] Rechercher un élément
- [x] Possibilité de spécifier une fonction de hachage personnalisée
- [x] Possibilité de spécifier une fonction de comparaison personnalisée
- [x] Possibilité de spécifier une fonction d'allocation et de copie personnalisée
- [x] Possibilité de spécifier une fonction de libération personnalisée
- [x] Afficher les éléments de la hashmap (fonction de print personnalisée)
- [x] Libérer la hashmap

- [] Faire en sorte que la hashmap soit thread-safe
- [] Donner un stream à la fonction de print pour afficher les éléments
- [] Ajouter des tests unitaires

## 📦 Utilisation

Le fichier `/src/demo.c` contient un exemple complet d'utilisation de la hashmap.

```bash
make demo && ./bin/demo
```

## 🚧 Limitations

Je ne recommande pas d'utiliser cette hashmap pour des applications nécessitant des performances élevées ou une utilisation intensive de la mémoire.

Le plus grand inconvénient est du à l'utilisation de listes chaînées pour gérer les collisions, la mémoire est fragmentée et les performances sont moins bonnes que celles d'une table de hachage avec des sondages linéaires ou quadratiques.

## 📝 License

Ce projet est sous licence Apache 2.0 - voir le fichier [LICENSE](LICENSE) pour plus d'informations.

## 📧 Contact

Wihler Ruben : [wihlerruben@gmail.com](mailto:wihlerruben@gmail.com)
