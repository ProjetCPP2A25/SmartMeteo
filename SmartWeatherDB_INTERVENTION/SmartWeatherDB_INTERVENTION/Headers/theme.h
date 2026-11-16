#pragma once
#include <QString>

// ============================================================
//  THEME : tous les styles de l'application sont ici.
//  Thème sombre bleu nuit + turquoise (charte SmartWeather).
//  Les onglets (QTabBar) et le calendrier sont stylés explicitement :
//  sans cela, le texte clair des onglets restait invisible sur fond gris.
// ============================================================
namespace Theme {

inline QString styleSheet()
{
    return QStringLiteral(R"(
QWidget { color: #e6f1f5; font-family: 'Segoe UI'; font-size: 13px; }
QMainWindow, QWidget#root { background: #07151f; }

/* ---------- Titres et cartes ---------- */
QLabel#title     { font-size: 22px; font-weight: 700; color: #ffffff; }
QLabel#subtitle  { color: #8fb3c2; }
QLabel#message   { color: #19c6b7; font-weight: 600; }
QLabel[class="sectionTitle"] { font-size: 16px; font-weight: 700; color: #ffffff; }
QLabel[class="caption"]      { color: #8fb3c2; font-size: 12px; }
QFrame#kpi, QFrame#statCard {
    background: #0c2333; border: 1px solid #17475a; border-radius: 12px;
}
QLabel#kpiValue   { font-size: 26px; font-weight: 700; color: #ffffff; background: transparent; }
QLabel#kpiCaption { color: #8fb3c2; background: transparent; }
QFrame#card { background: #0c2333; border: 1px solid #17475a; border-radius: 14px; }

/* ---------- Onglets (Liste des interventions / Statistiques) ---------- */
QTabWidget::pane {
    border: 1px solid #17475a; border-radius: 12px; background: #0c2333; top: -1px;
}
QTabBar { background: transparent; }
QTabBar::tab {
    background: #0f2f42; color: #b7d0da; border: 1px solid #1c5a6e; border-bottom: none;
    border-top-left-radius: 10px; border-top-right-radius: 10px;
    padding: 10px 22px; margin-right: 4px; font-weight: 600;
}
QTabBar::tab:hover:!selected { background: #133b52; color: #ffffff; }
QTabBar::tab:selected { background: #19c6b7; color: #04202a; border-color: #19c6b7; }

/* ---------- Boutons ---------- */
QPushButton {
    background: #0f2f42; border: 1px solid #1c5a6e; border-radius: 8px; padding: 8px 14px; color: #e6f1f5;
}
QPushButton:hover    { background: #133b52; }
QPushButton:pressed  { background: #0b2231; }
QPushButton:disabled { color: #56727e; border-color: #15384a; }
QPushButton#primary  { background: #19c6b7; color: #04202a; border: none; font-weight: 700; padding: 9px 18px; }
QPushButton#primary:hover { background: #3ddccd; }
QPushButton#ok     { border: 1px solid #19c6b7; color: #19c6b7; font-weight: 600; }
QPushButton#ok:hover { background: #0f3a45; }
QPushButton#warn   { border: 1px solid #e49740; color: #f0b26a; }
QPushButton#warn:hover { background: #3a2c1a; }
QPushButton#danger { border: 1px solid #cc4e4f; color: #ff8a8b; }
QPushButton#danger:hover { background: #3a1d26; }
QPushButton#metier {
    background: #0f5563; border: 1px solid #19c6b7; color: #ffffff; font-weight: 600; padding: 8px 16px;
}
QPushButton#metier:hover { background: #12707f; }
QPushButton#pill {
    background: #0a1c2a; border: 1px solid #1c5a6e; border-radius: 16px; padding: 6px 18px; color: #b7d0da;
}
QPushButton#pill:hover { background: #133b52; }
QPushButton#pill:checked { background: #19c6b7; color: #04202a; border: 1px solid #19c6b7; font-weight: 700; }

/* ---------- Champs ---------- */
QLineEdit, QComboBox, QDateEdit, QDateTimeEdit, QSpinBox {
    background: #0a1c2a; border: 1px solid #1c5a6e; border-radius: 8px; padding: 7px 10px;
    selection-background-color: #19c6b7; selection-color: #04202a; color: #e6f1f5;
}
QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDateTimeEdit:focus { border: 1px solid #19c6b7; }
QComboBox::drop-down, QDateEdit::drop-down, QDateTimeEdit::drop-down { border: none; width: 24px; }
QComboBox QAbstractItemView {
    background: #0a1c2a; border: 1px solid #1c5a6e; selection-background-color: #12586a; outline: 0; color: #e6f1f5;
}
QPlainTextEdit, QTextEdit {
    background: #0a1c2a; border: 1px solid #1c5a6e; border-radius: 8px; padding: 6px; color: #e6f1f5;
}
QPlainTextEdit:focus, QTextEdit:focus { border: 1px solid #19c6b7; }
QCheckBox { spacing: 8px; background: transparent; }
QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid #1c5a6e; border-radius: 4px; background: #0a1c2a; }
QCheckBox::indicator:checked { background: #19c6b7; border-color: #19c6b7; }

/* ---------- Calendrier (menu déroulant des dates) ---------- */
QCalendarWidget QWidget { background: #0a1c2a; color: #e6f1f5; }
QCalendarWidget QAbstractItemView {
    background: #0a1c2a; color: #e6f1f5; selection-background-color: #19c6b7; selection-color: #04202a;
}
QCalendarWidget QToolButton { background: #0f2f42; color: #e6f1f5; border-radius: 6px; padding: 4px 8px; }
QCalendarWidget QMenu { background: #0a1c2a; color: #e6f1f5; }

/* ---------- Tableaux ---------- */
QTableView, QTableWidget {
    background: transparent; alternate-background-color: #0e2839; border: none;
    gridline-color: transparent; selection-background-color: #12586a; selection-color: white;
}
QTableView::item { padding: 6px; border: none; }
QHeaderView::section {
    background: transparent; color: #19c6b7; border: none;
    border-bottom: 1px solid #17475a; padding: 8px; font-weight: 600;
}
QTableCornerButton::section { background: transparent; border: none; }

/* ---------- Dialogues ---------- */
QDialog, QMessageBox { background: #0a1c2a; }
QScrollArea { background: transparent; border: none; }
QScrollBar:vertical { background: transparent; width: 10px; }
QScrollBar::handle:vertical { background: #1c5a6e; border-radius: 5px; min-height: 30px; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }
QToolTip { background: #0c2333; color: white; border: 1px solid #19c6b7; }
)");
}

} // namespace Theme
