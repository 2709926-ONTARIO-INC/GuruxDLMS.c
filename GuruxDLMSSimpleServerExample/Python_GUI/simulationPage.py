import sys
import os
import json
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QTableWidget, QTableWidgetItem,
    QVBoxLayout, QPushButton, QWidget, QHBoxLayout, QMessageBox, QHeaderView
)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont
from eventsSimulation import EventSimulationApp
from utils import createLabel, open_next_page

class SimulationDetailsApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Simulation Details")
        self.setFixedSize(1550,725)
        self.setWindowFlag(Qt.WindowStaysOnTopHint)

        # File to store saved data
        self.data_file = "simulation.json"

        # Central Widget
        central_widget = QWidget(self)
        self.setCentralWidget(central_widget)

        # Layout
        main_layout = QVBoxLayout(central_widget)
        main_layout.setContentsMargins(10, 10, 10, 10)

        # Title
        title_label = createLabel("Simulation Details", 22, "#003366")
        main_layout.addWidget(title_label)

        # Table
        self.table = QTableWidget(self)
        self.table.setColumnCount(4)
        self.table.setHorizontalHeaderLabels(["Subtest Number", "Type", "Quantity", "Manufacturer"])
        self.load_table_data()

        # Table Styling and Resizing
        self.table.setStyleSheet(
            "QTableWidget { border: none; font-size: 16px; gridline-color: #dcdcdc; }"
            "QHeaderView::section { background-color: #f0f0f0; font-weight: bold; padding: 6px; }"
        )
        self.table.verticalHeader().setVisible(False)
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)  # Stretch columns to fit the window
        self.table.setEditTriggers(QTableWidget.NoEditTriggers)  # Disable editing by default
        self.table.setMinimumHeight(400)  # Ensure table doesn't collapse on resizing
        self.table.resizeRowsToContents()

        main_layout.addWidget(self.table)

        # Buttons
        button_layout = QHBoxLayout()
        button_layout.setContentsMargins(20, 10, 20, 10)

        self.edit_button = QPushButton("Edit", self)
        self.edit_button.setMaximumWidth(100)
        self.save_button = QPushButton("Save as template", self)
        self.save_button.setMaximumWidth(175)
        next_button = QPushButton("Next", self)
        next_button.setMaximumWidth(100)

        # Button Styling
        for button in [self.edit_button, self.save_button, next_button]:
            button.setStyleSheet(
                "QPushButton { font-size: 14px; background-color: #007BFF; color: white; border-radius: 8px; padding: 8px 16px; }"
                "QPushButton:hover { background-color: #0056b3; }"
            )
            button.setFont(QFont("Arial", 12))

        self.edit_button.clicked.connect(self.enable_edit_mode)
        self.save_button.clicked.connect(self.save_data)
        next_page = EventSimulationApp()
        next_button.clicked.connect(lambda: open_next_page(self, next_page))

        button_layout.addWidget(self.edit_button)
        button_layout.addWidget(self.save_button)
        button_layout.addWidget(next_button)

        main_layout.addLayout(button_layout)

        # Data Storage
        self.data = []

    def load_table_data(self):
        """Load data from the file or use default data."""
        if os.path.exists(self.data_file):
            with open(self.data_file, "r") as file:
                self.data = json.load(file)
        else:
            # Default data
            self.data = [
                ["1", "1 phase", "10,000", "AEW"],
                ["2", "1 phase", "5,000", "HPL"],
                ["3", "3 ph. WC", "12,000", "Genus"],
                ["4", "3 ph. WC", "150,000", "SEC"],
                ["5", "3 ph. LTCT", "64,000", "JPM"],
                ["6", "3 ph. HTCT", "33,000", "JPM"],
            ]

        # Populate the table
        self.table.setRowCount(len(self.data))
        for row, row_data in enumerate(self.data):
            for column, value in enumerate(row_data):
                item = QTableWidgetItem(value)
                item.setTextAlignment(Qt.AlignCenter)
                if column in [2, 3]:  # Editable columns
                    item.setFlags(item.flags() | Qt.ItemIsEditable)
                self.table.setItem(row, column, item)

        # Adjust row heights and resize
        self.table.resizeColumnsToContents()
        self.table.resizeRowsToContents()

    def enable_edit_mode(self):
        """Enable editing mode for the table."""
        self.table.setEditTriggers(QTableWidget.AllEditTriggers)
        QMessageBox.information(self, "Edit Mode", "You can now edit 'Type' 'Quantity' and 'Manufacturer' columns.")

    def save_data(self):
        """Save the updated data to a file."""
        self.table.setEditTriggers(QTableWidget.NoEditTriggers)  # Disable editing after save

        # Save the updated data from the table
        updated_data = []
        for row in range(self.table.rowCount()):
            row_data = [
                self.table.item(row, col).text() if self.table.item(row, col) else ""
                for col in range(self.table.columnCount())
            ]
            updated_data.append(row_data)

        self.data = updated_data

        # Save data to a JSON file
        with open(self.data_file, "w") as file:
            json.dump(self.data, file)

        QMessageBox.information(self, "Data Saved", "The data has been successfully saved!")
        print("Updated Data:", self.data)  # Debugging output

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = SimulationDetailsApp()
    window.show()
    sys.exit(app.exec_())
