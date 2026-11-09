# Métier avancé — Affectation intelligente des agents

Ce dossier contient uniquement le code du métier avancé destiné au module **Emploi / Personnel** de Smart Weather.

## Fichiers
- `Headers/affectationintelligentewidget.h` : déclaration du widget.
- `Sources/affectationintelligentewidget.cpp` : interface, recommandations, contrôles métier et sauvegarde Oracle.

## Fonctionnalités
- Charge les interventions planifiées ou en cours depuis Oracle.
- Recherche les agents au statut `Disponible`.
- Écarte les agents ayant une intervention active dont les horaires chevauchent la période demandée.
- Classe les agents en favorisant la même zone, puis la charge de travail active la plus faible.
- Vérifie à nouveau la disponibilité avant l'enregistrement.
- Met à jour `INTERVENTION.ID_AGENT` et écrit une entrée dans `INTERVENTION_HISTORIQUE` au sein d'une transaction Oracle.

## Intégration dans le projet Qt
Le projet utilise qmake (`SmartWeather.pro`) et C++17. Ajouter les fichiers au projet :

```qmake
SOURCES += Sources/affectationintelligentewidget.cpp
HEADERS += Headers/affectationintelligentewidget.h
```

Dans `emploipage.cpp`, inclure le nouveau header :

```cpp
#include "affectationintelligentewidget.h"
```

Puis ajouter le widget comme nouvel onglet du `QTabWidget` existant :

```cpp
tabs->addTab(new AffectationIntelligenteWidget(tabs),
             QStringLiteral("Affectation intelligente"));
```

Si la page Emploi contient déjà une ancienne implémentation intégrée de cette fonctionnalité, retirer l'ancienne construction de l'onglet et ses méthodes associées avant d'ajouter le widget indépendant, afin d'éviter les doublons.

## Prérequis Oracle
La connexion Qt par défaut doit être ouverte avant la création du widget. Le code s'appuie sur les tables/champs du schéma Smart Weather :
- `AGENT(ID_AGENT, NOM, PRENOM, ROLE, STATUT, ID_ZONE)`
- `INTERVENTION(ID_INTERVENTION, TYPE, STATUT, DATE_DEBUT, DATE_FIN, ID_ZONE, ID_AGENT)`
- `ZONE(ID_ZONE, NOM)`
- `INTERVENTION_HISTORIQUE(ID_INTERVENTION, DATE_CHANGEMENT, CHAMP, ANCIENNE_VALEUR, NOUVELLE_VALEUR)`

Les valeurs de statut utilisées sont celles du schéma : agent `Disponible`; intervention `Planifiée` ou `En cours`.

## Règle pour une date de fin absente
Le code considère provisoirement qu'une intervention sans `DATE_FIN` dure un jour pour détecter les chevauchements. Adapter cette convention aux consignes du projet si nécessaire.

## Vérifications avant démonstration
Compiler dans l'environnement Qt du projet, puis tester :
1. un agent disponible sans conflit ;
2. un agent déjà affecté pendant une période qui chevauche ;
3. une intervention terminée ou annulée ;
4. l'enregistrement Oracle et la ligne d'historique après une affectation valide.

Le code n'a pas été compilé contre l'installation Qt/Oracle du poste dans lequel il sera utilisé ; une vérification locale reste nécessaire.
