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
import meter_automation

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

        input_table = QTableWidget()
        # Table Styling and Resizing
        input_table.setStyleSheet(
            "QTableWidget {font-size: 16px; gridline-color: #dcdcdc; background-color: white;}"
            "QHeaderView::section { background-color: #f0f0f0; font-weight: bold; padding: 6px;}"
        )
        input_table.setRowCount(5)
        input_table.setColumnCount(3)
        input_table.setHorizontalHeaderLabels(["Parameters","Min","Max"])
        input_table.verticalHeader().setVisible(False)
        input_table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        for i, label in enumerate(["V", "I", "P.F", "f", "Block Load"]):
            item = QTableWidgetItem(label)
            item.setTextAlignment(Qt.AlignCenter)  
            item.setFlags(item.flags() & ~Qt.ItemIsEditable) 
            input_table.setItem(i, 0, item)

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
        meter_type_for_file_path = ""

        # Function to check if input is a valid integer
        def is_valid_input(item):
            if item and item.text():  # Ensure the item and its text are not None or empty
                text = item.text()
                try:
                    float(text)  # Try converting to a float
                    return True
                except ValueError:
                    return False
            return False

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
        no_of_meters = int(self.parent_table.item(current_row, 1).text()) if self.parent_table.item(current_row, 1).text() != "" else 0

        # Convert Meter Type for file name
        file_path_name = {"Single Phase": "single_phase", "Three Phase (WC)": "three_phase_wc", "Three Phase (LTCT)": "three_phase_ltct", "Three Phase (HTCT)": "three_phase_htct"}
        if meter_type:
            meter_type_for_file_path = file_path_name[meter_type]
        
        # Get the directory of the current script
        script_directory = os.path.dirname(os.path.abspath(__file__))

        # Construct the config file path
        print(manufacturer.lower().replace(' ', '_'))
        config_file_name = fr"Config\{manufacturer.lower().replace(' ', '_')}_{meter_type_for_file_path}_config.json"

        # Prepend the full directory path
        config_file_path = os.path.join(script_directory, config_file_name)

        json_keys = ["no_of_meters", "meter_type", "manufacturer", "voltage_limits", "current_limits", "frequency_limits", "power_factor_limits", "block_energy_limits"]
        phases = ["L1", "L2", "L3"]
        limits = ["lower_limit", "upper_limit"]
        
        config_data = {}

        # Loop to dynamically insert keys in config_data
        for i, key in enumerate(json_keys):
            # First three static keys
            if key == "no_of_meters":
                config_data[key] = no_of_meters
            elif key == "meter_type":
                config_data[key] = meter_type
            elif key == "manufacturer":
                config_data[key] = manufacturer
            else:
                # Keys about meter values
                for j, limit in enumerate(limits):
                    if key not in config_data:
                        config_data[key] = {}
                    # For Single Phase
                    if meter_type == "Single Phase":
                        # For current
                        if key == "current_limits":
                            subkeys = ["phase_current", "neutral_current"]
                            for subkey in subkeys:
                                if subkey not in config_data[key]:
                                    config_data[key][subkey] = {}
                                config_data[key][subkey][limit] = int(float(input_table.item(i-3, j+1).text()) * 1000)
                        # For block load
                        elif key == "block_energy_limits":
                            subkeys = ["kWh_import", "kWh_export"]
                            for subkey in subkeys:
                                if subkey not in config_data[key]:
                                    config_data[key][subkey] = {}
                                config_data[key][subkey][limit] = int(float(input_table.item(i-3, j+1).text()) * 100)
                        # For Signed power factor
                        elif key == "power_factor_limits":
                            del config_data[key]
                            if "signed_" + key not in config_data:      
                                config_data["signed_" + key] = {}
                            config_data["signed_"+key][limit] = int(float(input_table.item(i-3, j+1).text()) * 1000)
                        # Other keys
                        else:
                            config_data[key][limit] = int(float(input_table.item(i-3, j+1).text()) * 1000)
                    # For Three phase
                    else:
                        # For frequency
                        if key == "frequency_limits":
                            config_data[key][limit] = int(float(input_table.item(i-3, j+1).text()) * 1000)
                        # For block load
                        elif key == "block_energy_limits":
                            subkeys = ["kWh_import", "kVAh_lag", "kVAh_lead", "kVAh_import"]
                            for subkey in subkeys:
                                if subkey not in config_data[key]:
                                    config_data[key][subkey] = {}
                                config_data[key][subkey][limit] = int(float(input_table.item(i-3, j+1).text()) * 100)
                        # Other keys
                        else:
                            for phase in phases:
                                # Current Multiplier
                                multiplier = 100000 if key == "current_limits" else 1000
                                if phase not in config_data[key]:
                                    config_data[key][phase] = {}
                                config_data[key][phase][limit] = int(float(input_table.item(i-3, j+1).text()) * multiplier)

        # Save the configuration to a new JSON file
        with open(config_file_path, "w") as f:
            json.dump(config_data, f, indent=4)
            
        self.close()

