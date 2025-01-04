from PyQt5.QtWidgets import (
    QMainWindow,
    QWidget,
    QVBoxLayout,
    QTableWidget,
    QApplication,
    QHBoxLayout,
    QHeaderView,
    QTableWidgetItem,
    QCheckBox,
    QComboBox,
    QMessageBox
)
from PyQt5.QtCore import Qt
from utils import createLabel, open_next_page, open_previous_page, createButton
import sys
import json
import os

class ParameterPopup(QWidget):
    def __init__(self, parent_table):
        super().__init__()
        self.parent_table = parent_table
        self.initUI()

    def initUI(self):
        self.setWindowTitle("Input Parameters")
        self.setMinimumSize(500, 310)
        self.setStyleSheet("background-color: #F1F1F1;")
        self.setWindowModality(Qt.ApplicationModal)
        self.setWindowFlag(Qt.WindowStaysOnTopHint)

        input_table = QTableWidget()
        # Table Styling and Resizing
        input_table.setStyleSheet(
            "QTableWidget {font-size: 16px; gridline-color: #dcdcdc; background-color: white;}"
            "QHeaderView::section { background-color: #f0f0f0; font-weight: bold; padding: 6px;}"
        )
        input_table.setRowCount(5)
        input_table.setColumnCount(3)
        input_table.setHorizontalHeaderLabels(["Qty.","Min","Max"])
        input_table.verticalHeader().setVisible(False)
        input_table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        for i, label in enumerate(["V", "I", "P.F", "f", "Block Load"]):
            input_table.setItem(i, 0, QTableWidgetItem(label))

        popup_layout = QVBoxLayout(self)
        popup_layout.addWidget(input_table)

        # Save button
        save_button = createButton("Save")
        save_button.clicked.connect(lambda: self.save_to_json(input_table))

        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignCenter)
        button_layout.addWidget(save_button)
        popup_layout.addLayout(button_layout)

    def save_to_json(self, input_table):
        # Function to check if input is a valid integer
        def is_valid_input(item):
            return item and item.text().isdigit()

        # Check if all fields have valid input
        for row in range(5):
            for col in range(1, 3):
                item = input_table.item(row, col)
                if not is_valid_input(item):
                    QMessageBox.warning(self, "Input Error", "Please enter valid values.")
                    return
            
        current_row = self.parent_table.currentRow()
        if current_row == -1:
            QMessageBox.warning(self, "Selection Error", "Please select a row to save the data.")
            return
        meter_type = self.parent_table.cellWidget(current_row, 0).currentText()
        manufacturer = self.parent_table.item(current_row, 2).text()
        server_no = "SRV00" + str(current_row+1)

        # Construct the filename
        file_name = f"{manufacturer}_{server_no}_{meter_type}_config.json"
        file_path = os.path.join("GuruxDLMSSimpleServerExample", "Python_GUI", file_name)

        if meter_type == "Single Phase":
            config_data = {
                "manufacturer": manufacturer,
                "voltage_limits": {
                    "lower_limit": int(input_table.item(0, 1).text()) * 1000,
                    "upper_limit": int(input_table.item(0, 2).text()) * 1000
                },
                "current_limits": {
                    "phase_current": {
                        "lower_limit": int(input_table.item(1, 1).text()) * 1000,
                        "upper_limit": int(input_table.item(1, 2).text()) * 1000
                    },
                    "neutral_current": {
                        "lower_limit": int(input_table.item(1, 1).text()) * 1000,
                        "upper_limit": int(input_table.item(1, 2).text()) * 1000
                    }
                },
                "frequency_limits": {
                    "lower_limit": int(input_table.item(2, 1).text()) * 1000,
                    "upper_limit": int(input_table.item(2, 2).text()) * 1000
                },
                "signed_power_factor_limits": {
                    "lower_limit": int(input_table.item(3, 1).text()) * 1000,
                    "upper_limit": int(input_table.item(3, 2).text()) * 1000
                },
                "block_energy_limits": {
                    "kWh_import": {
                        "lower_limit": int(input_table.item(4, 1).text()) * 1000,
                        "upper_limit": int(input_table.item(4, 2).text()) * 1000
                    },
                    "kWh_export": {
                        "lower_limit": int(input_table.item(4, 1).text()) * 1000,
                        "upper_limit": int(input_table.item(4, 2).text()) * 1000
                    }
                }
            }

        else:
            config_data = {
                "manufacturer": manufacturer,
                "voltage_limits": {
                    "L1": {
                        "lower_limit": int(input_table.item(0, 1).text()) * 1000,
                        "upper_limit": int(input_table.item(0, 2).text()) * 1000
                    },
                    "L2": {
                        "lower_limit": int(input_table.item(0, 1).text()) * 1000,
                        "upper_limit": int(input_table.item(0, 2).text()) * 1000
                    },
                    "L3": {
                        "lower_limit": int(input_table.item(0, 1).text()) * 1000,
                        "upper_limit": int(input_table.item(0, 2).text()) * 1000
                    }
                },
                "current_limits": {
                    "L1": {
                        "lower_limit": int(input_table.item(1, 1).text()) * 100000,
                        "upper_limit": int(input_table.item(1, 2).text()) * 100000
                    },
                    "L2": {
                        "lower_limit": int(input_table.item(1, 1).text()) * 100000,
                        "upper_limit": int(input_table.item(1, 2).text()) * 100000
                    },
                    "L3": {
                        "lower_limit": int(input_table.item(1, 1).text()) * 100000,
                        "upper_limit": int(input_table.item(1, 2).text()) * 100000
                    }
                },
                "frequency_limits": {
                        "lower_limit": int(input_table.item(2, 1).text()) * 1000,
                        "upper_limit": int(input_table.item(2, 2).text()) * 1000
                },
                "power_factor_limits": {
                    "L1": {
                        "lower_limit": int(input_table.item(3, 1).text()) * 1000,
                        "upper_limit": int(input_table.item(3, 2).text()) * 1000
                    },
                    "L2": {
                        "lower_limit": int(input_table.item(3, 1).text()) * 1000,
                        "upper_limit": int(input_table.item(3, 2).text()) * 1000
                    },
                    "L3": {
                        "lower_limit": int(input_table.item(3, 1).text()) * 1000,
                        "upper_limit": int(input_table.item(3, 2).text()) * 1000
                    }
                },
                "block_energy_limits": {
                    "kWh_import": {
                        "lower_limit": int(input_table.item(4, 1).text()) * 100,
                        "upper_limit": int(input_table.item(4, 2).text()) * 100
                    },
                    "kVAh_lag": {
                        "lower_limit": int(input_table.item(4, 1).text()) * 100,
                        "upper_limit": int(input_table.item(4, 2).text()) * 100
                    },
                    "kVAh_lead": {
                        "lower_limit": int(input_table.item(4, 1).text()) * 100,
                        "upper_limit": int(input_table.item(4, 2).text()) * 100
                    },
                    "kVAh_import": {
                        "lower_limit": int(input_table.item(4, 1).text()) * 100,
                        "upper_limit": int(input_table.item(4, 2).text()) * 100
                    }
                }
            }

        # Save the configuration to a new JSON file
        with open(file_path, "w") as f:
            json.dump(config_data, f, indent=4)
            
        self.close()

