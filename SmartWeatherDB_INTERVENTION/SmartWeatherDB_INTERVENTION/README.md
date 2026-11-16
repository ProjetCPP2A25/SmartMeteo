# Smart Weather — Tâche INTERVENTION (Qt 6 / C++17 / Oracle 11g)

Dossier autonome : uniquement la gestion des interventions (CRUD, contrôle de saisie, 2 métiers, statistiques).

## Lancer
1. SQL Developer, connexion **METEO5 / meteo5** : ouvrir `SQL/install_intervention.sql`, exécuter avec **F5**
   (crée les 5 tables + données de démonstration). Puis `SQL/verify_model.sql` pour vérifier.
2. Source ODBC **64 bits** `source2A` (identifiants dans `Sources/connection.cpp`).
3. Qt Creator : ouvrir `SmartWeatherIntervention.pro` → Clean All → Run qmake → Rebuild → Ctrl+R.
   (modules Qt : Widgets + SQL, pilote ODBC. Qt Charts n'est plus nécessaire.)

## Ce qui a changé
- **Ajouter** : le formulaire enregistrait, puis la page enregistrait une 2ᵉ fois ; de plus les statuts accentués
  (`Planifiée`) étaient refusés par la base quand elle avait été créée avec un mauvais encodage. Maintenant :
  une seule écriture, valeurs sans accent en base, NULL typés pour le pilote ODBC.
- **Métiers** (en haut, à côté des onglets) : *Estimer la durée* (moyenne des interventions terminées, avec option
  « appliquer la date de fin ») et **nouveau** *Détecter les conflits* (même agent ou même équipement sur des horaires
  qui se chevauchent). Supprimés : recalcul de priorité, prérequis, interventions en retard, affectation intelligente.
- **Onglets** lisibles (style sombre explicite) ; **Statistiques** refaites : anneau + barres, cartes (total, taux de clôture,
  en retard, durée moyenne), choix de la dimension (statut, priorité, type, zone, mois, agent) et période.
- **Modèle physique** : 5 tables, sans historique (voir `docs/`).
- Export PDF réellement fonctionnel (avant : simulé).

## Structure
`Sources/` `Headers/` `Forms/` `SQL/` `docs/` — un `.cpp` par classe, requêtes préparées (`prepare` + `bindValue`).
