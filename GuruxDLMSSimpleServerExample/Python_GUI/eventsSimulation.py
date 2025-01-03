import sys
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QTableWidget, QTableWidgetItem, QComboBox,
    QVBoxLayout, QWidget, QHeaderView, QStyledItemDelegate, QPushButton, QHBoxLayout
)
from PyQt5.QtGui import QFont
from PyQt5.QtCore import Qt
from utils import createLabel, open_next_page, open_previous_page

class ComboBoxDelegate(QStyledItemDelegate):
    def __init__(self, options, parent=None):
        super().__init__(parent)
        self.options = options

    def createEditor(self, parent):
        combo_box = QComboBox(parent)
        combo_box.addItems(self.options)
        return combo_box

    def setEditorData(self, editor, index):
        value = index.data(Qt.EditRole)
        if value:
            editor.setCurrentText(value)

    def setModelData(self, editor, model, index):
        model.setData(index, editor.currentText(), Qt.EditRole)


class EventSimulationApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Events to be Simulated")
        self.setFixedSize(1550, 725)

        # Central Widget
        central_widget = QWidget(self)
        self.setCentralWidget(central_widget)

        # Layout
        main_layout = QVBoxLayout(central_widget)
        main_layout.setContentsMargins(10, 10, 10, 10)

        # Title
        title_label = createLabel("Events to be Simulated", 22, "#003366")
        main_layout.addWidget(title_label)

        # Table
        self.table = QTableWidget(self)
        self.table.setColumnCount(3)
        self.table.setHorizontalHeaderLabels(["Subtest Number", "Events", "Percentage of Subtest for events"])
        self.load_table_data()

        # Table Styling and Resizing
        self.table.setStyleSheet(
            "QTableWidget { border: none; font-size: 16px; gridline-color: #dcdcdc; }"
            "QHeaderView::section { background-color: #f0f0f0; font-weight: bold; padding: 6px; }"
        )
        self.table.verticalHeader().setVisible(False)
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.table.resizeRowsToContents()

        main_layout.addWidget(self.table)

        # Data Storage
        self.data = []

        # Set the dropdown delegate for the "Events" column
        event_options = ["Connect/Disconnect", "Outlier", "Data Gaps", "System Failure"]
        delegate = ComboBoxDelegate(event_options, self.table)
        self.table.setItemDelegateForColumn(1, delegate)

        back_btn = QPushButton("Back", self)
        back_btn.clicked.connect(lambda: self.openPrevPage())
        back_btn.setFont(QFont("Arial", 12))
        back_btn.setMinimumWidth(100)
        back_btn.setStyleSheet(
            "background-color: white; border: 1px solid black; border-radius: 5px;padding:8px"
        )

        # Submit button
        submit_button = QPushButton("Submit",self)
        submit_button.clicked.connect(lambda: self.openNextPage())
        submit_button.setFont(QFont("Arial", 12))
        submit_button.setMinimumWidth(100)
        submit_button.setStyleSheet("background-color: white; border: 1px solid black; border-radius: 5px;padding:8px")

        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignCenter)
        button_layout.addWidget(back_btn)
        button_layout.addWidget(submit_button)
        main_layout.addLayout(button_layout)

    def load_table_data(self):
        """Load data for the table."""
        # Default data
        self.data = [
            ["1", "Connect/Disconnect", "1"],
            ["2", "Outlier", "0.5"],
            ["3", "Data Gaps", "10"],
            ["4", "System Failure", "3.5"],
            ["5", "Connect/Disconnect", "2.2"],
            ["6", "Outlier", "12"],
        ]

        # Populate the table
        self.table.setRowCount(len(self.data))
        for row, row_data in enumerate(self.data):
            for column, value in enumerate(row_data):
                item = QTableWidgetItem(value)
                item.setTextAlignment(Qt.AlignCenter)
                self.table.setItem(row, column, item)

        # Adjust row heights and resize
        self.table.resizeColumnsToContents()
        self.table.resizeRowsToContents()

    def openNextPage(self):
        from profilePage import ProfilePage
        next_page = ProfilePage()
        open_next_page(self, next_page)

    def openPrevPage(self):
        from meterConfig import MeterConfig
        prev_page = MeterConfig()
        open_previous_page(self, prev_page)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = EventSimulationApp()
    window.show()
    sys.exit(app.exec_())