class ServerPage(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle("Server Page")
        self.setFixedSize(1550, 700)
        self.setStyleSheet("background-color: #F1F1F1;")
        self.setWindowFlag(Qt.WindowStaysOnTopHint)

        # Main layout
        main_layout = QVBoxLayout()
        main_layout.setAlignment(Qt.AlignCenter)

        title_label = createLabel("Please enter the server details", 20, "darkblue")
        main_layout.addWidget(title_label)

        self.table = QTableWidget(self)
        self.table.setColumnCount(7) 

        self.table.setHorizontalHeaderLabels(
            [
                "Type of Meter",
                "No. of Meters",
                "Manufacturer",
                "Starting Port No.",
                "Starting Instance no.",
                "Garbage Values",
                "Parameters"
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

        back_btn = createButton("Back")
        back_btn.clicked.connect(lambda: self.openPrevPage())
        button_layout.addWidget(back_btn)

        add_btn = createButton("Add Row")
        add_btn.clicked.connect(lambda: self.addRow())
        button_layout.addWidget(add_btn)

        # Submit button
        submit_button = createButton("Submit")
        submit_button.clicked.connect(self.printTableData)
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
                combobox.setStyleSheet("QComboBox { text-align: center; }")  
                self.table.setCellWidget(row_index, column, combobox)
            elif column == 5: 
                # Create a custom widget for the checkbox
                checkbox_widget = QWidget()
                checkbox = QCheckBox()
                checkbox_layout = QHBoxLayout()
                checkbox_layout.addWidget(checkbox)
                checkbox_layout.setAlignment(Qt.AlignCenter)  
                checkbox_layout.setContentsMargins(0, 0, 0, 0) 
                checkbox_widget.setLayout(checkbox_layout)
                self.table.setCellWidget(row_index, column, checkbox_widget)
            elif column == 6: 
                push_btn = createButton("Open", 10)
                popup_window = ParameterPopup(self.table)
                push_btn.clicked.connect(lambda: popup_window.show())
                self.table.setCellWidget(row_index, column, push_btn)
            else:
                item = QTableWidgetItem()
                item.setTextAlignment(Qt.AlignCenter)  
                self.table.setItem(row_index, column, item)

    def printTableData(self):
        for row in range(self.table.rowCount()):
            data = []
            for col in range(self.table.columnCount()):
                widget = self.table.cellWidget(row, col)
                if isinstance(widget, QComboBox):
                    data.append(widget.currentText())
                elif isinstance(widget, QWidget):
                    checkbox = widget.layout().itemAt(0).widget()
                    data.append("Checked" if checkbox.isChecked() else "Unchecked")
                else:
                    item = self.table.item(row, col)
                    data.append(item.text() if item else "")
            print(f"Row {row + 1}: {data}")

        self.openNextPage()

    def openNextPage(self):
        from meterConfig import MeterConfig
        next_page = MeterConfig()
        open_next_page(self, next_page)

    def openPrevPage(self):
        from loginPage import LoginPage
        prev_page = LoginPage()
        open_previous_page(self, prev_page)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = ServerPage()
    window.show()
    sys.exit(app.exec_())
