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
    QMessageBox
)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont
from utils import createLabel, open_next_page, open_previous_page
import sys
import json

class ParameterPopup(QWidget):
    def __init__(self, parent_table):
        super().__init__()
        self.parent_table = parent_table
        self.initUI()

    def initUI(self):
        self.setWindowTitle("Input Parameters")
        self.setMinimumSize(500, 310)
        self.setStyleSheet("background-color: #F1F1F1;")

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
        input_table.setItem(0,0, QTableWidgetItem("V"))
        input_table.setItem(1,0, QTableWidgetItem("I"))
        input_table.setItem(2,0, QTableWidgetItem("P.F"))
        input_table.setItem(3,0, QTableWidgetItem("f"))
        input_table.setItem(4,0, QTableWidgetItem("Block Load"))

        popup_layout = QVBoxLayout(self)
        popup_layout.addWidget(input_table)

        # Submit button
        submit_button = QPushButton("Save")
        submit_button.clicked.connect(lambda: self.save_to_json(input_table))
        submit_button.setFont(QFont("Arial", 12))
        submit_button.setMinimumWidth(100)
        submit_button.setStyleSheet("background-color: white; border: 1px solid black; border-radius: 5px; padding:8px")

        button_layout = QHBoxLayout()
        button_layout.setAlignment(Qt.AlignCenter)
        button_layout.addWidget(submit_button)
        popup_layout.addLayout(button_layout)

    def save_to_json(self, input_table):
        # Function to check if input is a valid integer
        def is_valid_integer(value):
            try:
                int(value)
                return True
            except ValueError:
                return False

        # Check if all fields have valid input
        for row in range(5):
            for col in range(1, 3):
                item = input_table.item(row, col)
                if not item or not is_valid_integer(item.text()):
                    QMessageBox.warning(self, "Input Error", "Please input a valid value.")
                    return  # Exit the function if any invalid input is found
            
        row_no = self.parent_table.rowCount() - 1
        meter_type = self.parent_table.cellWidget(row_no, 0).currentText()

        if meter_type == "Single Phase":
            with open("GuruxDLMSSimpleServerExample\Python_GUI\single_phase_config.json", "r") as f:
                config_data = json.loads(f.read())

            # Extract values from the table and update the JSON structure
            config_data["voltage_limits"]["lower_limit"] = int(input_table.item(0, 1).text())
            config_data["voltage_limits"]["upper_limit"] = int(input_table.item(0, 2).text())

            config_data["current_limits"]["phase_current"]["lower_limit"] = int(input_table.item(1, 1).text())
            config_data["current_limits"]["phase_current"]["upper_limit"] = int(input_table.item(1, 2).text())
            config_data["current_limits"]["neutral_current"]["lower_limit"] = int(input_table.item(1, 1).text())
            config_data["current_limits"]["neutral_current"]["upper_limit"] = int(input_table.item(1, 2).text())
            
            config_data["signed_power_factor_limits"]["lower_limit"] = int(input_table.item(2, 1).text())
            config_data["signed_power_factor_limits"]["upper_limit"] = int(input_table.item(2, 2).text())
            
            config_data["frequency_limits"]["lower_limit"] = int(input_table.item(3, 1).text())
            config_data["frequency_limits"]["upper_limit"] = int(input_table.item(3, 2).text())

            config_data["block_energy_limits"]["kWh_import"]["lower_limit"] = int(input_table.item(4, 1).text())
            config_data["block_energy_limits"]["kWh_import"]["upper_limit"] = int(input_table.item(4, 2).text())
            config_data["block_energy_limits"]["kWh_export"]["lower_limit"] = int(input_table.item(4, 1).text())
            config_data["block_energy_limits"]["kWh_export"]["upper_limit"] = int(input_table.item(4, 2).text())

            with open("GuruxDLMSSimpleServerExample\Python_GUI\single_phase_config.json", "w") as f:
                f.write(json.dumps(config_data))

        else:
            with open(r"GuruxDLMSSimpleServerExample\Python_GUI\three_phase_config.json", "r") as f:
                config_data = json.loads(f.read())

        # Extract values from the table and update the JSON structure
        voltage_value_min = int(input_table.item(0, 1).text())
        voltage_value_max = int(input_table.item(0, 2).text())
        config_data["voltage_limits"]["L1"]["lower_limit"] = voltage_value_min
        config_data["voltage_limits"]["L1"]["upper_limit"] = voltage_value_max
        config_data["voltage_limits"]["L2"]["lower_limit"] = voltage_value_min
        config_data["voltage_limits"]["L2"]["upper_limit"] = voltage_value_max
        config_data["voltage_limits"]["L3"]["lower_limit"] = voltage_value_min
        config_data["voltage_limits"]["L3"]["upper_limit"] = voltage_value_max
        
        current_value_min = int(input_table.item(1, 1).text())
        current_value_max = int(input_table.item(1, 2).text())
        config_data["current_limits"]["L1"]["lower_limit"] = current_value_min
        config_data["current_limits"]["L1"]["upper_limit"] = current_value_max
        config_data["current_limits"]["L2"]["lower_limit"] = current_value_min
        config_data["current_limits"]["L2"]["upper_limit"] = current_value_max
        config_data["current_limits"]["L3"]["lower_limit"] = current_value_min
        config_data["current_limits"]["L3"]["upper_limit"] = current_value_max
        
        power_factor_value_min = int(input_table.item(2, 1).text())
        power_factor_value_max = int(input_table.item(2, 2).text())
        config_data["power_factor_limits"]["L1"]["lower_limit"] = power_factor_value_min
        config_data["power_factor_limits"]["L1"]["upper_limit"] = power_factor_value_max
        config_data["power_factor_limits"]["L2"]["lower_limit"] = power_factor_value_min
        config_data["power_factor_limits"]["L2"]["upper_limit"] = power_factor_value_max
        config_data["power_factor_limits"]["L3"]["lower_limit"] = power_factor_value_min
        config_data["power_factor_limits"]["L3"]["upper_limit"] = power_factor_value_max

        config_data["frequency_limits"]["lower_limit"] = int(input_table.item(3, 1).text())
        config_data["frequency_limits"]["upper_limit"] = int(input_table.item(3, 2).text())

        # Extracting energy values for block energy limits
        block_energy_value_min = int(input_table.item(4, 1).text())
        block_energy_value_max = int(input_table.item(4, 2).text())
        config_data["block_energy_limits"]["kWh_import"]["lower_limit"] = block_energy_value_min
        config_data["block_energy_limits"]["kWh_import"]["upper_limit"] = block_energy_value_max
        config_data["block_energy_limits"]["kVAh_lag"]["lower_limit"] = block_energy_value_min
        config_data["block_energy_limits"]["kVAh_lag"]["upper_limit"] = block_energy_value_max
        config_data["block_energy_limits"]["kVAh_lead"]["lower_limit"] = block_energy_value_min
        config_data["block_energy_limits"]["kVAh_lead"]["upper_limit"] = block_energy_value_max
        config_data["block_energy_limits"]["kVAh_import"]["lower_limit"] = block_energy_value_min
        config_data["block_energy_limits"]["kVAh_import"]["upper_limit"] = block_energy_value_max

        with open(r"GuruxDLMSSimpleServerExample\Python_GUI\three_phase_config.json", "w") as f:
            f.write(json.dumps(config_data))
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

        back_btn = QPushButton("Back", self)
        back_btn.clicked.connect(lambda: self.openPrevPage())
        back_btn.setFont(QFont("Arial", 12))
        back_btn.setMinimumWidth(100)
        back_btn.setStyleSheet("background-color: white; border: 1px solid black; border-radius: 5px; padding:8px")
        button_layout.addWidget(back_btn)

        add_btn = QPushButton("Add Row", self)
        add_btn.clicked.connect(lambda: self.addRow())
        add_btn.setFont(QFont("Arial", 12))
        add_btn.setMinimumWidth(100)
        add_btn.setStyleSheet("background-color: white; border: 1px solid black; border-radius: 5px;padding:8px")
        button_layout.addWidget(add_btn)

        # Submit button
        submit_button = QPushButton("Submit", self)
        submit_button.clicked.connect(self.printTableData)
        submit_button.setFont(QFont("Arial", 12))
        submit_button.setMinimumWidth(100)
        submit_button.setStyleSheet("background-color: white; border: 1px solid black; border-radius: 5px; padding:8px")
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
            elif column == 5: 
                # Create a custom widget for the checkbox
                checkbox_widget = QWidget()
                checkbox = QCheckBox()
                checkbox_layout = QHBoxLayout()
                checkbox_layout.addWidget(checkbox)
                checkbox_layout.setAlignment(Qt.AlignCenter)  # Center align the checkbox
                checkbox_layout.setContentsMargins(0, 0, 0, 0)  # Remove extra padding
                checkbox_widget.setLayout(checkbox_layout)
                self.table.setCellWidget(row_index, column, checkbox_widget)
            elif column == 6: 
                push_btn = QPushButton("Open")
                push_btn.setStyleSheet("background-color: white; border: 1px solid black; border-radius: 5px;padding:8px")
                popup_window = ParameterPopup(self.table)
                push_btn.clicked.connect(lambda: popup_window.show())
                self.table.setCellWidget(row_index, column, push_btn)
            else:
                item = QTableWidgetItem("")
                item.setTextAlignment(Qt.AlignCenter)  # Center align text
                item.setToolTip("Enter text")  # Tooltip for better UX when text is truncated
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
                        row_data.append(str(combobox.currentText()))
                elif column == 5:  # Garbage Values (Checkbox)
                    checkbox_widget = self.table.cellWidget(row, column)
                    checkbox = checkbox_widget.layout().itemAt(0).widget()
                    row_data.append("Checked" if checkbox.isChecked() else "Unchecked")
                else:
                    item = self.table.item(row, column)
                    row_data.append(item.text() if item else "")
            print(f"Row {row + 1}: {row_data}")

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