class ServerPage(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle("Server Page")
        self.setWindowState(Qt.WindowMaximized)
        self.setStyleSheet("background-color: #F1F1F1;")
        
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
                "Configure Parameters"
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
        submit_button.clicked.connect(self.submitRows)
        button_layout.addWidget(submit_button)
        main_layout.addLayout(button_layout)

        next_btn = createButton("Next")
        next_btn.clicked.connect(lambda: self.openNextPage())
        button_layout.addWidget(next_btn)

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

    def submitRows(self):
        servers_started_successfully = 0
        servers_failed_to_start = 0
        binary_file_name = ""
        binary_file_path = ""

        # Loop through the rows of the table
        for row in range(self.table.rowCount()):
            meter_type_for_file_path = ""

            # Extract values from each column
            type_of_meter = self.table.cellWidget(row, 0).currentText() if self.table.cellWidget(row, 0) else ""
            num_meters = int(self.table.item(row, 1).text()) if self.table.item(row, 1).text() != "" else 0
            manufacturer = self.table.item(row, 2).text() if self.table.item(row, 2) else ""
            start_port = int(self.table.item(row, 3).text()) if self.table.item(row, 3).text() != "" else 0
            start_instance = int(self.table.item(row, 4).text()) if self.table.item(row, 4).text() != "" else 0
            is_garbage_enabled = self.table.cellWidget(row, 5).findChild(QCheckBox).isChecked() if self.table.cellWidget(row, 5) else False

            # Convert Meter Type for file name
            file_path_name = {"Single Phase": "single_phase", "Three Phase (WC)": "three_phase_wc", "Three Phase (LTCT)": "three_phase_ltct", "Three Phase (HTCT)": "three_phase_htct"}
            if type_of_meter:
                meter_type_for_file_path = file_path_name[type_of_meter]

            # Get the directory of the current script
            script_directory = os.path.dirname(os.path.abspath(__file__))

            # Construct the config file path
            config_file_name = fr"Config\{manufacturer.lower().replace(' ', '_')}_{meter_type_for_file_path}_config.json"

            # Prepend the full directory path
            config_file_path = os.path.join(script_directory, config_file_name)

            # Binary file path
            if type_of_meter == "Single Phase":
                binary_file_name = r"Bin\gurux.dlms.simple.server.single.phase.bin"
            else:
                binary_file_name = r"Bin\gurux.dlms.simple.server.three.phase.bin"

            binary_file_path = os.path.join(script_directory, binary_file_name)

            print(config_file_path)
            print(binary_file_path)

            # Call the start_servers function with extracted values
            temp_num_servers_started, temp_num_servers_failed = meter_automation.start_servers(binary_file_path, config_file_path, num_meters, start_port, start_instance, type_of_meter, use_wsl=True, is_garbage_enabled=is_garbage_enabled)

            servers_started_successfully += temp_num_servers_started
            servers_failed_to_start += temp_num_servers_failed
        
        QMessageBox.information(self, "Servers Status", f"{servers_started_successfully} servers started successfully, {servers_failed_to_start} servers failed to start.")

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
