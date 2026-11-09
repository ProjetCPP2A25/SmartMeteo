-- ============================================================
--  Smart Weather - Nettoyage des ANCIENNES tables (optionnel)
--
--  Les scripts d'installation ne suppriment que les tables qu'ils
--  recreent (ZONE, FOURNISSEUR, EQUIPEMENT, AGENT, INTERVENTION,
--  INTERVENTION_EQUIPEMENT). Les tables des anciennes versions du projet
--  restent donc dans le schema : ce script les supprime.
--
--  A executer avec l'utilisateur METEO. Il ne touche PAS aux tables
--  APEX$_*, DEMO_*, DEPT, EMP, USERS, MEASUREMENTS (exemples Oracle).
--  Verifier d'abord : SELECT table_name FROM user_tables ORDER BY 1;
-- ============================================================
SET SERVEROUTPUT ON
BEGIN
    FOR t IN (SELECT table_name FROM user_tables
              WHERE table_name IN ('AGENT_HISTORIQUE', 'EQUIPEMENT_HISTORIQUE',
                                   'FOURNISSEUR_HISTORIQUE', 'HISTORIQUE_FOURNISSEUR',
                                   'ZONE_HISTORIQUE', 'INTERV_EQUIPEMENT',
                                   'INTERVENTION_HIST', 'INTERVENTION_HISTORIQUE',
                                   'INTERVENTIONS', 'ZONES'))
    LOOP
        EXECUTE IMMEDIATE 'DROP TABLE "' || t.table_name || '" CASCADE CONSTRAINTS PURGE';
        DBMS_OUTPUT.PUT_LINE('Supprimee : ' || t.table_name);
    END LOOP;
END;
/
-- Il doit rester uniquement les 6 tables du projet :
SELECT table_name FROM user_tables
WHERE table_name IN ('ZONE', 'FOURNISSEUR', 'EQUIPEMENT', 'AGENT', 'INTERVENTION', 'INTERVENTION_EQUIPEMENT')
ORDER BY 1;
