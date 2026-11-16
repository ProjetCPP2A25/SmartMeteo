# Modèle physique — tâche INTERVENTION (Oracle 11g XE)

**5 tables, aucune table d'historique.** Image : `modele_physique_intervention.png` (même dossier).
Script : `SQL/install_intervention.sql`.

| Table | Clé primaire | Clés étrangères |
|---|---|---|
| ZONE | ID_ZONE | — |
| AGENT | ID_AGENT | ID_ZONE → ZONE |
| FOURNISSEUR | ID_FOURNISSEUR | — |
| EQUIPEMENT | ID_EQUIPEMENT | ID_FOURNISSEUR → FOURNISSEUR |
| INTERVENTION | ID_INTERVENTION | ID_ZONE → ZONE, ID_AGENT → AGENT, ID_EQUIPEMENT → EQUIPEMENT |

## Associations
- **Possède** : ZONE (1) — (0,N) INTERVENTION et ZONE (1) — (0,N) AGENT
- **Affect** : AGENT (0,1) — (0,N) INTERVENTION (une intervention a 0 ou 1 agent)
- **Utilise** : EQUIPEMENT (0,1) — (0,N) INTERVENTION (une intervention utilise 0 ou 1 équipement)
- **Fournit** : FOURNISSEUR (0,1) — (0,N) EQUIPEMENT

Comme il n'y a que 5 tables, « Utilise » est une clé étrangère `ID_EQUIPEMENT` dans INTERVENTION
(l'ancienne table d'association `INTERVENTION_EQUIPEMENT` est supprimée).

## Valeurs stockées sans accent
Statuts : `Planifiee`, `En cours`, `Terminee`, `Annulee`. L'application affiche « Planifiée », « Terminée »…
Cela corrige l'affichage `PlanifiÃ©e` et l'erreur de contrainte CHECK qui bloquait « Ajouter ».
