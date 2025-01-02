from PyQt5.QtWidgets import (
    QMainWindow,
    QWidget,
    QVBoxLayout,
    QTableWidget,
    QApplication,
    QHBoxLayout,
    QPushButton,
    QHeaderView,
    QTableWidgetItem,
    QCheckBox,
    QComboBox,
)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont
from meterConfig import MeterConfig
from utils import createLabel, open_next_page
import sys


class ServerPage(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle("Server Page")
        self.setFixedSize(1550, 700)
        self.setStyleSheet("background-color: #F1F1F1;")

        # Main layout
        main_layout = QVBoxLayout(self)
        main_layout.setAlignment(Qt.AlignCenter)

        title_label = createLabel("Please enter the server details", 20, "darkblue")
        main_layout.addWidget(title_label)

        self.table = QTableWidget(self)
        self.table.setColumnCount(7)  # Updated column count
        self.table.setHorizontalHeaderLabels(
            [
                "Type of Meter",
                "No. of Meters",
                "Path to Binary file",
                "Path to Config file",
                "Starting Port No.",
                "Starting Instance no.",
                "Garbage Values",
            ]
        )
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.table.setStyleSheet(
            "QTableWidget {font-size: 16px; gridline-color: #dcdcdc; background-color: white;}"
            "QHeaderView::section { background-color: #f0f0f0; font-weight: bold; padding: 6px;}"
            "QAbstractItemView::indicator { width: 25px; height: 25px }"
        )
        main_layout.addWidget(self.table)

        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignCenter)

        add_btn = QPushButton("Add Row", self)
        add_btn.clicked.connect(lambda: self.addRow())
        add_btn.setFont(QFont("Arial", 12))
        add_btn.setMinimumWidth(100)
        add_btn.setStyleSheet(
            "background-color: white; border: 1px solid black; border-radius: 5px;padding:8px"
        )
        button_layout.addWidget(add_btn)

        # Submit button
        submit_button = QPushButton("Submit", self)
        submit_button.clicked.connect(self.printTableData)
        submit_button.setFont(QFont("Arial", 12))
        submit_button.setMinimumWidth(100)
        submit_button.setStyleSheet(
            "background-color: white; border: 1px solid black; border-radius: 5px;padding:8px"
        )
        button_layout.addWidget(submit_button)
        main_layout.addLayout(button_layout)

        # Container widget
        container = QWidget(self)
        container.setLayout(main_layout)
        self.setCentralWidget(container)

    def addRow(self):
        row_index = self.table.rowCount()
        self.table.insertRow(row_index)

        meter_types = ["Single Phase", "Three Phase (WC)", "Three Phase (LTCT)", "Three Phase (HTCT)"]

        for column in range(self.table.columnCount()):
            if column == 0:  # Type of Meter (Dropdown)
                combobox = QComboBox()
                combobox.addItems(meter_types)
                combobox.setStyleSheet("QComboBox { text-align: center; }")  # Center align text
                self.table.setCellWidget(row_index, column, combobox)
            elif column == 6:  # Garbage Values (Checkbox)
                # Create a custom widget for the checkbox
                checkbox_widget = QWidget()
                checkbox = QCheckBox()
                checkbox_layout = QHBoxLayout(checkbox_widget)
                checkbox_layout.addWidget(checkbox)
                checkbox_layout.setAlignment(Qt.AlignCenter)  # Center align the checkbox
                checkbox_layout.setContentsMargins(0, 0, 0, 0)  # Remove extra padding
                self.table.setCellWidget(row_index, column, checkbox_widget)
            else:
                item = QTableWidgetItem("")
                item.setTextAlignment(Qt.AlignCenter)  # Center align text
                item.setToolTip(
                    "Enter text"
                )  # Tooltip for better UX when text is truncated
                self.table.setItem(row_index, column, item)

    def printTableData(self):
        row_count = self.table.rowCount()
        column_count = self.table.columnCount()

        for row in range(row_count):
            row_data = []
            for column in range(column_count):
                if column == 0:  # Type of Meter (Dropdown)
                    combobox = self.table.cellWidget(row, column)
                    if combobox is not None:
                        row_data.append(combobox.currentText())
                elif column == 6:  # Garbage Values (Checkbox)
                    checkbox_widget = self.table.cellWidget(row, column)
                    checkbox = checkbox_widget.layout().itemAt(0).widget()
                    row_data.append("Checked" if checkbox.isChecked() else "Unchecked")
                else:
                    item = self.table.item(row, column)
                    row_data.append(item.text() if item else "")
            print(f"Row {row + 1}: {row_data}")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ServerPage()
    window.show()
    sys.exit(app.exec_())
