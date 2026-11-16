#include "connection.h"
#include "interventionwidget.h"
#include "theme.h"

#include <QApplication>
#include <QDebug>
#include <QFont>
#include <QMainWindow>
#include <QMessageBox>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("SmartWeatherIntervention"));
    app.setFont(QFont(QStringLiteral("Segoe UI"), 10));
    app.setStyleSheet(Theme::styleSheet());

    // Connexion Oracle (Singleton) AVANT de créer la page : elle lit la base dès sa construction.
    Connection &c = Connection::createInstance();
    if (!c.createConnection()) {
        QMessageBox::critical(nullptr, QStringLiteral("Erreur de base de données"),
            QStringLiteral("Connexion à la base Oracle impossible.\n\n%1\n\n"
                           "Vérifiez la source ODBC « source2A » (64 bits), l'utilisateur « meteo5 » et le mot de passe « meteo5 ».")
                .arg(c.lastError()));
    } else {
        qDebug() << ">>> DATABASE CONNECTION SUCCESSFUL <<<";
    }

    QMainWindow window;
    window.setWindowTitle(QStringLiteral("Smart Weather — Gestion des interventions"));
    auto *root = new QWidget;
    root->setObjectName(QStringLiteral("root"));
    auto *lay = new QVBoxLayout(root);
    lay->setContentsMargins(20, 16, 20, 16);
    lay->addWidget(new InterventionWidget);
    window.setCentralWidget(root);
    window.resize(1320, 800);
    window.show();

    return app.exec();
}
